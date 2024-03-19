#include "test.hpp"
#include "eventFlags.hpp"
#include "file.hpp"
#include "logger.hpp"
#include "mutex.hpp"
#include "semaphore.hpp"
#include "trace.hpp"

using namespace std::chrono_literals;

class Device
{
  public:
    Device &operator=(const Device &) = delete;
    Device(const Device &) = delete;

    static Device &instance();
    ThreadPool m_memoryPool;
    Thread0 m_thread0;
    Thread1 m_thread1;
    Thread2 m_thread2;
    Thread3_4 m_thread3;
    Thread3_4 m_thread4;
    Thread5 m_thread5;
    Thread6_7 m_thread6;
    Thread6_7 m_thread7;
    Thread8 m_thread8;
    Thread9 m_thread9;
    ThreadFileSystem m_threadFileSystem;
    ThreadX::Mutex m_mutex;
    ThreadX::BinarySemaphore m_semaphore;
    ThreadX::EventFlags m_eventFlags;
    MsgQueue m_queue;
#ifndef NDEBUG
    [[gnu::section(".trace")]] static inline ThreadX::Trace<traceBufferSize> trace{20};
#endif

  private:
    Device();
};

static std::byte ramMem[20 * 512];

namespace ThreadX::Native
{
extern "C" void _fx_ram_driver(FX_MEDIA *media_ptr);
} // namespace ThreadX::Native

static void statckErrorCallback(ThreadX::Thread &thread)
{
    LOG_ERR("Stack Overflow in %s", thread.name().data());
}

void runTestCode()
{
    Logger::init(LogType::debug);
    ThreadX::Thread::registerStackErrorNotifyCallback(statckErrorCallback);
    Device::instance();
}

Device::Device()
    : m_memoryPool(), m_thread0("Thread0", m_memoryPool, thread0StackSize, {}, 1, 1),
      m_thread1("Thread1", m_memoryPool, thread1StackSize, {}, 16, 16, 4),
      m_thread2("Thread2", m_memoryPool, thread2StackSize, {}, 16, 16, 4),
      m_thread3("Thread3", m_memoryPool, thread3StackSize, {}, 8, 8),
      m_thread4("Thread4", m_memoryPool, thread4StackSize, {}, 8, 8),
      m_thread5("Thread5", m_memoryPool, thread5StackSize, {}, 4, 4),
      m_thread6("Thread6", m_memoryPool, thread6StackSize, {}, 8, 8),
      m_thread7("Thread7", m_memoryPool, thread7StackSize, {}, 8, 8),
      m_thread8(
          "Thread8", m_memoryPool, thread8StackSize, std::bind_front(&Thread8::enteryExitNotifyCallback, &m_thread8)),
      m_thread9("Thread9", m_memoryPool, thread9StackSize),
      m_threadFileSystem("ThreadFS", m_memoryPool, threadFileSystemStackSize, ramMem), m_mutex(),
      m_semaphore("Semaphore1", 1), m_eventFlags("Event flags1"),
      m_queue("Queue1", m_memoryPool, queueSize, std::bind_front(&Thread2::queueCallback, &m_thread2))
{
}

Device &Device::instance()
{
    static Device device;
    return device;
}

void Thread0::entryCallback()
{
    using namespace std::chrono_literals;

    Logger::clear();
    LOG_INFO("Thread 0 entered");
    auto &dev{Device::instance()};
    /* This thread simply sits in while-forever-sleep loop.  */
    while (1)
    {

        /* Increment the thread counter.  */
        m_counter++;

        /* Sleep for 10 ticks.  */
        ThreadX::ThisThread::sleepFor(100ms);

        /* Set event flag 0 to wakeup thread 5. */
        if (ThreadX::Error error{dev.m_eventFlags.set(0x1)}; error != ThreadX::Error::success)
        {
            LOG_ERR("Main thread ThreadX error %u!", error);
            break;
        }

        LOG_INFO("Event flag 0 set.");
    }
}

Thread1::Thread1(const std::string_view name, ThreadPool &pool, ThreadX::Ulong stackSize,
                 const NotifyCallback &entryExitNotifyCallback, ThreadX::Uint priority, ThreadX::Uint preamptionThresh,
                 ThreadX::Ulong timeSlice)
    : Thread(name, pool, stackSize, entryExitNotifyCallback, priority, preamptionThresh, timeSlice),
      m_timer1(500ms, std::bind_front(&Thread1::timerCallback, this)),
      m_timer2(1s, std::bind_front(&Thread1::timerCallback, this))
{
}

Thread2::Thread2(const std::string_view name, ThreadPool &pool, ThreadX::Ulong stackSize,
                 const NotifyCallback &entryExitNotifyCallback, ThreadX::Uint priority, ThreadX::Uint preamptionThresh,
                 ThreadX::Ulong timeSlice)
    : Thread(name, pool, stackSize, entryExitNotifyCallback, priority, preamptionThresh, timeSlice),
      m_timer(2s, std::bind_front(&Thread2::timerCallback, this))
{
}

void Thread1::entryCallback()
{
    LOG_INFO("Thread 1 entered.");
    auto &dev{Device::instance()};
    /* This thread simply sends messages to a queue shared by thread 2.  */
    while (1)
    {
        /* Increment the thread counter.  */
        m_counter++;
        /* Send message to queue 0.  */
        if (ThreadX::Error error{dev.m_queue.send(m_messages_sent)}; error != ThreadX::Error::success)
        {
            LOG_ERR("Thread 1 ThreadX error %u!", error);
            break;
        }

        LOG_DBG("Queue message sent: %u", m_messages_sent);
        /* Increment the message sent.  */
        m_messages_sent++;
        LOG_INFO("No. of message sent: %u", m_messages_sent);
        LOG_INFO("Timer1 callback counter: %u", timer1_counter);
        LOG_INFO("Timer2 callback counter: %u", timer2_counter);
    }
}

void Thread1::timerCallback(const uint32_t id)
{
    // timer is handled in interrupt (TX_TIMER_PROCESS_IN_ISR is enabled). So cannot call logger from here because it
    // cannot lock the mutex and returns with error.
    if (id == m_timer1.id())
    {
        ++timer1_counter;
    }
    else if (id == m_timer2.id())
    {
        ++timer2_counter;
    }
}

void Thread2::entryCallback()
{
    LOG_INFO("Thread 2 entered.");
    /* This thread retrieves messages placed on the queue by thread 1.  */
    while (1)
    {
        /* Increment the thread counter.  */
        m_counter++;
        /* Retrieve a message from the queue.  */
        auto [error, received_message] = Device::instance().m_queue.receive();
        if (error != ThreadX::Error::success)
        {
            LOG_ERR("Thread 2 ThreadX error %u!", error);
            break;
        }
        /* Check completion status and make sure the message is what we expected.  */
        if (received_message != m_messages_received)
        {
            LOG_ERR("Thread 2 error %u!", Error::unexpectedValue);
            break;
        }

        /* Otherwise, all is okay.  Increment the received message count.  */
        m_messages_received++;
        LOG_DBG("Queue message received: %u", received_message);
        LOG_INFO("No. of message received: %u", m_messages_received);
        LOG_INFO("Timer3 callback counter: %u", timer_counter);
    }
}

void Thread2::timerCallback(const uint32_t callbackID)
{
    // timer is handled in interrupt (TX_TIMER_PROCESS_IN_ISR is enabled). So cannot call logger from here because it
    // cannot lock the mutex and returns with error.
    if (callbackID == m_timer.id())
    {
        ++timer_counter;
    }
}

void Thread2::queueCallback([[maybe_unused]] MsgQueue &queue)
{
    LOG_INFO("queue message callback called.");
}

void Thread3_4::entryCallback()
{
    LOG_INFO("Thread 3 or 4 entered.");
    auto &dev{Device::instance()};
    ThreadX::Error error{};
    /* This function is executed from thread 3 and thread 4.  As the loop
       below shows, these function compete for ownership of semaphore_0.  */
    while (1)
    {
        /* Increment the thread counter.  */
        m_counter++;
        /* Get the semaphore with suspension.  */
        if (error = dev.m_semaphore.acquire(); error != ThreadX::Error::success)
        {
            LOG_ERR("Thread 3 or 4 ThreadX error %u!", error);
            break;
        }

        LOG_INFO("Semaphore acquired.");
        /* Sleep for 2 ticks to hold the semaphore.  */
        ThreadX::ThisThread::sleepFor(20ms);
        /* Release the semaphore.  */
        if (error = dev.m_semaphore.release(); error != ThreadX::Error::success)
        {
            break;
        }

        LOG_INFO("Semaphore released.");
    }

    LOG_ERR("Thread 3 or 4 ThreadX error %u!", error);
}

void Thread5::entryCallback()
{
    LOG_INFO("Thread 5 entered.");
    /* This thread simply waits for an event in a forever loop.  */
    while (1)
    {
        /* Increment the thread counter.  */
        m_counter++;
        /* Wait for event flag 0.  */
        auto [error, actual_flags] = Device::instance().m_eventFlags.waitAll(0x1);
        if (error != ThreadX::Error::success)
        {
            LOG_ERR("Thread 5 ThreadX error %u!", error);
            break;
        }

        /* Check status.  */
        if (actual_flags != 0x1)
        {
            LOG_ERR("Thread 5 error %u!", Error::unexpectedValue);
            break;
        }

        LOG_INFO("Event flag 0 got.");
    }
}

void Thread6_7::entryCallback()
{
    LOG_INFO("Thread 6 or 7 entered");
    auto &dev{Device::instance()};
    ThreadX::Error error{};
    /* This function is executed from thread 6 and thread 7.  As the loop
       below shows, these function compete for ownership of mutex_0.  */
    while (1)
    {
        /* Increment the thread counter.  */
        m_counter++;
        /* Get the mutex with suspension.  */
        if (error = dev.m_mutex.lock(); error != ThreadX::Error::success)
        {
            break;
        }

        LOG_INFO("mutex Locked.");
        /* Get the mutex again with suspension.  This shows
           that an owning thread may retrieve the mutex it
           owns multiple times.  */
        if (error = dev.m_mutex.lock(); error != ThreadX::Error::success)
        {
            break;
        }

        LOG_INFO("mutex Locked.");
        /* Sleep for 2 ticks to hold the mutex.  */
        ThreadX::ThisThread::sleepFor(20ms);
        /* Release the mutex.  */
        if (error = dev.m_mutex.unlock(); error != ThreadX::Error::success)
        {
            break;
        }

        LOG_INFO("mutex unlocked.");
        /* Release the mutex again.  This will actually
           release ownership since it was obtained twice.  */
        if (error = dev.m_mutex.unlock(); error != ThreadX::Error::success)
        {
            break;
        }

        LOG_INFO("mutex unlocked.");
    }

    LOG_ERR("Thread 6 or 7 ThreadX error %u!", error);
}

void Thread8::enteryExitNotifyCallback(
    [[maybe_unused]] ThreadX::Thread &thread, const ThreadX::NotifyCondition condition)
{
    if (condition == ThreadX::NotifyCondition::entry)
    {
        LOG_INFO("Thread 8 entry callback called.");
    }
    else
    {
        LOG_INFO("Thread 8 exit callback called.");
    }
}

void Thread8::entryCallback()
{
    LOG_INFO("Thread 8 entered.");
    if (auto error{ThreadX::ThisThread::sleepFor(3s)}; error != ThreadX::Error::success)
    {
        LOG_ERR("Thread 8 ThreadX error %u!", error);
    }
}

void Thread9::entryCallback()
{
    LOG_INFO("Thread 9 entered.");
    auto &dev{Device::instance()};

    while (1)
    {
        dev.m_thread8.join();
        LOG_INFO("Thread 8 joined to thread 9.");

        if (auto error{dev.m_thread8.restart()}; error != ThreadX::Error::success)
        {
            LOG_ERR("Thread 9 ThreadX error %u!", error);
            break;
        }
    }
}

ThreadFileSystem::ThreadFileSystem(
    const std::string_view name, ThreadPool &pool, ThreadX::Ulong stackSize, void *driverInfoPtr)
    : Thread(name, pool, stackSize), Media(driverInfoPtr)
{
}

void ThreadFileSystem::entryCallback()
{
    LOG_INFO("Thread file system entered");
    FileX::Error error{open()};

    do
    {
        if (error == FileX::Error::bootError)
        {
            if (error = format(20 * 512); error != FileX::Error::success)
            {
                break;
            }

            if (error = open(); error != FileX::Error::success)
            {
                break;
            }
        }

        if (error = createFile("my file.txt"); error != FileX::Error::success)
        {
            break;
        }

        while (true)
        {
            FileX::File file("my file.txt", *this, FileX::OpenOption::write);

            if (error = file.seek(0); error != FileX::Error::success)
            {
                break;
            }

            if (error = file.write(" ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"); error != FileX::Error::success)
            {
                break;
            }

            if (error = file.seek(0); error != FileX::Error::success)
            {
                break;
            }

            std::byte localBuffer[28];
            ThreadX::Uint actual;
            if (std::tie(error, actual) = file.read(localBuffer); error != FileX::Error::success)
            {
                break;
            }

            file.close();

            if (actual != 28)
            {
                LOG_ERR("Error reading file.");
                return;
            }
            else
            {
                LOG_INFO("Success reading file.");
            }

            LOG_INFO("Thread file system max stack used: %u%%", stackInfo().maxUsedPercent);
        };
    } while (0);

    LOG_ERR("Thread file system FileX error %u!", error);
}

void ThreadFileSystem::driverCallbackImpl(FileX::Media<> &media)
{
    _fx_ram_driver(std::addressof(media));
}

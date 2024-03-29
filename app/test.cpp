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
    Logger::init(Logger::Type::debug);
    ThreadX::Thread::registerStackErrorNotifyCallback(statckErrorCallback);
    Device::instance();
}

Device::Device()
    : m_memoryPool(), m_thread0("thread 0", m_memoryPool, thread0StackSize, {}, 1, 1),
      m_thread1("thread 1", m_memoryPool, thread1StackSize, {}, 16, 16, 4),
      m_thread2("thread 2", m_memoryPool, thread2StackSize, {}, 16, 16, 4),
      m_thread3("thread 3", m_memoryPool, thread3StackSize, {}, 8, 8),
      m_thread4("thread 4", m_memoryPool, thread4StackSize, {}, 8, 8),
      m_thread5("thread 5", m_memoryPool, thread5StackSize, {}, 4, 4),
      m_thread6("thread 6", m_memoryPool, thread6StackSize, {}, 8, 8),
      m_thread7("thread 7", m_memoryPool, thread7StackSize, {}, 8, 8),
      m_thread8(
          "thread 8", m_memoryPool, thread8StackSize, std::bind_front(&Thread8::enteryExitNotifyCallback, &m_thread8)),
      m_thread9("thread 9", m_memoryPool, thread9StackSize),
      m_threadFileSystem("thread FS", m_memoryPool, threadFileSystemStackSize, ramMem), m_mutex(),
      m_semaphore("semaphore 1", 1), m_eventFlags("event flags 1"),
      m_queue("queue 1", m_memoryPool, queueSize, std::bind_front(&Thread2::queueCallback, &m_thread2))
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

    LOG_CLR();
    LOG_INFO("%s entered", name().data());
    auto &dev{Device::instance()};
    auto eventName{dev.m_eventFlags.name().data()};
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
            LOG_ERR("%s ThreadX error %u!", eventName, error);
            break;
        }

        LOG_INFO("%s flag 0 set.", eventName);
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
    LOG_INFO("%s entered", name().data());
    auto &dev{Device::instance()};
    /* This thread simply sends messages to a queue shared by thread 2.  */
    while (1)
    {
        /* Increment the thread counter.  */
        m_counter++;
        /* Send message to queue 0.  */
        if (ThreadX::Error error{dev.m_queue.send(m_messages_sent)}; error != ThreadX::Error::success)
        {
            LOG_ERR("%s ThreadX error %u!", dev.m_queue.name().data(), error);
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
    const auto threadName{name().data()};
    LOG_INFO("%s entered", threadName);
    auto &dev{Device::instance()};
    auto queueName{dev.m_queue.name().data()};

    /* This thread retrieves messages placed on the queue by thread 1.  */
    while (1)
    {
        /* Increment the thread counter.  */
        m_counter++;
        /* Retrieve a message from the queue.  */
        auto [error, received_message] = dev.m_queue.receive();
        if (error != ThreadX::Error::success)
        {
            LOG_ERR("%s ThreadX error %u!", queueName, error);
            break;
        }
        /* Check completion status and make sure the message is what we expected.  */
        if (received_message != m_messages_received)
        {
            LOG_ERR("%s recieved message %u!", threadName, Error::unexpectedValue);
            break;
        }

        /* Otherwise, all is okay.  Increment the received message count.  */
        m_messages_received++;
        LOG_DBG("%s message received: %u", queueName, received_message);
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
    LOG_INFO("%s message callback called.", queue.name().data());
}

void Thread3_4::entryCallback()
{
    const auto threadName{name().data()};
    LOG_INFO("%s entered", threadName);
    auto &dev{Device::instance()};
    auto semaphoreName{dev.m_semaphore.name().data()};
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
            LOG_ERR("%s ThreadX error %u!", semaphoreName, error);
            break;
        }

        LOG_INFO("%s acquired.", semaphoreName);
        /* Sleep for 2 ticks to hold the semaphore.  */
        ThreadX::ThisThread::sleepFor(20ms);
        /* Release the semaphore.  */
        if (error = dev.m_semaphore.release(); error != ThreadX::Error::success)
        {
            LOG_ERR("%s ThreadX error %u!", semaphoreName, error);
            break;
        }

        LOG_INFO("%s released.", semaphoreName);
    }
}

void Thread5::entryCallback()
{
    const auto threadName{name().data()};
    LOG_INFO("%s entered", threadName);
    /* This thread simply waits for an event in a forever loop.  */
    while (1)
    {
        /* Increment the thread counter.  */
        m_counter++;
        /* Wait for event flag 0.  */
        auto [error, actual_flags] = Device::instance().m_eventFlags.waitAll(0x1);
        if (error != ThreadX::Error::success)
        {
            LOG_ERR("%s ThreadX error %u!", threadName, error);
            break;
        }

        /* Check status.  */
        if (actual_flags != 0x1)
        {
            LOG_ERR("%s error %u!", threadName, Error::unexpectedValue);
            break;
        }

        LOG_INFO("%s flag 0 got.", Device::instance().m_eventFlags.name().data());
    }
}

void Thread6_7::entryCallback()
{
    const auto threadName{name().data()};
    LOG_INFO("%s entered", threadName);
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

    LOG_ERR("%s ThreadX error %u!", threadName, error);
}

void Thread8::enteryExitNotifyCallback(
    [[maybe_unused]] ThreadX::Thread &thread, const ThreadX::Thread::NotifyCondition condition)
{
    const auto threadName{name().data()};

    if (condition == ThreadX::Thread::NotifyCondition::entry)
    {
        LOG_INFO("%s entry callback called.", threadName);
    }
    else
    {
        LOG_INFO("%s exit callback called.", threadName);
    }
}

void Thread8::entryCallback()
{
    const auto threadName{name().data()};
    LOG_INFO("%s entered", threadName);
    if (auto error{ThreadX::ThisThread::sleepFor(3s)}; error != ThreadX::Error::success)
    {
        LOG_ERR("%s ThreadX error %u!", threadName, error);
    }
}

void Thread9::entryCallback()
{
    const auto threadName{name().data()};
    LOG_INFO("%s entered", threadName);
    auto &dev{Device::instance()};

    while (1)
    {
        dev.m_thread8.join();
        LOG_INFO("%s joined to %s.", dev.m_thread8.name().data(), threadName);

        if (auto error{dev.m_thread8.restart()}; error != ThreadX::Error::success)
        {
            LOG_ERR("%s ThreadX error %u!", threadName, error);
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
    const auto threadName{name().data()};
    LOG_INFO("%s entered", threadName);
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

            LOG_INFO("%s max stack used: %u%%", threadName, stackInfo().maxUsedPercent);
        };
    } while (0);

    LOG_ERR("%s error %u!", threadName, error);
}

void ThreadFileSystem::driverCallbackImpl(FileX::Media<> &media)
{
    _fx_ram_driver(std::addressof(media));
}

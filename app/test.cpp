#include "test.hpp"
#include "eventFlags.hpp"
#include "file.hpp"
#include "mutex.hpp"
#include "norFlash.hpp"
#include "rttLogger.hpp"
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
    ThreadRamFileSystem m_threadRamFileSystem;
    ThreadNorFileSystem m_threadNorFileSystem;
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

static ThreadX::Uchar ramMem[20 * 512];
static ThreadX::Uchar norMem[20 * 512];

namespace ThreadX::Native
{
extern "C" void _fx_ram_driver(FX_MEDIA *media_ptr);
extern "C" void _fx_nor_flash_simulator_driver(FX_MEDIA *media_ptr);
extern "C" UINT _lx_nor_flash_simulator_erase_all();
extern "C" UINT _lx_nor_flash_simulator_read(
    LX_NOR_FLASH *nor_flash, ULONG *flash_address, ULONG *destination, ULONG words);
extern "C" UINT _lx_nor_flash_simulator_write(
    LX_NOR_FLASH *nor_flash, ULONG *flash_address, ULONG *source, ULONG words);
extern "C" UINT _lx_nor_flash_simulator_block_erase(LX_NOR_FLASH *nor_flash, ULONG block, ULONG erase_count);
extern "C" UINT _lx_nor_flash_simulator_block_erased_verify(LX_NOR_FLASH *nor_flash, ULONG block);
} // namespace ThreadX::Native

static void statckErrorCallback(ThreadX::ThreadBase &thread)
{
    LOG_ERR("Stack Overflow in %s", thread.name().data());
}

void runTestCode()
{
    RttLogger::init(RttLogger::Type::debug);
    Thread::registerStackErrorNotifyCallback(statckErrorCallback);
    Device::instance();
}

struct PrintName
{
    void operator()(ThreadX::ThreadBase &thread, ThreadX::ThreadBase::NotifyCondition cond) const
    {
        if (cond == ThreadX::ThreadBase::NotifyCondition::entry)
        {
            LOG_INFO("%s entered", thread.name().data());
        }
    }
};

Device::Device()
    : m_memoryPool("byte pool"), m_thread0("thread 0", m_memoryPool, thread0StackSize, PrintName(), 1, 1),
      m_thread1("thread 1", m_memoryPool, thread1StackSize, PrintName(), 16, 16, 4),
      m_thread2("thread 2", m_memoryPool, thread2StackSize, PrintName(), 16, 16, 4),
      m_thread3("thread 3", m_memoryPool, thread3StackSize, PrintName(), 8, 8),
      m_thread4("thread 4", m_memoryPool, thread4StackSize, PrintName(), 8, 8),
      m_thread5("thread 5", m_memoryPool, thread5StackSize, PrintName(), 4, 4),
      m_thread6("thread 6", m_memoryPool, thread6StackSize, PrintName(), 8, 8),
      m_thread7("thread 7", m_memoryPool, thread7StackSize, PrintName(), 8, 8),
      m_thread8(
          "thread 8", m_memoryPool, thread8StackSize, std::bind_front(&Thread8::enteryExitNotifyCallback, &m_thread8)),
      m_thread9("thread 9", m_memoryPool, thread9StackSize),
      m_threadRamFileSystem("thread ram FS", m_memoryPool, threadRamFileSystemStackSize, PrintName(), ramMem),
      m_threadNorFileSystem("thread nor FS", m_memoryPool, threadNorFileSystemStackSize, PrintName()), m_mutex(),
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
      m_timer1("timer1", 500ms, std::bind_front(&Thread1::timerCallback, this)),
      m_timer2("timer2", 1s, std::bind_front(&Thread1::timerCallback, this))
{
}

void Thread1::entryCallback()
{
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

Thread2::Thread2(const std::string_view name, ThreadPool &pool, ThreadX::Ulong stackSize,
                 const NotifyCallback &entryExitNotifyCallback, ThreadX::Uint priority, ThreadX::Uint preamptionThresh,
                 ThreadX::Ulong timeSlice)
    : Thread(name, pool, stackSize, entryExitNotifyCallback, priority, preamptionThresh, timeSlice),
      m_timer("timer3", 2s, std::bind_front(&Thread2::timerCallback, this))
{
}

void Thread2::entryCallback()
{
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
            LOG_ERR("%s recieved message %u!", name().data(), Error::unexpectedValue);
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

void Thread2::queueCallback(ThreadX::QueueBase<uint32_t> &queue)
{
    LOG_INFO("%s message callback called.", queue.name().data());
}

void Thread3_4::entryCallback()
{
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
        ThreadX::ThisThread::sleepFor(100ms);
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
        ThreadX::ThisThread::sleepFor(100ms);
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

    LOG_ERR("%s ThreadX error %u!", name().data(), error);
}

void Thread8::enteryExitNotifyCallback([[maybe_unused]] ThreadBase &thread, const Thread::NotifyCondition condition)
{
    if (condition == Thread::NotifyCondition::entry)
    {
        LOG_INFO("%s entry callback called.", name().data());
    }
    else
    {
        LOG_INFO("%s exit callback called.", name().data());
    }
}

void Thread8::entryCallback()
{
    if (auto error{ThreadX::ThisThread::sleepFor(3s)}; error != ThreadX::Error::success)
    {
        LOG_ERR("%s ThreadX error %u!", name().data(), error);
    }
}

void Thread9::entryCallback()
{
    auto &dev{Device::instance()};

    while (1)
    {
        dev.m_thread8.join();
        LOG_INFO("%s joined to %s.", dev.m_thread8.name().data(), name().data());

        if (auto error{dev.m_thread8.restart()}; error != ThreadX::Error::success)
        {
            LOG_ERR("%s ThreadX error %u!", name().data(), error);
            break;
        }
    }
}

ThreadRamFileSystem::ThreadRamFileSystem(const std::string_view name, ThreadPool &pool, ThreadX::Ulong stackSize,
                                         const ThreadBase::NotifyCallback &notifyCallback, void *driverInfoPtr)
    : Thread(name, pool, stackSize, notifyCallback),
      m_media(std::bind_front(&ThreadRamFileSystem::driverCallback, this), driverInfoPtr)
{
}

void ThreadRamFileSystem::entryCallback()
{
    FileX::Error error{m_media.open("ram media")};

    do
    {
        if (error == FileX::Error::bootError)
        {
            if (error = m_media.format("ram disk", 20 * 512); error != FileX::Error::success)
            {
                break;
            }

            if (error = m_media.open("ram media"); error != FileX::Error::success)
            {
                break;
            }
        }
        else if (error != FileX::Error::success)
        {
            break;
        }

        if (error = m_media.createFile("my file.txt"); error != FileX::Error::success)
        {
            break;
        }

        while (true)
        {
            FileX::File file("my file.txt", m_media, FileX::OpenOption::write);

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

            LOG_INFO("%s max stack used: %u%%", name().data(), stackInfo().maxUsedPercent);

            ThreadX::ThisThread::sleepFor(1s);
        };
    } while (0);

    LOG_ERR("%s error %X!", name().data(), error);
}

void ThreadRamFileSystem::driverCallback(ThreadX::Native::FX_MEDIA *mediaPtr)
{
    _fx_ram_driver(mediaPtr);
}

ThreadNorFileSystem::ThreadNorFileSystem(const std::string_view name, ThreadPool &pool, ThreadX::Ulong stackSize,
                                         const ThreadBase::NotifyCallback &notifyCallback)
    : Thread(name, pool, stackSize, notifyCallback),
      m_media(std::bind_front(&ThreadNorFileSystem::driverCallback, this))
{
}

void ThreadNorFileSystem::entryCallback()
{
    LevelX::NorFlashBase::Driver driver{
        nullptr,
        ThreadX::Native::_lx_nor_flash_simulator_read,
        ThreadX::Native::_lx_nor_flash_simulator_write,
        ThreadX::Native::_lx_nor_flash_simulator_block_erase,
        ThreadX::Native::_lx_nor_flash_simulator_block_erased_verify,
        nullptr};
    LevelX::NorFlash<> norFlash(32 * 512, 4096, driver, reinterpret_cast<ThreadX::Ulong>(norMem));

    FileX::Error error{m_media.open("nor media")};

    do
    {
        if (error != FileX::Error::success)
        {
            ThreadX::Native::_lx_nor_flash_simulator_erase_all();

            if (error = m_media.format("nor disk", norFlash.formatSize()); error != FileX::Error::success)
            {
                break;
            }

            if (error = m_media.open("nor media"); error != FileX::Error::success)
            {
                break;
            }
        }

        if (error = m_media.createFile("my file.txt"); error != FileX::Error::success)
        {
            break;
        }

        while (true)
        {
            FileX::File file("my file.txt", m_media, FileX::OpenOption::write);

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

            LOG_INFO("%s max stack used: %u%%", name().data(), stackInfo().maxUsedPercent);

            ThreadX::ThisThread::sleepFor(1s);
        };
    } while (0);

    LOG_ERR("%s error 0x%X!", name().data(), error);
}

void ThreadNorFileSystem::driverCallback(ThreadX::Native::FX_MEDIA *mediaPtr)
{
    _fx_nor_flash_simulator_driver(mediaPtr);
}
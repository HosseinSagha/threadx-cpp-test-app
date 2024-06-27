#pragma once

#include "media.hpp"
#include "memoryPool.hpp"
#include "queue.hpp"
#include "thread.hpp"
#include "tickTimer.hpp"
#include "norFlash.hpp"
#include <cstddef>

inline constexpr ThreadX::Ulong thread0StackSize{ThreadX::ThreadBase::minimumStackSize};
inline constexpr ThreadX::Ulong thread1StackSize{2 * ThreadX::ThreadBase::minimumStackSize};
inline constexpr ThreadX::Ulong thread2StackSize{ThreadX::ThreadBase::minimumStackSize};
inline constexpr ThreadX::Ulong thread3StackSize{ThreadX::ThreadBase::minimumStackSize};
inline constexpr ThreadX::Ulong thread4StackSize{ThreadX::ThreadBase::minimumStackSize};
inline constexpr ThreadX::Ulong thread5StackSize{ThreadX::ThreadBase::minimumStackSize};
inline constexpr ThreadX::Ulong thread6StackSize{ThreadX::ThreadBase::minimumStackSize};
inline constexpr ThreadX::Ulong thread7StackSize{ThreadX::ThreadBase::minimumStackSize};
inline constexpr ThreadX::Ulong thread8StackSize{2 * ThreadX::ThreadBase::minimumStackSize};
inline constexpr ThreadX::Ulong thread9StackSize{ThreadX::ThreadBase::minimumStackSize};
inline constexpr ThreadX::Ulong threadRamFileSystemStackSize{3 * ThreadX::ThreadBase::minimumStackSize};
inline constexpr ThreadX::Ulong threadNorFileSystemStackSize{4 * ThreadX::ThreadBase::minimumStackSize};
inline constexpr ThreadX::Ulong queueSize{100};

inline constexpr ThreadX::Ulong threadMemPoolSize{ThreadX::BytePoolBase::minimumPoolSize(
    {{thread0StackSize, thread1StackSize, thread2StackSize, thread3StackSize, thread4StackSize, thread5StackSize,
      thread6StackSize, thread7StackSize, thread8StackSize, thread9StackSize, threadRamFileSystemStackSize,
      threadNorFileSystemStackSize, ThreadX::Ulong{sizeof(uint32_t) * queueSize}}})};

inline constexpr ThreadX::Ulong traceBufferSize{32'000};
inline constexpr size_t loggerStringReservedMemory{256};

using ThreadPool = ThreadX::BytePool<threadMemPoolSize>;
using Thread = ThreadX::Thread<ThreadPool>;
using MsgQueue = ThreadX::Queue<uint32_t, ThreadPool>;

class Thread0 : public Thread
{
  public:
    using Thread::Thread;

  private:
    void entryCallback() final;

    uint32_t m_counter{};
};

class Thread1 : public Thread
{
  public:
    Thread1(const std::string_view name, ThreadPool &pool, ThreadX::Ulong stackSize,
            const NotifyCallback &entryExitNotifyCallback, ThreadX::Uint priority, ThreadX::Uint preamptionThresh,
            ThreadX::Ulong timeSlice);

  private:
    void entryCallback() final;
    void timerCallback(const uint32_t callbackID);

    uint32_t m_counter{};
    uint32_t m_messages_sent{};
    ThreadX::TickTimer m_timer1;
    ThreadX::TickTimer m_timer2;
    uint32_t timer1_counter{};
    uint32_t timer2_counter{};
};

class Thread2 : public Thread
{
  public:
    Thread2(const std::string_view name, ThreadPool &pool, ThreadX::Ulong stackSize,
            const NotifyCallback &entryExitNotifyCallback, ThreadX::Uint priority, ThreadX::Uint preamptionThresh,
            ThreadX::Ulong timeSlice);
    void queueCallback(ThreadX::QueueBase<uint32_t> &queue);

  private:
    void entryCallback() final;
    void timerCallback(const uint32_t callbackID);

    uint32_t m_counter{};
    uint32_t m_messages_received{};
    ThreadX::TickTimer m_timer;
    uint32_t timer_counter{};
};

class Thread3_4 : public Thread
{
  public:
    using Thread::Thread;

  private:
    void entryCallback() final;

    uint32_t m_counter{};
};

class Thread5 : public Thread
{
  public:
    using Thread::Thread;

  private:
    void entryCallback() final;

    uint32_t m_counter{};
};

class Thread6_7 : public Thread
{
  public:
    using Thread::Thread;

  private:
    void entryCallback() final;

    uint32_t m_counter{};
};

class Thread8 : public Thread
{
  public:
    using Thread<ThreadPool>::Thread;
    void enteryExitNotifyCallback(ThreadBase &thread, const Thread::NotifyCondition condition);

  private:
    void entryCallback() final;
};

class Thread9 : public Thread
{
  public:
    using Thread::Thread;

  private:
    void entryCallback() final;
};

class ThreadRamFileSystem : public Thread
{
  public:
    ThreadRamFileSystem(const std::string_view name, ThreadPool &pool, ThreadX::Ulong stackSize,
                        const ThreadBase::NotifyCallback &notifyCallback, void *driverInfoPtr);

  private:
    void entryCallback() final;
    void driverCallback(ThreadX::Native::FX_MEDIA *mediaPtr);

    FileX::Media<> m_media;
};

class ThreadNorFileSystem : public Thread
{
  public:
    ThreadNorFileSystem(const std::string_view name, ThreadPool &pool, ThreadX::Ulong stackSize,
                        const ThreadBase::NotifyCallback &notifyCallback);

  private:
    void entryCallback() final;
    void driverCallback(ThreadX::Native::FX_MEDIA *mediaPtr);

    FileX::Media<LevelX::NorFlashBase::sectorSize()> m_media;
};

void runTestCode();

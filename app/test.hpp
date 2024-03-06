#pragma once

#include "media.hpp"
#include "memoryPool.hpp"
#include "queue.hpp"
#include "thread.hpp"
#include "tickTimer.hpp"
#include <cstddef>

inline constexpr ThreadX::Ulong thread0StackSize{1024};
inline constexpr ThreadX::Ulong thread1StackSize{2 * 1024};
inline constexpr ThreadX::Ulong thread2StackSize{1024};
inline constexpr ThreadX::Ulong thread3StackSize{1024};
inline constexpr ThreadX::Ulong thread4StackSize{1024};
inline constexpr ThreadX::Ulong thread5StackSize{1024};
inline constexpr ThreadX::Ulong thread6StackSize{1024};
inline constexpr ThreadX::Ulong thread7StackSize{1024};
inline constexpr ThreadX::Ulong threadFileSystemStackSize{20 * 1024};
inline constexpr ThreadX::Ulong queueSize{100};

using MsgQueue = ThreadX::Queue<uint32_t>;

inline constexpr ThreadX::Ulong threadMemPoolSize{ThreadX::BytePoolBase::minimumPoolSize(
    thread0StackSize, thread1StackSize, thread2StackSize, thread3StackSize, thread4StackSize, thread5StackSize,
    thread6StackSize, thread7StackSize, threadFileSystemStackSize, MsgQueue::messageSize() * queueSize)};

inline constexpr ThreadX::Ulong traceBufferSize{32'000};
inline constexpr size_t loggerStringReservedMemory{256};

using ThreadPool = ThreadX::BytePool<threadMemPoolSize>;

class Thread0 : public ThreadX::Thread
{
  public:
    using Thread::Thread;

  private:
    void entryCallback() final;

    uint32_t m_counter{};
};

class Thread1 : public ThreadX::Thread
{
  public:
    Thread1(ThreadPool &pool, ThreadX::Ulong stackSize, const NotifyCallback &entryExitNotifyCallback,
            ThreadX::Uint priority, ThreadX::Uint preamptionThresh, ThreadX::Ulong timeSlice);

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

class Thread2 : public ThreadX::Thread
{
  public:
    Thread2(ThreadPool &pool, ThreadX::Ulong stackSize, const NotifyCallback &entryExitNotifyCallback,
            ThreadX::Uint priority, ThreadX::Uint preamptionThresh, ThreadX::Ulong timeSlice);
    void queueCallback(MsgQueue &queue);

  private:
    void entryCallback() final;
    void timerCallback(const uint32_t callbackID);

    uint32_t m_counter{};
    uint32_t m_messages_received{};
    ThreadX::TickTimer m_timer;
    uint32_t timer_counter{};
};

class Thread3_4 : public ThreadX::Thread
{
  public:
    using Thread::Thread;

  private:
    void entryCallback() final;

    uint32_t m_counter{};
};

class Thread5 : public ThreadX::Thread
{
  public:
    using Thread::Thread;

  private:
    void entryCallback() final;

    uint32_t m_counter{};
};

class Thread6_7 : public ThreadX::Thread
{
  public:
    using Thread::Thread;

  private:
    void entryCallback() final;

    uint32_t m_counter{};
};

class ThreadFileSystem : public ThreadX::Thread, public FileX::Media<>
{
  public:
    ThreadFileSystem(ThreadPool &pool, ThreadX::Ulong stackSize, void *driverInfoPtr);

  private:
    void driverCallbackImpl(FileX::Media<> &) final;
    void entryCallback() final;
};

void runTestCode();

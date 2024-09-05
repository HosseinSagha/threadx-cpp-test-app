#pragma once

#include "media.hpp"
#include "memoryPool.hpp"
#include "norFlash.hpp"
#include "queue.hpp"
#include "thread.hpp"
#include "tickTimer.hpp"
#include <cstddef>

inline constexpr ThreadX::Ulong thread0StackSize{ThreadX::minimumStackSize};
inline constexpr ThreadX::Ulong thread1StackSize{2 * ThreadX::minimumStackSize};
inline constexpr ThreadX::Ulong thread2StackSize{ThreadX::minimumStackSize};
inline constexpr ThreadX::Ulong thread3StackSize{ThreadX::minimumStackSize};
inline constexpr ThreadX::Ulong thread4StackSize{ThreadX::minimumStackSize};
inline constexpr ThreadX::Ulong thread5StackSize{ThreadX::minimumStackSize};
inline constexpr ThreadX::Ulong thread6StackSize{ThreadX::minimumStackSize};
inline constexpr ThreadX::Ulong thread7StackSize{ThreadX::minimumStackSize};
inline constexpr ThreadX::Ulong thread8StackSize{2 * ThreadX::minimumStackSize};
inline constexpr ThreadX::Ulong thread9StackSize{ThreadX::minimumStackSize};
inline constexpr ThreadX::Ulong threadRamFileSystemStackSize{3 * ThreadX::minimumStackSize};
inline constexpr ThreadX::Ulong threadNorFileSystemStackSize{3 * ThreadX::minimumStackSize};
inline constexpr ThreadX::Ulong queueSize{100};

inline constexpr ThreadX::Ulong threadMemPoolSize{ThreadX::minimumPoolSize(
    {{thread0StackSize, thread1StackSize, thread2StackSize, thread3StackSize, thread4StackSize, thread5StackSize,
      thread6StackSize, thread7StackSize, thread8StackSize, thread9StackSize, threadRamFileSystemStackSize,
      threadNorFileSystemStackSize, ThreadX::Ulong{sizeof(uint32_t) * queueSize}}})};

inline constexpr ThreadX::Ulong traceBufferSize{32'000};
inline constexpr size_t loggerStringReservedMemory{256};

using ThreadPool = ThreadX::BytePool<threadMemPoolSize>;
using Thread = ThreadX::Thread<ThreadPool>;
using MsgQueue = ThreadX::Queue<uint32_t, ThreadPool>;
using NorFlash = LevelX::NorFlash<16>;

inline constexpr auto norStorageSize{64 * 1024};
inline constexpr auto norBlocks{norStorageSize / sizeof(NorFlash::Block)};

class Thread0 : public Thread
{
  public:
    using Thread::Thread;
    virtual ~Thread0() = default;

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
    virtual ~Thread1() = default;

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
    virtual ~Thread2() = default;

    void queueCallback(MsgQueue &queue);

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

    virtual ~Thread3_4() = default;

  private:
    void entryCallback() final;

    uint32_t m_counter{};
};

class Thread5 : public Thread
{
  public:
    using Thread::Thread;

    virtual ~Thread5() = default;

  private:
    void entryCallback() final;

    uint32_t m_counter{};
};

class Thread6_7 : public Thread
{
  public:
    using Thread::Thread;

    virtual ~Thread6_7() = default;

  private:
    void entryCallback() final;

    uint32_t m_counter{};
};

class Thread8 : public Thread
{
  public:
    using Thread<ThreadPool>::Thread;

    virtual ~Thread8() = default;

  private:
    void entryCallback() final;
};

class Thread9 : public Thread
{
  public:
    using Thread::Thread;

    virtual ~Thread9() = default;

  private:
    void entryCallback() final;
};

class RamMedia : public FileX::Media<>
{
  public:
    RamMedia(std::byte *driverInfoPtr);
    virtual ~RamMedia() = default;

    void driverCallback() final;
};

class ThreadRamFileSystem : public Thread
{
  public:
    ThreadRamFileSystem(const std::string_view name, ThreadPool &pool, ThreadX::Ulong stackSize,
                        const Thread::NotifyCallback &notifyCallback, std::byte *driverInfoPtr);
    virtual ~ThreadRamFileSystem() = default;

  private:
    void entryCallback() final;

    RamMedia m_media;
};

class NorFlashDriver : public NorFlash
{
  public:
    using NorFlash::NorFlash;
    virtual ~NorFlashDriver() = default;

    LevelX::Error readCallback(
        ThreadX::Ulong *flashAddress, ThreadX::Ulong *destination, const ThreadX::Ulong words) final;
    LevelX::Error writeCallback(ThreadX::Ulong *flashAddress, ThreadX::Ulong *source, const ThreadX::Ulong words) final;
    LevelX::Error eraseBlockCallback(const ThreadX::Ulong block, const ThreadX::Ulong eraseCount) final;
    LevelX::Error verifyErasedBlockCallback(const ThreadX::Ulong block) final;
};

class NorMedia : public FileX::Media<NorFlash::sectorSize()>
{
  public:
    NorMedia(NorFlashDriver &m_norFlash);
    virtual ~NorMedia() = default;

    void driverCallback() final;

    NorFlashDriver &m_norFlash;
};

class ThreadNorFileSystem : public Thread
{
  public:
    ThreadNorFileSystem(const std::string_view name, ThreadPool &pool, ThreadX::Ulong stackSize,
                        const Thread::NotifyCallback &notifyCallback);
    virtual ~ThreadNorFileSystem() = default;

  private:
    void entryCallback() final;

    NorFlashDriver m_norFlash;
    NorMedia m_media;
};

void runTestCode();

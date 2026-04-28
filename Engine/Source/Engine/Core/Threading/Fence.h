#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <cstdint>

namespace Engine
{
    // -----------------------------------------------------------------------
    // CPUFence — decrement-to-zero gate.
    //
    // Usage:
    //   CPUFence fence(jobCount);
    //   // In each job:  fence.Signal();
    //   // On main:      fence.Wait();
    // -----------------------------------------------------------------------
    class CPUFence
    {
    public:
        explicit CPUFence(uint32_t count = 0) : m_Remaining(count) {}

        void Reset(uint32_t count)
        {
            std::lock_guard lock(m_Mutex);
            m_Remaining.store(count);
        }

        // Called by a worker when its job slice completes.
        void Signal()
        {
            if (m_Remaining.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                std::lock_guard lock(m_Mutex);
                m_CV.notify_all();
            }
        }

        // Block calling thread until counter reaches zero.
        void Wait()
        {
            std::unique_lock lock(m_Mutex);
            m_CV.wait(lock, [this] { return m_Remaining.load(std::memory_order_acquire) == 0; });
        }

        bool IsDone() const { return m_Remaining.load(std::memory_order_acquire) == 0; }

    private:
        std::atomic<uint32_t>   m_Remaining;
        std::mutex              m_Mutex;
        std::condition_variable m_CV;
    };
}

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Engine
{
    // Fixed-size thread pool. Workers sleep on condition_variable — no spinning.
    // Designed for CPU-only work. Vulkan submission must NOT go through here.
    class ThreadPool
    {
    public:
        explicit ThreadPool(uint32_t threadCount = 0); // 0 = hardware_concurrency - 1
        ~ThreadPool();

        // Non-copyable, non-movable
        ThreadPool(const ThreadPool&)            = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        void Submit(std::function<void()> task);

        // Block until queue is empty and all workers are idle.
        void WaitIdle();

        uint32_t ThreadCount() const { return static_cast<uint32_t>(m_Workers.size()); }

    private:
        void WorkerLoop();

        std::vector<std::thread>        m_Workers;
        std::queue<std::function<void()>> m_Queue;
        std::mutex                      m_Mutex;
        std::condition_variable         m_CV;
        std::condition_variable         m_IdleCV;
        std::atomic<uint32_t>           m_ActiveTasks{ 0 };
        bool                            m_Stop = false;
    };
}

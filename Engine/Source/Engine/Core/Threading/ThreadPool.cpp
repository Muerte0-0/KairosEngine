#include "kepch.h"
#include "ThreadPool.h"

namespace Engine
{
    ThreadPool::ThreadPool(uint32_t threadCount)
    {
        if (threadCount == 0)
        {
            // Leave one core for the main/render thread.
            const uint32_t hw = std::thread::hardware_concurrency();
            threadCount = hw > 1 ? hw - 1 : 1;
        }

        m_Workers.reserve(threadCount);
        for (uint32_t i = 0; i < threadCount; ++i)
            m_Workers.emplace_back(&ThreadPool::WorkerLoop, this);
    }

    ThreadPool::~ThreadPool()
    {
        {
            std::lock_guard lock(m_Mutex);
            m_Stop = true;
        }
        m_CV.notify_all();
        for (auto& w : m_Workers)
            w.join();
    }

    void ThreadPool::Submit(std::function<void()> task)
    {
        {
            std::lock_guard lock(m_Mutex);
            m_Queue.push(std::move(task));
        }
        m_CV.notify_one();
    }

    void ThreadPool::WaitIdle()
    {
        std::unique_lock lock(m_Mutex);
        m_IdleCV.wait(lock, [this]
        {
            return m_Queue.empty() && m_ActiveTasks.load() == 0;
        });
    }

    void ThreadPool::WorkerLoop()
    {
        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock lock(m_Mutex);
                m_CV.wait(lock, [this] { return m_Stop || !m_Queue.empty(); });

                if (m_Stop && m_Queue.empty())
                    return;

                task = std::move(m_Queue.front());
                m_Queue.pop();
                ++m_ActiveTasks;
            }

            task();

            {
                std::lock_guard lock(m_Mutex);
                --m_ActiveTasks;
            }
            // Wake anyone waiting in WaitIdle().
            m_IdleCV.notify_all();
        }
    }
}

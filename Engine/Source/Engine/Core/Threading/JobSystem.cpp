#include "kepch.h"
#include "JobSystem.h"

namespace Engine
{
    Scope<ThreadPool> JobSystem::s_Pool = nullptr;

    void JobSystem::Init(uint32_t workerCount)
    {
        s_Pool = CreateScope<ThreadPool>(workerCount);
    }

    void JobSystem::Shutdown()
    {
        s_Pool.reset();
    }

    JobHandle JobSystem::Submit(std::function<void()> work, JobPriority /*priority*/)
    {
        // Priority is reserved for a future priority-queue upgrade.
        // Current impl: all jobs share one FIFO queue (simple, low-overhead).
        auto task    = std::make_shared<std::packaged_task<void()>>(std::move(work));
        JobHandle handle{ task->get_future().share() };
        s_Pool->Submit([task]() { (*task)(); });
        return handle;
    }

    JobHandle JobSystem::SubmitAfter(std::vector<JobHandle> deps,
                                     std::function<void()> work,
                                     JobPriority priority)
    {
        // Spin up a helper thread that waits on deps then re-submits via Submit().
        // The helper thread is cheap: it blocks on futures, not on pool workers.
        auto innerWork = std::move(work);
        auto innerDeps = std::move(deps);

        return Submit([deps = std::move(innerDeps), work = std::move(innerWork), priority]() mutable
        {
            for (auto& d : deps)
                d.Wait();
            // Re-submit into pool from inside a worker — safe because the
            // packaged_task future chain is independent.
            JobSystem::Submit(std::move(work), priority).Wait();
        }, priority);
    }

    std::vector<JobHandle> JobSystem::SubmitBatch(uint32_t count,
                                                   std::function<void(uint32_t)> work,
                                                   JobPriority priority)
    {
        std::vector<JobHandle> handles;
        handles.reserve(count);
        for (uint32_t i = 0; i < count; ++i)
            handles.push_back(Submit([work, i]() { work(i); }, priority));
        return handles;
    }

    void JobSystem::WaitIdle()
    {
        s_Pool->WaitIdle();
    }

    uint32_t JobSystem::WorkerCount()
    {
        return s_Pool ? s_Pool->ThreadCount() : 0;
    }
}

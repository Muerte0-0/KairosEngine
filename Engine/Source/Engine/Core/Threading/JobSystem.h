#pragma once

#include "ThreadPool.h"
#include "Engine/Core/Base.h"

#include <atomic>
#include <functional>
#include <future>
#include <vector>

namespace Engine
{
    // -----------------------------------------------------------------------
    // Job priority — higher = dequeued first (not enforced in ThreadPool,
    // but used by JobSystem for ordering before submit).
    // -----------------------------------------------------------------------
    enum class JobPriority : uint8_t
    {
        IO      = 0,   // asset streaming, file reads
        Normal  = 1,   // ECS, physics, animation
        GPUPrep = 2,   // culling, batch building (still CPU work)
        High    = 3,   // urgent one-off tasks
    };

    // -----------------------------------------------------------------------
    // JobHandle — lightweight future wrapper.
    // Caller can .Wait() or ignore (fire-and-forget).
    // -----------------------------------------------------------------------
    struct JobHandle
    {
        std::shared_future<void> Future;

        bool IsValid() const { return Future.valid(); }
        void Wait()    const { if (Future.valid()) Future.wait(); }
        bool IsDone()  const
        {
            if (!Future.valid()) return true;
            return Future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        }
    };

    // -----------------------------------------------------------------------
    // JobSystem
    // Singleton facade over ThreadPool. Adds priority sorting and
    // dependency chaining (a job can declare prerequisite JobHandles).
    //
    // GPU RULE: Never submit Vulkan commands from jobs dispatched here.
    // GPU command recording jobs must write to intermediate data structures;
    // actual vkQueueSubmit happens on the render thread via RenderSubmitter.
    // -----------------------------------------------------------------------
    class JobSystem
    {
    public:
        // Must be called once before any Submit.
        static void Init(uint32_t workerCount = 0);
        static void Shutdown();

        // Fire-and-forget.
        static JobHandle Submit(std::function<void()> work,
                                JobPriority priority = JobPriority::Normal);

        // Submit with prerequisites — work runs only after all deps are done.
        // Blocking wait on deps is done on a short-lived helper thread so
        // worker threads are never blocked (avoids deadlocks).
        static JobHandle SubmitAfter(std::vector<JobHandle> deps,
                                     std::function<void()> work,
                                     JobPriority priority = JobPriority::Normal);

        // Submit a batch: same function called with indices [0, count).
        // Useful for ECS component batches. Returns one handle per element.
        static std::vector<JobHandle> SubmitBatch(uint32_t count,
                                                  std::function<void(uint32_t)> work,
                                                  JobPriority priority = JobPriority::Normal);

        // Block main thread until all pending jobs finish.
        static void WaitIdle();

        static uint32_t WorkerCount();

    private:
        static Scope<ThreadPool> s_Pool;
    };
}

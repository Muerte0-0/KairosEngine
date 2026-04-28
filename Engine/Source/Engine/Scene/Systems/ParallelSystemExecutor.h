#pragma once

#include "Engine/Core/Threading/JobSystem.h"
#include "Engine/Core/Threading/Fence.h"

#include <entt/entt.hpp>
#include <functional>
#include <vector>

namespace Engine
{
    // -----------------------------------------------------------------------
    // ParallelSystemExecutor
    //
    // Dispatches ECS systems across JobSystem workers.
    //
    // RULES:
    //   - Systems dispatched here MUST NOT touch GPU resources (no Vulkan).
    //   - Systems that only READ registry data can share a parallel group.
    //   - Systems that WRITE must either operate on disjoint component sets
    //     or be serialized with SubmitSerial().
    //
    // Usage:
    //   ParallelSystemExecutor exec;
    //   exec.SubmitParallel([&]{ TransformSystem::Update(reg, dt); });
    //   exec.SubmitParallel([&]{ PhysicsSystem::Integrate(reg, dt); });
    //   exec.Flush();   // wait for all parallel jobs
    //   exec.SubmitSerial([&]{ AnimationSystem::Blend(reg, dt); });
    //   exec.Flush();
    // -----------------------------------------------------------------------
    class ParallelSystemExecutor
    {
    public:
        // Queue a system to run concurrently with others in the current group.
        void SubmitParallel(std::function<void()> system)
        {
            m_Pending.push_back(JobSystem::Submit(std::move(system), JobPriority::Normal));
        }

        // Wait for all pending parallel jobs, then run system on calling thread.
        void SubmitSerial(std::function<void()> system)
        {
            Flush();
            system();
        }

        // Block until all queued parallel jobs complete.
        void Flush()
        {
            for (auto& h : m_Pending)
                h.Wait();
            m_Pending.clear();
        }

    private:
        std::vector<JobHandle> m_Pending;
    };
}

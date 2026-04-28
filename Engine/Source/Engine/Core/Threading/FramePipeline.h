#pragma once

#include "Engine/Core/Threading/TaskGraph.h"
#include "Engine/Core/Threading/JobSystem.h"

#include <functional>

namespace Engine
{
    // -----------------------------------------------------------------------
    // FramePipeline
    //
    // Owns the per-frame TaskGraph and provides named hook points that
    // match the engine's logical frame stages:
    //
    //   1. ECSUpdate       — parallel component systems (no GPU)
    //   2. Simulation      — physics / animation (no GPU)
    //   3. RenderExtract   — visibility cull, LOD, transform snapshot
    //   4. CommandBuild    — build render batches, sort draw calls (no submit)
    //   5. GPUSubmit       — (main thread only, not dispatched via TaskGraph)
    //
    // Usage (called once per frame by Application::Run):
    //   m_FramePipeline.SetECSUpdate([&](float dt){ ... });
    //   m_FramePipeline.SetSimulation([&](float dt){ ... });
    //   m_FramePipeline.SetRenderExtract([&]{ ... });
    //   m_FramePipeline.SetCommandBuild([&]{ ... });
    //   m_FramePipeline.Dispatch(deltaTime);   // submit to job system
    //   // ... main-thread GPU submit ...
    //   m_FramePipeline.Wait();                // ensure all CPU stages done
    //   m_FramePipeline.Reset();               // prepare for next frame
    // -----------------------------------------------------------------------
    class FramePipeline
    {
    public:
        using WorkFn   = std::function<void()>;
        using DtWorkFn = std::function<void(float)>;

        void SetECSUpdate    (DtWorkFn fn) { m_ECSUpdate     = std::move(fn); }
        void SetSimulation   (DtWorkFn fn) { m_Simulation    = std::move(fn); }
        void SetRenderExtract(WorkFn   fn) { m_RenderExtract = std::move(fn); }
        void SetCommandBuild (WorkFn   fn) { m_CommandBuild  = std::move(fn); }

        // Dispatch all CPU stages to the job system with proper ordering.
        //
        // Stage dependency graph:
        //   ECSUpdate ──┬──> Simulation
        //               └──> RenderExtract ──> CommandBuild
        //
        void Dispatch(float deltaTime)
        {
            m_Graph.Reset();

            if (m_ECSUpdate)
            {
                auto fn = m_ECSUpdate; // capture by value (dt baked in)
                m_Graph.AddStage("ECSUpdate", {}, [fn, deltaTime]{ fn(deltaTime); },
                                 JobPriority::Normal);
            }

            if (m_Simulation)
            {
                auto fn = m_Simulation;
                m_Graph.AddStage("Simulation", { "ECSUpdate" },
                                 [fn, deltaTime]{ fn(deltaTime); },
                                 JobPriority::Normal);
            }

            if (m_RenderExtract)
            {
                auto fn = m_RenderExtract;
                m_Graph.AddStage("RenderExtract", { "ECSUpdate" },
                                 fn, JobPriority::GPUPrep);
            }

            if (m_CommandBuild)
            {
                auto fn = m_CommandBuild;
                m_Graph.AddStage("CommandBuild", { "RenderExtract" },
                                 fn, JobPriority::GPUPrep);
            }

            m_Graph.Submit();
        }

        // Block until all dispatched CPU stages are complete.
        // Call before vkQueueSubmit on the render thread.
        void Wait()  { m_Graph.WaitAll(); }
        void Reset() { m_Graph.Reset();   }

        bool IsCommandBuildDone() const { return m_Graph.IsStageDone("CommandBuild"); }

    private:
        TaskGraph  m_Graph;
        DtWorkFn   m_ECSUpdate;
        DtWorkFn   m_Simulation;
        WorkFn     m_RenderExtract;
        WorkFn     m_CommandBuild;
    };
}

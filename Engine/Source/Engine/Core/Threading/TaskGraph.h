#pragma once

#include "JobSystem.h"
#include "Engine/Core/Base.h"

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

namespace Engine
{
    // -----------------------------------------------------------------------
    // TaskGraph — lightweight DAG of named stages over JobSystem.
    //
    // Designed for the frame pipeline. Stages declare their dependencies
    // by name; TaskGraph resolves ordering and submits jobs to JobSystem.
    //
    // Usage:
    //   TaskGraph graph;
    //   graph.AddStage("ECS_Update",     {},                 [&]{ ... });
    //   graph.AddStage("Physics",        {"ECS_Update"},     [&]{ ... });
    //   graph.AddStage("RenderExtract",  {"ECS_Update"},     [&]{ ... });
    //   graph.AddStage("CommandBuild",   {"RenderExtract"},  [&]{ ... });
    //   graph.Submit();     // dispatches all stages respecting deps
    //   graph.WaitAll();    // block until every stage done
    //
    // GPU RULE: Stages dispatched via TaskGraph must not call vkQueueSubmit.
    // Submit happens externally (RenderSubmitter / main render thread).
    // -----------------------------------------------------------------------

    struct TaskStage
    {
        std::string              Name;
        std::vector<std::string> Dependencies;  // names of stages that must finish first
        std::function<void()>    Work;
        JobPriority              Priority = JobPriority::Normal;
    };

    class TaskGraph
    {
    public:
        // Register a stage. Call before Submit().
        void AddStage(std::string name,
                      std::vector<std::string> deps,
                      std::function<void()> work,
                      JobPriority priority = JobPriority::Normal)
        {
            m_Stages.push_back({ std::move(name), std::move(deps), std::move(work), priority });
        }

        // Dispatch all stages to JobSystem, respecting dependency ordering.
        // Each stage waits on its declared dependency handles before running.
        void Submit()
        {
            m_Handles.clear();

            for (auto& stage : m_Stages)
            {
                // Collect prerequisite handles by name.
                std::vector<JobHandle> prereqs;
                prereqs.reserve(stage.Dependencies.size());
                for (const auto& depName : stage.Dependencies)
                {
                    auto it = m_Handles.find(depName);
                    if (it != m_Handles.end())
                        prereqs.push_back(it->second);
                    // Unknown dep name: silently skip (no-op dep).
                }

                JobHandle handle;
                if (prereqs.empty())
                    handle = JobSystem::Submit(stage.Work, stage.Priority);
                else
                    handle = JobSystem::SubmitAfter(std::move(prereqs), stage.Work, stage.Priority);

                m_Handles[stage.Name] = handle;
            }
        }

        // Block calling thread until every registered stage completes.
        void WaitAll()
        {
            for (auto& [name, handle] : m_Handles)
                handle.Wait();
        }

        // Clear stages and handles. Call between frames if reusing the graph.
        void Reset()
        {
            m_Stages.clear();
            m_Handles.clear();
        }

        // Check whether a named stage is done (non-blocking).
        bool IsStageDone(const std::string& name) const
        {
            auto it = m_Handles.find(name);
            return it == m_Handles.end() || it->second.IsDone();
        }

    private:
        std::vector<TaskStage>                    m_Stages;
        std::unordered_map<std::string, JobHandle> m_Handles;
    };
}

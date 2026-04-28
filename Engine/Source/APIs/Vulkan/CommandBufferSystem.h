#pragma once

#include "Engine/Core/Threading/JobSystem.h"
#include "Engine/Core/Threading/Fence.h"
#include "Engine/Core/Base.h"

#include <vulkan/vulkan_raii.hpp>
#include <functional>
#include <vector>

namespace Engine
{
    class VulkanDevice;

    // -----------------------------------------------------------------------
    // CommandBufferSystem
    //
    // Manages a pool of secondary command buffers for parallel recording.
    // Workers fill secondary buffers; the main render thread executes them
    // into the primary command buffer via vkCmdExecuteCommands.
    //
    // RULES:
    //   - Allocate secondary buffers once per frame (reuse ring per slot).
    //   - Workers call Record(index, fn) — fn receives the secondary cmdbuf.
    //   - After all Record jobs finish, main thread calls Execute(primary).
    //   - NO vkQueueSubmit from workers — only recording.
    //
    // Usage:
    //   // In FramePipeline CommandBuild stage (worker thread):
    //   g_CmdBufSystem.Record(chunkIndex, [](vk::CommandBuffer cb){ ... });
    //
    //   // On main render thread after FramePipeline::Wait():
    //   g_CmdBufSystem.Execute(*primaryCmdBuf);
    //   g_CmdBufSystem.Reset(frameIndex);
    // -----------------------------------------------------------------------
    class CommandBufferSystem
    {
    public:
        // Must be called once after VulkanDevice + CommandPool are ready.
        // bufferCount = max parallel secondary buffers (typically worker count).
        void Init(VulkanDevice* device,
                  vk::raii::CommandPool& commandPool,
                  uint32_t bufferCount,
                  uint32_t framesInFlight);
        void Shutdown();

        // Dispatch a recording job for slot [index] in the current frame.
        // fn receives a ready-to-record secondary vk::CommandBuffer.
        // Safe to call from any thread — recording itself is per-buffer.
        JobHandle Record(uint32_t frameIndex,
                         uint32_t bufferIndex,
                         vk::RenderingInfo renderingInfo,
                         std::function<void(vk::CommandBuffer)> fn);

        // Wait for all in-flight Record jobs for this frame, then
        // execute their secondary buffers into primaryCmdBuf.
        // Must be called on the MAIN render thread only.
        void Execute(uint32_t frameIndex, vk::CommandBuffer primaryCmdBuf);

        // Call after Execute() to reset secondary buffers for next use.
        void Reset(uint32_t frameIndex);

        uint32_t BufferCount() const { return m_BufferCount; }

    private:
        struct FrameBuffers
        {
            std::vector<vk::raii::CommandBuffer> Secondaries;
            std::vector<bool>                    Used;   // which slots were recorded
            std::vector<JobHandle>               Jobs;   // one per recorded slot
        };

        VulkanDevice*                m_Device      = nullptr;
        uint32_t                     m_BufferCount = 0;
        std::vector<FrameBuffers>    m_Frames;   // one per frame-in-flight slot
    };
}

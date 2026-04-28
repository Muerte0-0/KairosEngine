#include "kepch.h"
#include "CommandBufferSystem.h"

#include "Components/VulkanDevice.h"
#include "Engine/Debugging/Log.h"

namespace Engine
{
    void CommandBufferSystem::Init(VulkanDevice* device,
                                   vk::raii::CommandPool& commandPool,
                                   uint32_t bufferCount,
                                   uint32_t framesInFlight)
    {
        ASSERT(device, "CommandBufferSystem::Init — null device");
        m_Device      = device;
        m_BufferCount = bufferCount;

        m_Frames.resize(framesInFlight);
        for (auto& frame : m_Frames)
        {
            vk::CommandBufferAllocateInfo allocInfo{
                *commandPool,
                vk::CommandBufferLevel::eSecondary,
                bufferCount
            };
            frame.Secondaries = device->GetDevice().allocateCommandBuffers(allocInfo);
            frame.Used.assign(bufferCount, false);
            frame.Jobs.resize(bufferCount);
        }

        LOG(LogLevel::Info,
            "CommandBufferSystem: {} secondary buffers x {} frames allocated.",
            bufferCount, framesInFlight);
    }

    void CommandBufferSystem::Shutdown()
    {
        if (!m_Device) return;
        m_Device->WaitIdle();
        m_Frames.clear();
        m_Device = nullptr;
    }

    JobHandle CommandBufferSystem::Record(uint32_t frameIndex,
                                          uint32_t bufferIndex,
                                          vk::RenderingInfo renderingInfo,
                                          std::function<void(vk::CommandBuffer)> fn)
    {
        ASSERT(frameIndex  < m_Frames.size(),       "CommandBufferSystem: frameIndex out of range");
        ASSERT(bufferIndex < m_BufferCount,          "CommandBufferSystem: bufferIndex out of range");

        auto& frame = m_Frames[frameIndex];
        frame.Used[bufferIndex] = true;

        // Capture raw handle — vk::raii objects are not copyable.
        vk::CommandBuffer cb = *frame.Secondaries[bufferIndex];

        auto job = JobSystem::Submit([cb, renderingInfo, fn = std::move(fn)]() mutable
        {
            vk::CommandBufferInheritanceRenderingInfo inheritRender{};
            // Inherit color/depth format from renderingInfo if provided.
            // Caller is responsible for setting correct formats.

            vk::CommandBufferInheritanceInfo inheritInfo{};
            inheritInfo.pNext = &inheritRender;

            vk::CommandBufferBeginInfo beginInfo{
                vk::CommandBufferUsageFlagBits::eRenderPassContinue |
                vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
                &inheritInfo
            };

            cb.begin(beginInfo);
            fn(cb);
            cb.end();
        }, JobPriority::GPUPrep);

        frame.Jobs[bufferIndex] = job;
        return job;
    }

    void CommandBufferSystem::Execute(uint32_t frameIndex, vk::CommandBuffer primaryCmdBuf)
    {
        ASSERT(frameIndex < m_Frames.size(), "CommandBufferSystem: frameIndex out of range");
        auto& frame = m_Frames[frameIndex];

        // Wait for all recording jobs to finish.
        for (uint32_t i = 0; i < m_BufferCount; ++i)
            if (frame.Used[i])
                frame.Jobs[i].Wait();

        // Collect recorded secondary buffers.
        std::vector<vk::CommandBuffer> toExecute;
        toExecute.reserve(m_BufferCount);
        for (uint32_t i = 0; i < m_BufferCount; ++i)
            if (frame.Used[i])
                toExecute.push_back(*frame.Secondaries[i]);

        if (!toExecute.empty())
            primaryCmdBuf.executeCommands(toExecute);
    }

    void CommandBufferSystem::Reset(uint32_t frameIndex)
    {
        ASSERT(frameIndex < m_Frames.size(), "CommandBufferSystem: frameIndex out of range");
        auto& frame = m_Frames[frameIndex];
        for (uint32_t i = 0; i < m_BufferCount; ++i)
        {
            if (frame.Used[i])
                frame.Secondaries[i].reset();
            frame.Used[i] = false;
            frame.Jobs[i] = {};
        }
    }
}

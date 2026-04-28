#include "kepch.h"
#include "GPUFrameScheduler.h"

#include "Components/VulkanDevice.h"
#include "Components/VulkanCommand.h"
#include "Engine/Debugging/Log.h"

namespace Engine
{
    void GPUFrameScheduler::Init(VulkanDevice* device, VulkanCommand* command)
    {
        ASSERT(device,  "GPUFrameScheduler::Init — null device");
        ASSERT(command, "GPUFrameScheduler::Init — null command");
        m_Device  = device;
        m_Command = command;
        LOG(LogLevel::Info, "GPUFrameScheduler initialized (deferred deletion queue active).");
    }

    void GPUFrameScheduler::OnShutdown()
    {
        if (!m_Device) return;
        m_Device->WaitIdle();
        for (auto& slot : m_Slots)
            FlushSlot(slot);
        m_Device  = nullptr;
        m_Command = nullptr;
    }

    void GPUFrameScheduler::OnBeginFrame(uint32_t frameIndex)
    {
        // Fence wait already done by VulkanRenderAPI::BeginScene().
        // We just flush the deferred deletions for this slot — GPU is done with it.
        m_FrameIndex = frameIndex;
        FlushSlot(m_Slots[m_FrameIndex]);
    }

    void GPUFrameScheduler::DeferDelete(std::function<void()> fn)
    {
        m_Slots[m_FrameIndex].PendingDeletions.push_back(std::move(fn));
    }

    void GPUFrameScheduler::FlushSlot(Slot& slot)
    {
        for (auto& fn : slot.PendingDeletions)
            fn();
        slot.PendingDeletions.clear();
    }
}

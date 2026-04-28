#pragma once

#include <functional>
#include <vector>
#include <array>
#include <cstdint>

// GFS_MAX_FRAMES must match MAX_FRAMES_IN_FLIGHT in VulkanRenderAPI.h
constexpr uint32_t GFS_MAX_FRAMES = 3;

namespace Engine
{
    class VulkanDevice;
    class VulkanCommand;

    // -----------------------------------------------------------------------
    // GPUFrameScheduler
    //
    // Does NOT own fences or semaphores — those live in VulkanCommand.
    // Responsibility: deferred deletion queue (GPU-safe resource destruction).
    //
    // Lifecycle (called by VulkanRenderAPI):
    //   Init(device, command)        — store pointers
    //   OnBeginFrame(frameIndex)     — flush deletions for this slot
    //                                  (call AFTER the existing fence wait)
    //   DeferDelete(fn)              — queue destruction for current slot
    //   OnShutdown()                 — device.waitIdle, flush all slots
    //
    // Usage pattern in VulkanRenderAPI::BeginScene():
    //   // ... existing fence wait ...
    //   m_FrameScheduler.OnBeginFrame(m_CurrentFrameIndex);
    //
    // Usage pattern for GPU resource destruction:
    //   // Instead of deleting immediately:
    //   m_FrameScheduler.DeferDelete([buf = std::move(myBuffer)](){});
    // -----------------------------------------------------------------------
    class GPUFrameScheduler
    {
    public:
        void Init(VulkanDevice* device, VulkanCommand* command);
        void OnShutdown();

        // Must be called each frame after the existing in-flight fence wait,
        // before recording new commands for this slot.
        void OnBeginFrame(uint32_t frameIndex);

        // Queue fn() to run once GPU finishes the current frame slot.
        void DeferDelete(std::function<void()> fn);

        uint32_t CurrentFrameIndex() const { return m_FrameIndex; }

    private:
        struct Slot
        {
            std::vector<std::function<void()>> PendingDeletions;
        };

        VulkanDevice*                      m_Device  = nullptr;
        VulkanCommand*                     m_Command = nullptr;
        std::array<Slot, GFS_MAX_FRAMES>   m_Slots;
        uint32_t                           m_FrameIndex = 0;

        void FlushSlot(Slot& slot);
    };
}

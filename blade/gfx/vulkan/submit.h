#ifndef BLADE_GFX_VULKAN_SUBMIT_H
#define BLADE_GFX_VULKAN_SUBMIT_H

#include "gfx/vulkan/command.h"
#include "gfx/vulkan/common.h"
#include "gfx/vulkan/device.h"
#include <complex>
#include <memory>
#include <queue>
#include <stack>
namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            struct submit
            {
                [[nodiscard]] explicit submit(std::weak_ptr<class device> device)
                    : device { device }
                {}

                submit& set_wait_semaphores(u32 count, VkSemaphore* waits) noexcept
                {
                    wait_semaphores = waits;
                    wait_semaphore_count = count;
                    return *this;
                }

                submit& set_signal_semaphores(u32 count, VkSemaphore* signals) noexcept
                {
                    signal_semaphores = signals;
                    signal_semaphore_count = count;
                    return *this;
                }

                submit& set_wait_stages(VkPipelineStageFlags* stages) noexcept
                {
                    wait_stages = stages;
                    return *this;
                }

                submit& set_command_buffers(u32 count, VkCommandBuffer* buffers) noexcept
                {
                    command_buffer_count = count;
                    command_buffers = buffers;
                    return *this;
                }

                submit& set_p_next(void* next) noexcept
                {
                    p_next = next;
                    return *this;
                }

                VkResult submit_to_queue(const queue_type queue, VkFence fence) const noexcept
                {
                    VkSubmitInfo submit_info {
                        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                        .waitSemaphoreCount = wait_semaphore_count,
                        .pWaitSemaphores = wait_semaphores,
                        .pWaitDstStageMask = wait_stages,
                        .commandBufferCount = command_buffer_count,
                        .pCommandBuffers = command_buffers,
                        .signalSemaphoreCount = signal_semaphore_count,
                        .pSignalSemaphores = signal_semaphores
                    };

                    return vkQueueSubmit(
                        device.lock()->get_queue(queue).value(),
                        1,
                        &submit_info,
                        fence
                    );
                }

                std::weak_ptr<class device> device {};

                VkSemaphore* wait_semaphores       { nullptr};
                u32 wait_semaphore_count           { 0 };

                VkSemaphore* signal_semaphores       { nullptr};
                u32 signal_semaphore_count           { 0 };

                VkPipelineStageFlags* wait_stages  { nullptr };

                VkCommandBuffer* command_buffers   { nullptr };
                u32 command_buffer_count           { 0 };

                void* p_next                       { nullptr };
            };
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_SUBMIT_H

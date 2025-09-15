#ifndef BLADE_GFX_VULKAN_SWAPCHAIN_H
#define BLADE_GFX_VULKAN_SWAPCHAIN_H
#include "gfx/vulkan/common.h"
#include "gfx/vulkan/device.h"
#include "gfx/vulkan/surface.h"
#include <functional>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            // TODO: move to GFX system API
            /// @breif Supported presentation modes for swapchain configuration
            enum class present_mode 
            {
                /// @brief Triple-buffering. Newer images replace older ones in queue
                MAILBOX,

                /// @brief Traditional VSYNC. Submitting images to queue faster than presentation leads to waiting. Guaranteed support on vulkan
                FIFO,

                /// @brief Like `FIFO` but if application is late submitting, it presents immediately rather than waiting for vertical blank
                FIFO_RELAXED,

                /// @brief Images presented immedately without waiting for vertical blank. Can cause screen tearing, but provides lowest latency
                IMMEDIATE,
            };

            inline constexpr VkPresentModeKHR present_mode_to_vulkan(present_mode mode)
            {
                switch (mode)
                {
                    case present_mode::MAILBOX:
                        return VK_PRESENT_MODE_MAILBOX_KHR;
                    case present_mode::FIFO:
                        return VK_PRESENT_MODE_FIFO_KHR;
                    case present_mode::FIFO_RELAXED:
                        return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
                    case present_mode::IMMEDIATE:
                    default:
                        return VK_PRESENT_MODE_IMMEDIATE_KHR;
                }
            }
            class swapchain
            {
                public:
                    class builder
                    {
                        public:
                            [[nodiscard]] explicit builder(
                                std::weak_ptr<class device> device
                                , std::weak_ptr<const surface> surface
                            ) noexcept 
                                : info { device, surface }
                            {}

                            std::optional<std::unique_ptr<swapchain>> build() const noexcept;

                            builder& set_extent(struct width width, struct height height) noexcept;
                            builder& prefer_present_mode(present_mode mode) noexcept;
                            builder& prefer_format(VkFormat format) noexcept;
                            builder& request_min_image_count(u32 count) noexcept;
                            builder& require_image_usage(VkImageUsageFlagBits usage) noexcept;
                            builder& set_composite_alpha(VkCompositeAlphaFlagBitsKHR alpha) noexcept;
                            builder& set_clipped(VkBool32 clipped) noexcept;
                            builder& set_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept;

                            struct
                            {
                                std::weak_ptr<class device> device;
                                std::weak_ptr<const struct surface> surface;

                                present_mode preferred_present_mode { present_mode::FIFO };
                                VkAllocationCallbacks* allocation_callbacks { nullptr };
                                VkFormat preferred_format { VK_FORMAT_R8G8B8A8_UNORM };
                                u32 min_image_count { 0 };
                                VkImageUsageFlags image_usage { };
                                VkCompositeAlphaFlagBitsKHR composite_alpha { VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR };
                                VkBool32 clipped { VK_TRUE };
                                VkSwapchainKHR old_swapchain { VK_NULL_HANDLE };

                                u32 graphics_queue_index { 0 };
                                u32 present_queue_index { 0 };

                                struct
                                {
                                    struct width width   { 0 };
                                    struct height height { 0 };
                                } extent {};
                            } info;


                        private:
                            VkSurfaceFormatKHR select_surface_format_(const std::vector<VkSurfaceFormatKHR>& formats) const noexcept;
                            VkPresentModeKHR select_present_mode_(const std::vector<VkPresentModeKHR>& present_modes) const noexcept;
                            VkExtent2D select_extent_(const VkSurfaceCapabilitiesKHR& capabilities) const noexcept;
                    };

                    swapchain(std::weak_ptr<const class device> device) noexcept
                        : _device { device }
                    {}

                    void destroy() noexcept;

                    VkSwapchainKHR handle() const noexcept { return _swapchain; } 
                    VkExtent2D get_extent() const noexcept { return _extent; }
                    u32 num_image_views() const noexcept { return static_cast<u32>(_image_views.size()); }
                    const VkImageView& get_image_view(usize index) const noexcept { return _image_views[index]; }
                    const std::vector<VkImageView>& get_image_views() const noexcept { return _image_views; }
                    const VkImageView* get_image_views_raw() const noexcept { return _image_views.data(); }
                    VkFormat get_format() const noexcept { return _format; }
                    
                    [[nodiscard]] std::optional<u32> get_image_index(VkSemaphore semaphore, VkFence fence = VK_NULL_HANDLE) const noexcept;

                private:
                    std::weak_ptr<const class device> _device {};
                    std::vector<VkImage> _images {};
                    std::vector<VkImageView> _image_views {};
                    VkAllocationCallbacks* _allocation_callbacks { nullptr };
                    VkSwapchainKHR _swapchain { VK_NULL_HANDLE };
                    VkFormat _format {};
                    VkExtent2D _extent {};

                private:
                    std::optional<std::vector<VkImage>> create_images_() const noexcept;
                    std::optional<std::vector<VkImageView>> create_image_views_() const noexcept;

            };
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_SWAPCHAIN_H

#include "gfx/vulkan/renderer.h"
#include "core/types.h"
#include "gfx/handle.h"
#include "gfx/program.h"
#include "gfx/view.h"
#include "gfx/vulkan/common.h"
#include "gfx/vulkan/platform.h"
#include "gfx/vulkan/shader.h"
#include "gfx/vulkan/types.h"
#include "gfx/vulkan/utils.h"
#include <cstring>
#include <optional>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            vulkan_backend::vulkan_backend() noexcept
            {
                logger::debug("vulkan_backend constructor called");
            }

            vulkan_backend::~vulkan_backend() noexcept
            {
                logger::debug("vulkan_backend destructor called");
                if (_is_initialized)
                {
                    logger::warn("Vulkan backend destroying without explicit shutdown. Shutting down now.");
                    shutdown();
                }
            }

            bool vulkan_backend::init(const init_info& init) noexcept
            {
                logger::info("Initializing vulkan backend.");

                std::vector<const char*> extensions = {};
                std::vector<const char*> validation_layers = {};

                if (!init.headless) 
                {
                    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

                    auto platform_extensions = get_platform_extensions();
                    for (const auto& ext : platform_extensions)
                        extensions.push_back(ext);
                }

                // TODO: move to function
                if (init.enable_debug)
                {
                    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

                    validation_layers = get_debug_validation_layers().value();
                }

                auto instance_opt = instance::builder()
                    .set_engine_name("Blade Engine Name")
                    .set_application_name("Blade Application Name")
                    .set_engine_version(version { .major{1}, .minor{0}, .patch{0} })
                    .require_api_version(version { .major{1}, .minor{3}, .patch{0} })
                    .request_extensions(extensions)
                    .request_validation_layers(validation_layers)
                    .build();


                // auto instance_opt = instance::create();
                _instance = std::make_shared<class instance>(std::move(instance_opt.value()));

                auto builder = device::builder(_instance)
                    .require_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                    .set_allocation_callbacks(nullptr);

                auto device_opt = builder.build();
                
                _device = device_opt.value();

                if (init.enable_debug)
                {
                    _instance->create_debug_messenger();
                }

                VkAttachmentDescription color_attachment {
                    .format = VK_FORMAT_B8G8R8A8_SRGB,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                };
                
                VkAttachmentReference color_attachment_reference {
                    .attachment = 0,
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                };

                VkSubpassDescription subpass {
                    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                    .colorAttachmentCount = 1,
                    .pColorAttachments = &color_attachment_reference,
                };

                // auto device_opt = device::create(_instance, { .use_swapchain = true });
                // _device = device_opt.value();

                _is_initialized = true;
                return true;
            }

            std::vector<const char*> vulkan_backend::get_platform_extensions() const noexcept
            {
                std::vector<const char*> extensions {};
                #if defined(BLADE_PLATFORM_WINDOWS)
                extensions.push_back("VK_KHR_Win32_surface");
                #elif defined(BLADE_PLATFORM_LINUX)
                // NOTE: if using xcb, use xcb rather than xlib
                extensions.push_back("VK_KHR_xlib_surface");
                #endif

                return extensions;
            }

            std::optional<std::vector<const char*>> vulkan_backend::get_debug_validation_layers() const noexcept
            {
                std::vector<const char*> validation_layers { "VK_LAYER_KHRONOS_validation" };
                std::vector<VkLayerProperties> available_layers {};
                VkLayerProperties *dummy_layer_properties = nullptr;
                u32 available_layer_count = 0;
                

                VK_ASSERT(vkEnumerateInstanceLayerProperties(&available_layer_count, dummy_layer_properties));
                available_layers.resize(available_layer_count);
                VK_ASSERT(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data()));

                // TODO: move checking for validation layers into the instance::builder
                for (const auto& layer : validation_layers)
                {
                    bool found_layer = false;
                    logger::debug("Searching for validation layer {}", layer);

                    for (const auto& available_layer : available_layers)
                    {
                        if (strcmp(layer, available_layer.layerName) == 0)
                        {
                            found_layer = true;
                            logger::debug("Found.");
                            break;
                        }
                    }

                    if (!found_layer)
                    {
                        logger::error("Validation layer not found: {}", layer);
                        return std::nullopt;
                    }
                }

                return validation_layers;
            }

            bool vulkan_backend::shutdown() noexcept
            {
                logger::info("Vulkan backend shutting down");

                for (auto&& shader : _shaders)
                {
                    logger::info("Destroying shader...");
                    shader.second.destroy();
                    logger::info("Destroyed.");
                }

                for (auto&& view : _views)
                {
                    logger::info("Destroying view: {}", view.first.index);
                    view.second.destroy();
                    logger::info("Destroyed view {}.", view.first.index);
                }

                if (_device)
                {
                    _device->destroy();
                }

                _instance->destroy_debug_messenger();
                _instance->destroy();

                _is_initialized = false;
                return true;
            }

            framebuffer_handle vulkan_backend::create_framebuffer(framebuffer_create_info create_info) noexcept
            {
                logger::info("Creating framebuffer...");
                static u16 framebuffer_handle_id = 0;
                
                auto surface_opt = surface::create(_instance, create_info);
                if (!surface_opt.has_value())
                {
                    logger::error("Failed to create framebuffer");
                    return framebuffer_handle{.index = BLADE_NULL_HANDLE};
                }

                auto surface = surface_opt.value();

                vulkan_backend::view view = {
                    .device = _device,
                    .surface = surface,
                    .pipeline_builder = std::make_unique<pipeline::builder>(_device),
                };
                

                if (create_info.native_window_data)
                {
                    logger::info("Creating swapchain...");
                    view.swapchain = swapchain::builder(_device, view.surface)
                        .set_allocation_callbacks(nullptr)
                        .set_composite_alpha(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
                        .require_image_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                        .set_clipped(VK_TRUE)
                        .set_extent(create_info.width, create_info.height)
                        .prefer_present_mode(present_mode::MAILBOX)
                        .build();

                    if (!view.swapchain.has_value())
                    {
                        logger::error("Failed to create swapchain");
                        return framebuffer_handle{.index = BLADE_NULL_HANDLE};
                    }
                    
                    VkAttachmentDescription color_attachment {
                        .format = view.swapchain.value()->get_format(),
                        .samples = VK_SAMPLE_COUNT_1_BIT,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                    };
                    
                    VkAttachmentReference color_attachment_reference {
                        .attachment = 0,
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    };

                    VkSubpassDescription subpass {
                        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                        .colorAttachmentCount = 1,
                        .pColorAttachments = &color_attachment_reference,
                    };

                    view.renderpass = renderpass::builder(_device)
                        .add_subpass_description(subpass)
                        .add_attachment(color_attachment)
                        .build()
                        .value();


                    view.pipeline_builder.get()->set_extent(view.swapchain.value()->get_extent());

                    logger::info("Swapchain created.");
                }

                const framebuffer_handle handle {.index=framebuffer_handle_id};
                _views.insert(std::make_pair(handle, std::move(view)));

                framebuffer_handle_id += 1;
                
                logger::info("Created framebuffer.");
                return handle;
            }

            shader_handle vulkan_backend::create_shader(
                const std::vector<u8>& mem
            ) noexcept {
                static u16 shader_handle_id = 1;
                const auto shader_opt = shader::builder(*_device)
                    .use_allocation_callbacks(nullptr)
                    .set_code(mem)
                    .build();

                if (!shader_opt.has_value())
                {
                    return  { BLADE_NULL_HANDLE };
                }

                auto shader = shader_opt.value();

                shader_handle handle { .index = shader_handle_id };
                _shaders.insert(std::make_pair(handle, shader));

                shader_handle_id += 1;

                return handle;
            }

            void vulkan_backend::frame() noexcept
            {
            }

            void vulkan_backend::submit() noexcept
            {
                for (const auto& view_item : _views)
                {
                    auto handle = view_item.first;
                    const auto& view = view_item.second;
                }
            }

            bool vulkan_backend::view::create_framebuffers(std::weak_ptr<class renderpass> renderpass) noexcept
            {
                usize num_images = 1;
                if (swapchain.has_value())
                {
                    num_images = swapchain.value().get()->num_image_views();
                }

                // framebuffers.resize(num_images);
                // logger::info("Num Images: {}", framebuffers.size());

                for (usize i = 0; i < num_images; i++)
                {
                    logger::info("Creating Vulkan Framebuffer {}", i);
                    std::array<VkImageView, 1> attachments = {
                        swapchain.value().get()->get_image_view(i)
                    };

                    if (renderpass.lock()->handle() == VK_NULL_HANDLE)
                    {
                        logger::info("REND NULL");
                    }

                    VkFramebufferCreateInfo framebuffer_info {
                        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                        .renderPass = renderpass.lock()->handle(),
                        .attachmentCount = static_cast<u32>(attachments.size()),
                        .pAttachments = attachments.data(),
                        .width = swapchain.value().get()->get_extent().width,
                        .height = swapchain.value().get()->get_extent().height,
                        .layers = 1
                    };
                    logger::info("Framebuffer info");

                    VkFramebuffer framebuffer;
                    const VkResult result = vkCreateFramebuffer(
                        device.lock()->handle(),
                        &framebuffer_info,
                        allocation_callbacks,
                        &framebuffer
                    );
                    framebuffers.push_back(framebuffer);

                    if (result != VK_SUCCESS)
                    {
                        return false;
                    }
                }

                return true;
            }

            program_handle vulkan_backend::create_view_program(
                const framebuffer_handle framebuffer
                , const shader_handle vert
                , const shader_handle frag
            ) noexcept {
                logger::info("Creating program");
                static u16 program_handle_id = 1;

                const auto& view = _views.find(framebuffer);

                if (
                    _shaders.find(vert) == _shaders.end() 
                    || _shaders.find(frag) == _shaders.end()
                    || view == _views.end()
                ) {
                    return { BLADE_NULL_HANDLE };
                }

                logger::info("Found program and shaders");

                struct program program {
                    .vertex = vert,
                    .fragment = frag
                };

                view->second.program = program;

                program_handle handle { program_handle_id };

                _programs.insert(std::make_pair(handle, program));

                view->second.pipeline_builder.get()
                    ->add_shader(shader::type::vertex, _shaders.find(vert)->second.handle())
                    .add_shader(shader::type::fragment, _shaders.find(frag)->second.handle());

                logger::info("Added shaders to pipeline");

                view->second.create_framebuffers(view->second.renderpass);
                logger::info("Framebuffers created");

                view->second.graphics_pipeline = view->second.pipeline_builder
                    ->add_renderpass(view->second.renderpass->handle())
                    .build().value();

                program_handle_id += 1;

                return handle;
            }

            void vulkan_backend::view::destroy() noexcept
            {
                if (graphics_pipeline)
                {
                    logger::info("Destroying graphics pipeline...");
                    graphics_pipeline->destroy();
                    graphics_pipeline = nullptr;
                    logger::info("Destroyed.");
                }

                if (renderpass)
                {
                    logger::info("Destroying renderpass...");
                    renderpass->destroy();
                    renderpass = nullptr;
                    logger::info("Destroyed.");
                }
                
                for (const auto& framebuffer : framebuffers)
                {
                    vkDestroyFramebuffer(device.lock()->handle(), framebuffer, allocation_callbacks);
                }
                
                if (swapchain.has_value())
                {
                    swapchain.value()->destroy();
                }
                surface->destroy();
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace

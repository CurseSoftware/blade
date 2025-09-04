#include "gfx/vulkan/renderer.h"
#include "core/memory.h"
#include "core/types.h"
#include "gfx/handle.h"
#include "gfx/program.h"
#include "gfx/vertex.h"
#include "gfx/view.h"
#include "gfx/view.h"
#include "gfx/vulkan/buffer.h"
#include "gfx/vulkan/common.h"
#include "gfx/vulkan/platform.h"
#include "gfx/vulkan/shader.h"
#include "gfx/vulkan/types.h"
#include "gfx/vulkan/utils.h"
#include <cstdint>
#include <cstring>
#include <locale>
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

                logger::info("Creating command pool");
//                auto pool_res = command_pool::builder(_device)
//                    .use_allocation_callbacks(nullptr)
//                    .set_queue_family_index(_device->get_physical_device().lock()->graphics_queue_index().value())
//                    .build();

                _cmd_handler = std::make_shared<command_handler>(_device);
//                _command_pool = pool_res.value();
//                _command_pool->allocate_buffers(8);

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
                vkDeviceWaitIdle(_device->handle());

                if (_cmd_handler)
                {
                    _cmd_handler->destroy();
                }
                
                for (auto&& buffer : _index_input_infos)
                {
                    buffer.second->destroy();
                }

                for (auto&& buffer : _vertex_input_infos)
                {
                    buffer.second->destroy();
                }

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

//                logger::info("Destroying command pool...");
//                if (_command_pool)
//                {
//                    _command_pool->destroy();
//                }
//                logger::info("Destroyed.");

                logger::info("Destroying device...");
                if (_device)
                {
                    _device->destroy();
                }
                logger::info("Destroyed.");

                _instance->destroy_debug_messenger();
                _instance->destroy();

                _is_initialized = false;
                return true;
            }

            framebuffer_handle vulkan_backend::create_framebuffer(framebuffer_create_info create_info) noexcept
            {
                logger::info("Creating framebuffer...");
                static u16 framebuffer_handle_id = 0;

                auto view_opt = view::create(_instance, _device, create_info);
                if (!view_opt.has_value())
                {
                    logger::info("Failed to create view");
                    return { BLADE_NULL_HANDLE };
                }

                auto view = std::move(view_opt.value());

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
                for (auto&& view: _views)
                {
                    view.second.frame();
                }
            }

            void vulkan_backend::set_viewport(const framebuffer_handle framebuffer, f32 x, f32 y, struct width width, struct height height) noexcept
            {
                auto view_it = _views.find(framebuffer);
                if (view_it == _views.end())
                {
                    logger::info("SetViewport framebuffer not found");
                    return;
                }

                view_it->second.set_viewport(x, y, width, height);
            }

            void vulkan_backend::submit() noexcept
            {
                for (const auto& view_item : _views)
                {
                    auto handle = view_item.first;
                    const auto& view = view_item.second;
                }
            }

            void view::record_commands(class command_buffer& command_buffer) const noexcept
            {
                auto recording = command_buffer.begin();

                u32 vertex_count = 3,
                    instance_count = 1,
                    first_vertex = 0,
                    first_instance = 0;

                std::vector<VkClearValue> clear_values(2);
                clear_values[0].color = {{ 0.f, 0.f, 0.f, 0.f }};
                clear_values[1].depthStencil = { 1.f, 0 };

                VkRect2D render_area {
                    .offset = { 0, 0 },
                    .extent = get_extent(),
                };
                
                auto pass = recording->begin_renderpass(renderpass, framebuffers[current_image_index], clear_values, render_area);
                pass.bind_pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline->handle());
                pass.set_viewport(viewport);
                pass.set_scissor(render_area);
                pass.bind_vertex_buffers(buffer.lock()->handle_ptr());
                pass.draw(vertex_count, instance_count, first_vertex, first_instance);

                if (index_buffer != nullptr)
                {
                    u32 index_count = 6
                        , first_index = 0
                        , first_instance = 0;
                    i32 vertex_offset = 0;
                    pass.bind_index_buffers(index_buffer->handle(), index_buffer->size());
                    pass.draw_indexed(index_count, instance_count, first_index, vertex_offset, vertex_offset);
                }

                pass.end();

                command_buffer.end();
            }

            VkExtent2D view::get_extent() const noexcept
            {
                if (swapchain.has_value())
                {
                    return swapchain.value()->get_extent();
                }

                return VkExtent2D {
                    .width = static_cast<u32>(viewport.width),
                    .height = static_cast<u32>(viewport.height),
                };
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

                program_handle handle { program_handle_id };

                _programs.insert(std::make_pair(handle, program));

                view->second.create_program(program, _shaders.find(vert)->second, _shaders.find(frag)->second);

                program_handle_id += 1;

                return handle;
            }

            void vulkan_backend::attach_vertex_buffer(const buffer_handle handle) noexcept
            {
                if (handle.index > _vertex_input_infos.size()-1)
                {
                    return;
                }

                // TODO: allow specifying of views
                auto first_view = _views.begin();
                if (first_view != _views.end())
                {
                    first_view->second.attach_vertex_buffer(_vertex_input_infos[handle]);
                }
            }

            void vulkan_backend::set_index_buffer(const buffer_handle handle) noexcept
            {
                auto first_view = _views.begin();
                if (first_view != _views.end())
                {
                    auto index = _index_input_infos[handle];
                    if (!index)
                        logger::error("NO INDEX");
                    first_view->second.set_index_buffer(_index_input_infos[handle]);
                }
            }

            void vulkan_backend::set_vertex_buffer(const buffer_handle handle) noexcept
            {
                auto first_view = _views.begin();
                if (first_view != _views.end())
                {
                    first_view->second.set_vertex_buffer(_vertex_input_infos[handle]);
                }
            }
            
            static const VkFormat vertex_formats[][4][2] =
            {
                { // Uint8
                    { VK_FORMAT_R8_UINT,                 VK_FORMAT_R8_UNORM                 },
                    { VK_FORMAT_R8G8_UINT,               VK_FORMAT_R8G8_UNORM               },
                    { VK_FORMAT_R8G8B8A8_UINT,           VK_FORMAT_R8G8B8A8_UNORM           },
                    { VK_FORMAT_R8G8B8A8_UINT,           VK_FORMAT_R8G8B8A8_UNORM           },
                },
                { // Int16
                    { VK_FORMAT_R16_SINT,                VK_FORMAT_R16_SNORM                },
                    { VK_FORMAT_R16G16_SINT,             VK_FORMAT_R16G16_SNORM             },
                    { VK_FORMAT_R16G16B16_SINT,          VK_FORMAT_R16G16B16_SNORM          },
                    { VK_FORMAT_R16G16B16A16_SINT,       VK_FORMAT_R16G16B16A16_SNORM       },
                },
                { // Float
                    { VK_FORMAT_R32_SFLOAT,              VK_FORMAT_R32_SFLOAT               },
                    { VK_FORMAT_R32G32_SFLOAT,           VK_FORMAT_R32G32_SFLOAT            },
                    { VK_FORMAT_R32G32B32_SFLOAT,        VK_FORMAT_R32G32B32_SFLOAT         },
                    { VK_FORMAT_R32G32B32A32_SFLOAT,     VK_FORMAT_R32G32B32A32_SFLOAT      },
                },
                { // Uint10
                    { VK_FORMAT_A2R10G10B10_UINT_PACK32, VK_FORMAT_A2R10G10B10_UNORM_PACK32 },
                    { VK_FORMAT_A2R10G10B10_UINT_PACK32, VK_FORMAT_A2R10G10B10_UNORM_PACK32 },
                    { VK_FORMAT_A2R10G10B10_UINT_PACK32, VK_FORMAT_A2R10G10B10_UNORM_PACK32 },
                    { VK_FORMAT_A2R10G10B10_UINT_PACK32, VK_FORMAT_A2R10G10B10_UNORM_PACK32 },
                },
                { // Half
                    { VK_FORMAT_R16_SFLOAT,              VK_FORMAT_R16_SFLOAT               },
                    { VK_FORMAT_R16G16_SFLOAT,           VK_FORMAT_R16G16_SFLOAT            },
                    { VK_FORMAT_R16G16B16_SFLOAT,        VK_FORMAT_R16G16B16_SFLOAT         },
                    { VK_FORMAT_R16G16B16A16_SFLOAT,     VK_FORMAT_R16G16B16A16_SFLOAT      },
                },
            };

            buffer_handle vulkan_backend::create_index_buffer(const core::memory* memory) noexcept
            {
                buffer_handle handle = { _buffer_handle_index };

                auto staging_buffer_opt = buffer::builder(_device)
                    .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
                    .set_size(memory->size)
                    .set_allocation_callbacks(nullptr)
                    .build();

                if (!staging_buffer_opt.has_value())
                {
                    logger::error("FAILED TO CREATE STAGING BUFFER");
                    return { BLADE_NULL_HANDLE };
                }

                auto staging_buffer = staging_buffer_opt.value();
                staging_buffer->allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                staging_buffer->map_memory(memory->data);
                
                auto index_buffer_opt = buffer::builder(_device)
                    .set_size(memory->size)
                    .set_usage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                    .set_allocation_callbacks(nullptr)
                    .build();

                if (!index_buffer_opt.has_value())
                {
                    return { BLADE_NULL_HANDLE };
                }
                
                auto index_buffer = index_buffer_opt.value();
                index_buffer->allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                // class command_buffer command_buffer(_command_pool->acquire_command_buffer());
                class command_buffer command_buffer(_cmd_handler->acquire_command_buffer());
                command_buffer.begin()
                    ->begin_transfer()
                    .copy_buffers(staging_buffer->handle(), index_buffer->handle(), memory->size);
                command_buffer.end();

                VkResult submit_result = _cmd_handler->submit_buffer(
                    command_buffer.handle()
                    , _device->get_queue(queue_type::transfer).value()
                );
//                _command_pool->submit_buffer(
//                    command_buffer.handle()
//                    , _device->get_queue(queue_type::transfer).value()
//                );
                _cmd_handler->update();
                // _command_pool->update();
                vkQueueWaitIdle(_device->get_queue(queue_type::transfer).value());
                staging_buffer->destroy();

                _index_input_infos[handle] = index_buffer;

                _buffer_handle_index++;
                logger::info("INDEX HANDLE: {}", handle.index);
                return handle;
            }

            buffer_handle vulkan_backend::create_vertex_buffer(const core::memory* memory, const vertex_layout& layout) noexcept
            {
                buffer_handle handle { _buffer_handle_index };

                auto staging_buffer_opt = buffer::builder(_device)
                    .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
                    .set_size(memory->size)
                    .set_allocation_callbacks(nullptr)
                    .build();

                if (!staging_buffer_opt.has_value())
                {
                    logger::error("FAILED TO CREATE STAGING BUFFER");
                    return { BLADE_NULL_HANDLE };
                }

                auto staging_buffer = staging_buffer_opt.value();
                staging_buffer->allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                staging_buffer->map_memory(memory->data);
                
                auto vertex_buffer_opt = buffer::builder(_device)
                    .set_size(memory->size)
                    .set_usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
                    .set_allocation_callbacks(nullptr)
                    .build();

                if (!vertex_buffer_opt.has_value())
                {
                    logger::error("FAILED TO CREATE VERTEX BUFFER");
                    return { BLADE_NULL_HANDLE };
                }

                auto vertex_buffer = vertex_buffer_opt.value();
                vertex_buffer->allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                logger::info("STRIDE: {}", layout.stride());

                class command_buffer command_buffer(_cmd_handler->acquire_command_buffer());
                // class command_buffer command_buffer(_command_pool->acquire_command_buffer());
                command_buffer.begin()
                    ->begin_transfer()
                    .copy_buffers(staging_buffer->handle(), vertex_buffer->handle(), memory->size);
                command_buffer.end();

                VkResult submit_result = _cmd_handler->submit_buffer(
                    command_buffer.handle()
                    , _device->get_queue(queue_type::transfer).value()
                );
//                _command_pool->submit_buffer(
//                    command_buffer.handle()
//                    , _device->get_queue(queue_type::transfer).value()
//                );
//                _command_pool->update();
                _cmd_handler->update();
                vkQueueWaitIdle(_device->get_queue(queue_type::transfer).value());
                
                vertex_buffer->set_input_binding_description(VkVertexInputBindingDescription {
                    .binding = _num_bindings,
                    .stride = layout.stride(),
                    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                });

                for (usize i = 0; i < layout.attributes().size(); i++)
                {
                    const attribute& attr = layout.attributes()[i];
                    VkVertexInputAttributeDescription desc {
                        .location = static_cast<u32>(attr.semantic),
                        .binding = _num_bindings,
                        .format = vertex_formats[static_cast<u32>(attr.type)][attr.count-1][0],
                        .offset = attr.offset
                    };
                    
                    logger::debug("Attribute \"{}\" offset: {}, location: {}, format: {}"
                            , attr.name
                            , attr.offset
                            , static_cast<u32>(attr.semantic)
                            , vk_vertex_format_str(vertex_formats[static_cast<u32>(attr.type)][attr.count-1][0])
                    );
                    vertex_buffer->add_input_attribute_description(desc);
                }

                staging_buffer->destroy();

                _vertex_input_infos.insert(std::make_pair(handle, vertex_buffer));

                _num_bindings++;
                
                _buffer_handle_index++;
                return handle;
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace

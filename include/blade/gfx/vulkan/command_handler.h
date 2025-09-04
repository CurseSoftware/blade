#ifndef BLADE_GFX_VULKAN_COMMAND_HANDLER_H
#define BLADE_GFX_VULKAN_COMMAND_HANDLER_H
#include "gfx/vulkan/common.h"
#include "gfx/vulkan/command.h"

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            class command_handler
            {
                public:
                    [[nodiscard]] explicit command_handler(std::weak_ptr<class device> device) noexcept;

                    /**
                     * @brief Submit a command buffer after being recorded
                     */
                    [[nodiscard]] VkResult submit_buffer(
                        VkCommandBuffer buffer
                        , VkQueue queue
                        , const VkSemaphore* wait_semaphores = nullptr
                        , u32 wait_semaphore_count = 0
                        , const VkSemaphore* signal_semaphores = nullptr
                        , u32 signal_semaphore_count = 0
                        , VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
                    ) const noexcept;

                    /**
                     * @brief Update the command buffers and free list
                     */
                    void update() noexcept;

                    /**
                     * @brief Retrieve an available command buffer from the free list
                     * @return `VkCommandBuffer` command buffer if available. `VK_NULL_HANDLE` if none free
                     */
                    VkCommandBuffer acquire_command_buffer() noexcept;
                    
                    /**
                     * @brief Wait on a command buffer to be ready
                     */
                    void wait_for_command_buffer(VkCommandBuffer buffer) const noexcept;
                    
                    /**
                     * @brief Reset the fence for a command buffer
                     */
                    void reset_command_buffer_fence(VkCommandBuffer) const noexcept;

                    /**
                     * @brief Destroy created resources and other shutdown behavior
                     */
                    void destroy() noexcept;

                public:
                    /**
                     * @brief Free list for holding and retrieving available command buffers
                     */
                    class buffer_free_list
                    {
                        public:
                            /**
                             * @brief Node in free list with buffer and fence
                             */
                            struct node
                            {
                                VkCommandBuffer command_buffer { VK_NULL_HANDLE };
                                VkFence fence                  { VK_NULL_HANDLE };
                                bool is_submitted              { false };
                                node* next                     { nullptr };

                                [[nodiscard]] explicit node(
                                    VkCommandBuffer buffer
                                    , VkFence fence
                                ) noexcept
                                    : command_buffer{ buffer }
                                    , fence{ fence }
                                {}
                            };

                            /**
                             * @brief Push a node to the front of the free list
                             * @param node The node to place at the front
                             */
                            void push_front(node* buffer_node) noexcept;

                            /**
                             * @brief Get the first node in the free list and remove it from the list
                             * @return `node*` of the first node AND `nullptr` if no valid nodes
                             */
                            [[nodiscard]] node* pop_front() noexcept;

                            [[nodiscard]] bool is_empty() const noexcept;

                        private:
                            node* _front { nullptr };
                    };
                private:
                    std::optional<VkFence> create_fence_() const noexcept;
                    void process_completed_buffers_() noexcept;

                private:
                    std::weak_ptr<device> _device                                              {};
                    std::shared_ptr<command_pool> _command_pool                                { nullptr };
                    std::unordered_map<VkCommandBuffer, buffer_free_list::node*> _active_nodes {};
                    std::vector<std::unique_ptr<buffer_free_list::node>> _all_buffer_nodes     {};
                    std::vector<VkCommandBuffer> _all_command_buffers                          {};
                    buffer_free_list _free_list                                                {};
            };
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_COMMAND_HANDLER_H

#include "vulkanapp/VulkanCleanupQueue.h"
#include <ranges>

std::vector<std::function<void()>> vulkanapp::VulkanCleanupQueue::cleanup_functions;

void vulkanapp::VulkanCleanupQueue::push_cleanup_function(std::function<void()>&& function)
{
    cleanup_functions.push_back(std::move(function));
}

void vulkanapp::VulkanCleanupQueue::flush()
{
    for (auto& cleanup_function : std::views::reverse(cleanup_functions))
    {
        cleanup_function();
    }
            
    cleanup_functions.clear();
}

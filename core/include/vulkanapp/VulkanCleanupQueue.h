#pragma once
#include <functional>

#define CLEANUP_FUNCTION(function_call) \
[this]() { function_call; }

//Cleanups resources
namespace vulkanapp
{
    class VulkanCleanupQueue
    {
    public:
        // Pushes a cleanup function to the queue.
        static void push_cleanup_function(std::function<void()>&& function);

        static void flush();

    private:
        static std::vector<std::function<void()>> cleanup_functions;
    };
}

#pragma once

#if defined(_DEBUG)
#define ENABLE_VALIDATION
#endif

#if defined(WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
#include <Windows.h>
#endif

#include <vulkan/vulkan.h>
#include <vector>
#include <functional>

#include "core/log.h"


VK_DEFINE_HANDLE(VmaAllocator);
VK_DEFINE_HANDLE(VmaAllocation);

namespace vg::vk
{
    
    template<typename Type> class __VkObject
    {
    public:
		__VkObject() {}

        operator Type(){
            return handle;
        }

        operator bool()
        {
            return handle != VK_NULL_HANDLE;
        }

        bool isValid()
        {
            return handle != VK_NULL_HANDLE;
        }

        __VkObject& operator = (const __VkObject&) = delete;
        __VkObject(const __VkObject&) = delete;

        __VkObject&& operator = (const __VkObject&&) = default;
        __VkObject(const __VkObject&&) = default;
    protected:
        Type handle = VK_NULL_HANDLE;
    };

} // name

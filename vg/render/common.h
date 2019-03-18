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

namespace vg
{
    
    template<typename Type> class __VkObject
    {
    public:
		__VkObject() {}
		__VkObject(Type& obj) : handle(obj) {}

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

		const Type* getPtr() const
		{
			return &handle;
		}

        __VkObject& operator = (const __VkObject&) = delete;
        __VkObject(const __VkObject&) = delete;
    protected:
        Type handle = VK_NULL_HANDLE;
    };

} // name

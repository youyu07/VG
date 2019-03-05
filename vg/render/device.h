#pragma once


#include "common.h"

namespace vg::vk
{
    
    class Device : public __VkObject<VkDevice>
    {
    public:
        Device();

        
        class CommandPool* createCommandPool();

		Image* createImage(const ImageInfo& info);

		inline VmaAllocator getAllocator() const {
			return allocator;
		}
    private:
        class Instance* instance;
        class PhysicalDevice* physicalDevice;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
		uint32_t queueFamilyIndex = 0;

		VmaAllocator allocator;
		class CommandPool* commandPool = nullptr;
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    };

} // name

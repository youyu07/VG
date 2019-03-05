#include "device.h"
#include "instance.h"
#include "commandBuffer.h"
#include "buffer.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace vg::vk
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
	{
		if (messageSeverity& VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			log_warning(callbackData->pMessage);
		}
		else if (messageSeverity& VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			log_error(callbackData->pMessage);
		}
		else
		{
			log_info(callbackData->pMessage);
		}

		return VK_FALSE;
	}

    Device::Device()
	{
		instance = new Instance(true,DebugMessengerCallback);
		if(!instance->isValid())return;
		physicalDevice = new PhysicalDevice(*instance);
		if(!physicalDevice->isValid())return;

		float priorities = 1.0f;
		queueFamilyIndex = physicalDevice->getQueueFamily();
		VkDeviceQueueCreateInfo queueInfos = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		queueInfos.queueCount = 1;
		queueInfos.queueFamilyIndex = queueFamilyIndex;
		queueInfos.pQueuePriorities = &priorities;

		const char* extension[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkDeviceCreateInfo info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		info.enabledExtensionCount = 1;
		info.ppEnabledExtensionNames = extension;
		info.queueCreateInfoCount = 1;
		info.pQueueCreateInfos = &queueInfos;

		if (vkCreateDevice(*physicalDevice, &info, nullptr, &handle) != VK_SUCCESS) {
			log_error("Failed to create device!");
			return;
		}

		{
			vkGetDeviceQueue(handle, queueFamilyIndex, 0, &graphicsQueue);
			if (graphicsQueue == VK_NULL_HANDLE) {
				log_error("Faild to get graphics queue.");
				return;
			}

			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.physicalDevice = *physicalDevice;
			allocatorInfo.device = handle;

			vmaCreateAllocator(&allocatorInfo, &allocator);

			commandPool = createCommandPool();

			std::vector<VkDescriptorPoolSize> poolSize = {
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1000},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1000},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1000},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,1000},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,1000}
			};

			VkDescriptorPoolCreateInfo descriptorPoolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
			descriptorPoolInfo.maxSets = static_cast<uint32_t>(poolSize.size() * 1000);
			descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
			descriptorPoolInfo.pPoolSizes = poolSize.data();
			vkCreateDescriptorPool(handle,&descriptorPoolInfo,nullptr,&descriptorPool);
		}
	}

	CommandPool* Device::createCommandPool()
	{
		return new CommandPool(handle,graphicsQueue,queueFamilyIndex);
	}

	Image* Device::createImage(const ImageInfo& info)
	{
		
	}
} // 

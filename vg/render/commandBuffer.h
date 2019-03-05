#pragma once
#include "common.h"

namespace vg::vk
{
	class CommandBuffer;

    class CommandPool : public __VkObject<VkCommandPool>
	{
		VkDevice device = VK_NULL_HANDLE;
		VkQueue queue = VK_NULL_HANDLE;
		uint32_t queueFamilyIndex = 0;
	public:
		CommandPool(VkDevice dev,VkQueue queue,uint32_t index) : device(dev), queue(queue),queueFamilyIndex(index)
		{
			VkCommandPoolCreateInfo info = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
			info.queueFamilyIndex = index;
			info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			vkCreateCommandPool(device, &info, nullptr, &handle);
		}

		~CommandPool()
		{
			if(handle != VK_NULL_HANDLE)vkDestroyCommandPool(device, handle, nullptr);
		}

		inline const uint32_t getFamilyIndex() const
		{
			return queueFamilyIndex;
		}

		inline const VkQueue getQueue() const
		{
			return queue;
		}
	};


	class CommandBuffer : public __VkObject<VkCommandBuffer>
	{
		VkDevice device = VK_NULL_HANDLE;
		CommandPool* pool = nullptr;
	public:
		CommandBuffer(VkDevice dev,CommandPool* pool) : device(dev),pool(pool)
		{
			VkCommandBufferAllocateInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
			info.commandBufferCount = 1;
			info.commandPool = *pool;
			info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			
			vkAllocateCommandBuffers(device, &info, &handle);
		}

		~CommandBuffer()
		{
			if (handle != VK_NULL_HANDLE)vkFreeCommandBuffers(device, *pool, 1,&handle);
		}

		void submit(bool waitIdle = true) {
			auto queue = pool->getQueue();

			VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &handle;

			vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

			if (waitIdle) {
				vkQueueWaitIdle(queue);
			}
		}

		void begin(VkCommandBufferUsageFlags flag = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
		{
			VkCommandBufferBeginInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
			info.flags = flag;

			vkBeginCommandBuffer(handle, &info);
		}

		void end()
		{
			vkEndCommandBuffer(handle);
		}
	};

} // name

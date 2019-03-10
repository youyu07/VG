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

		CommandBuffer* createCommandBuffer();

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

		void submit(VkSemaphore wait = VK_NULL_HANDLE,VkSemaphore signal = VK_NULL_HANDLE,const VkPipelineStageFlags waitMask = 0,VkFence fence = VK_NULL_HANDLE) 
		{
			auto queue = pool->getQueue();

			VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &handle;
			submitInfo.waitSemaphoreCount = wait == VK_NULL_HANDLE ? 0 : 1;
			submitInfo.pWaitSemaphores = wait == VK_NULL_HANDLE ? nullptr : &wait;
			submitInfo.signalSemaphoreCount = signal == VK_NULL_HANDLE ? 0 : 1;
			submitInfo.pSignalSemaphores = signal == VK_NULL_HANDLE ? nullptr : &signal;
			submitInfo.pWaitDstStageMask = waitMask == 0 ? nullptr : &waitMask;

			vkQueueSubmit(queue, 1, &submitInfo, fence);
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

		void beginRenderPass(VkRenderPass renderPass, VkFramebuffer frameBuffer,const VkRect2D& area,const std::vector<VkClearValue>& clearValue)
		{
			VkRenderPassBeginInfo info = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
			info.clearValueCount = static_cast<uint32_t>(clearValue.size());
			info.pClearValues = clearValue.data();
			info.framebuffer = frameBuffer;
			info.renderPass = renderPass;
			info.renderArea = area;

			vkCmdBeginRenderPass(handle, &info, VK_SUBPASS_CONTENTS_INLINE);
		}

		void endRenderPass()
		{
			vkCmdEndRenderPass(handle);
		}

		void bindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS)
		{
			vkCmdBindPipeline(handle, bindPoint, pipeline);
		}

		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0)
		{
			vkCmdDraw(handle, vertexCount, instanceCount, firstVertex, firstInstance);
		}

		template<typename T> void setViewport(T width,T height)
		{
			VkViewport viewport = {0.0f,static_cast<float>(height),static_cast<float>(width),-static_cast<float>(height),0.0f,1.0f};
			vkCmdSetViewport(handle, 0, 1, &viewport);
		}

		template<typename T1,typename T2> void setScissor(T1 x,T1 y,T2 width,T2 height)
		{
			VkRect2D scissor = { {static_cast<int32_t>(x),static_cast<int32_t>(y)},{static_cast<uint32_t>(width),static_cast<uint32_t>(height)} };
			vkCmdSetScissor(handle, 0, 1, &scissor);
		}
	};


	inline CommandBuffer* CommandPool::createCommandBuffer() {
		return new CommandBuffer(device, this);
	}
} // name

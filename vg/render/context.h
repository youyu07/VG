#pragma once

//#include <vku.hpp>
#include "vk/vkt.h"

namespace vg
{
	class Context_T
	{
		vk::Instance instance;
		vk::Device device;
		vk::DescriptorPool descriptorPool;
		vk::CommandPool commandPool;
		vk::Surface surface;
		vk::Swapchain swapchain;

		uint32_t graphicsQueueFamilyIndex = ~0;
		uint32_t computerQueueFamilyIndex = ~0;
		vk::Queue graphicsQueue;
		vk::Queue computerQueue;

		vk::RenderPass renderPass;

		vk::Image depth;
		struct
		{
			vk::Image color;
			vk::Image depth;
		}multiSample;

		std::vector<vk::FrameBuffer> frameBuffers;
		std::vector<vk::CommandBuffer> commandBuffers;

		VkSampleCountFlagBits sampeCount = VK_SAMPLE_COUNT_8_BIT;
		VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
		VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
	public:
		Context_T(const void* windowHandle)
		{
			vk::InstanceMaker im;
			instance = im.create();

			surface = instance->createSurface(windowHandle);

			auto gpus = instance->getPhysicalDevice();
			VkPhysicalDevice physicalDevice = gpus[0];

			{
				auto queueProps = vk::getQueueFamilyProperties(physicalDevice);
				VkQueueFlags search = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
				for (uint32_t qi = 0; qi != queueProps.size(); ++qi) {
					auto& qprop = queueProps[qi];
					std::cout << std::to_string(qprop.queueFlags) << "\n";
					if ((qprop.queueFlags & search) == search) {
						graphicsQueueFamilyIndex = qi;
						computerQueueFamilyIndex = qi;
						break;
					}
				}

				if (graphicsQueueFamilyIndex == ~0 || computerQueueFamilyIndex == ~0) {
					log_error("oops, missing a queue\n");
					return;
				}
			}

			if (!vk::getSurfaceSupport(physicalDevice, graphicsQueueFamilyIndex, *surface)) {
				log_error("surface not support");
			}

			vk::DeviceMaker dm;
			dm.extension(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
			VkPhysicalDeviceFeatures feature = {};
			feature.wideLines = VK_TRUE;
			feature.geometryShader = VK_TRUE;
			feature.fillModeNonSolid = VK_TRUE;
			dm.features(feature);
			dm.queue(graphicsQueueFamilyIndex);
			if (computerQueueFamilyIndex != graphicsQueueFamilyIndex) {
				dm.queue(computerQueueFamilyIndex);
			}
			device = dm.create(physicalDevice);

			auto prop = device->getPhysicalDeviceProperties();
			log_info("Use device : ", prop.deviceName);
			log_info("Max memory allocation count : ", prop.limits.maxMemoryAllocationCount);

			graphicsQueue = device->getQueue(graphicsQueueFamilyIndex);
			if (computerQueueFamilyIndex == graphicsQueueFamilyIndex) {
				computerQueue = graphicsQueue->clone();
			}
			else {
				computerQueue = device->getQueue(computerQueueFamilyIndex);
			}

			descriptorPool = device->createDescriptorPool();
			commandPool = device->createCommandPool(graphicsQueueFamilyIndex);
			
			swapchain = device->createSwapchain(surface);
			colorFormat = swapchain->getColorFormat();
			vk::RenderpassMaker rm;
			rm.attachmentBegin(colorFormat,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			rm.attachmentSamples(sampeCount);
			rm.attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
			rm.attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE);

			rm.attachmentBegin(colorFormat, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			rm.attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE);

			rm.attachmentBegin(depthFormat,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			rm.attachmentSamples(sampeCount);
			rm.attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);

			rm.attachmentBegin(depthFormat, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			rm.attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE);

			rm.subpassBegin();
			rm.subpassColorAttachment(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0);
			rm.subpassResolveAttachment(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);
			rm.subpassDepthStencilAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 2);
			renderPass = rm.create(device);

			createFrameBuffer();
		}

		void createFrameBuffer() {
			const auto extent = swapchain->getExtent();
			if (extent.width == 0 || extent.height == 0) {return;}

			depth = device->createDepthStencilAttachment(extent.width, extent.height);

			multiSample.color = device->createColorAttachment(extent.width, extent.height, sampeCount);
			multiSample.depth = device->createDepthStencilAttachment(extent.width, extent.height, sampeCount);

			frameBuffers.swap(std::vector<vk::FrameBuffer>());
			for (uint32_t i = 0; i < swapchain->getImageCount(); i++)
			{
				std::array<VkImageView, 4> attachments = { multiSample.color->view(), swapchain->getView(i),multiSample.depth->view(), depth->view() };
				frameBuffers.emplace_back(device->createFrameBuffer(renderPass, extent.width, extent.height, attachments));
			}

			if (commandBuffers.size() == 0) {
				for (uint32_t i = 0; i < swapchain->getImageCount(); i++)
				{
					commandBuffers.emplace_back(commandPool->createCommandBuffer());
				}
			}
		}

		bool resize() {
			if (swapchain->reCreate()) {
				createFrameBuffer();
				return true;
			}
			return false;
		}

		vk::Device& getDevice() { return device; }
		vk::Swapchain& getSwapchain() { return swapchain; }
		vk::Queue& getGraphicsQueue() { return graphicsQueue; }
		VkExtent2D getExtent() const { return swapchain->getExtent(); }
		vk::FrameBuffer& getFrameBuffer(uint32_t index) { return frameBuffers.at(index); }
		vk::CommandBuffer& getCommandBuffer(uint32_t index) { return commandBuffers.at(index); }
		vk::RenderPass& getRenderPass() { return renderPass; }
		vk::CommandPool& getCommandPool() { return commandPool; }
		vk::DescriptorPool& getDescriptorPool() { return descriptorPool; }
		vk::CommandBuffer createCommandBuffer() { return commandPool->createCommandBuffer(); }
		VkSampleCountFlagBits getSampleCount() { return sampeCount; }
		
	};

	using Context = std::unique_ptr<Context_T>;

	static Context createContext(const void* windowHandle) {
		return std::make_unique<Context_T>(windowHandle);
	}
}
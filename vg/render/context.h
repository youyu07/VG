#pragma once

//#include <vku.hpp>
#include "vk/vkt.h"

namespace vg
{
	/*
	class Context
	{
		vk::UniqueInstance instance;
		vk::PhysicalDevice physicalDevice;
		vk::UniqueDevice device;
		vk::UniqueDescriptorPool descriptorPool;

		vk::UniqueSurfaceKHR surface;
		vk::UniqueSwapchainKHR swapchain;
		vk::Extent2D extent;

		uint32_t graphicsQueueFamilyIndex = ~0;
		uint32_t computerQueueFamilyIndex = ~0;
		vk::Queue graphicsQueue;
		vk::Queue computerQueue;

		vk::PhysicalDeviceMemoryProperties memoryProperties;

		std::vector<vk::Image> swapchainImages;
		std::vector<vk::UniqueImageView> swapchainImageViews;
		vk::UniqueCommandPool commandPool;

		//global variable
		static vk::SampleCountFlagBits sample;
		static vk::Format swapchainFormat;
		static vk::Format depthFormat;
	public:
		Context() {};

		Context(const void* windowHandle);

		bool createSwapchain();

		vku::ShaderModule compileGLSLToSpv(vk::ShaderStageFlagBits stage,const std::string& src) const;

		inline const vk::Queue& getGraphicsQueue() const { return graphicsQueue; }
		inline const vk::Queue& getComputerQueue() const { return computerQueue; }
		inline const vk::Device& getDevice() const { return device.get(); }
		inline const vk::SwapchainKHR& getSwapchain() const { return swapchain.get(); }
		inline const vk::CommandPool& getCommandPool() const { return commandPool.get(); }
		inline const vk::DescriptorPool& getDescriptorPool() const { return descriptorPool.get(); }
		inline const vk::PhysicalDeviceMemoryProperties& getMemoryProperties() const { return memoryProperties; }
		inline const vk::Extent2D& getExtent() const { return extent; }
		inline uint32_t getSwapchainImageCount() const { return static_cast<uint32_t>(swapchainImages.size()); }
		inline const vk::ImageView& getSwapchainImageView(uint32_t index) const { return swapchainImageViews.at(index).get(); }

		operator vk::Device() const
		{
			return device.get();
		}

	public:
		static vk::SampleCountFlagBits getSample() { return sample; }
		static vk::Format getSwapchainFormat() { return swapchainFormat; }
		static vk::Format getDepthFormat() { return depthFormat; }
	};
	*/

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

			vk::DeviceMaker dm;
			dm.extension(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
			VkPhysicalDeviceFeatures feature;
			feature.wideLines = VK_TRUE;
			dm.features(feature);
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

			resize();

			vk::RenderpassMaker rm;
			rm.attachmentBegin(swapchain->getColorFormat(),VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			rm.attachmentSamples(sampeCount);
			rm.attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
			rm.attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE);

			rm.attachmentBegin(swapchain->getColorFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
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
		}

		bool resize() {
			swapchain = device->createSwapchain(surface);

			const auto extent = swapchain->getExtent();
			if (extent.width == 0 || extent.height == 0) {
				return false;
			}
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

			return true;
		}

		vk::Device_T* getDevice() const { return device.get(); }
		vk::Swapchain_T* getSwapchain() const { return swapchain.get(); }
		vk::Queue_T* getGraphicsQueue() const { return graphicsQueue.get(); }
		VkExtent2D getExtent() const { return swapchain->getExtent(); }
	};

	using Context = std::unique_ptr<Context_T>;

	static Context createContext(const void* windowHandle) {
		return std::make_unique<Context_T>(windowHandle);
	}
}
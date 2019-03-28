#pragma once

#include <vku.hpp>

namespace vg
{

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

}
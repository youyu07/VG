#pragma once
#include "device.h"
#include <algorithm>

namespace vg::vk
{

	class Swapchain : public ISwapchain, public __VkObject<VkSwapchainKHR>
	{
		using ISwapchain::ISwapchain;
	public:
		bool setup(VkSurfaceKHR surface)
		{
			this->surface = surface;

			auto& device = getDevice<Device>();
			const VkPhysicalDevice& physicalDevice = device.getPhysicalDevice();

			uint32_t formatCount = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);

			if (formatCount == 0) {
				log_error("this surface has no support format.");
				return false;
			}

			std::vector<VkSurfaceFormatKHR> formats(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

			surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
			for (const auto& format : formats)
			{
				if (format.format == VK_FORMAT_B8G8R8A8_UNORM || format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_R8G8B8A8_SRGB || format.format == VK_FORMAT_B8G8R8A8_SRGB)
				{
					surfaceFormat = format;
					break;
				}
			}

			VkSurfaceCapabilitiesKHR surfCapabilities = {};
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfCapabilities);
			VkExtent2D swapchainExtent = {};
			swapchainExtent.width = std::max<uint32_t>(surfCapabilities.currentExtent.width, 1u);
			swapchainExtent.height = std::max<uint32_t>(surfCapabilities.currentExtent.height, 1u);

			uint32_t presentModeCount = 0;
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
			std::vector<VkPresentModeKHR> presentModes(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

			VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
			uint32_t desiredNumberOfSwapChainImages = 2;
			if (std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) == presentModes.end())
			{
				swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				desiredNumberOfSwapChainImages = 3;
			}

			auto preTransform = (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : surfCapabilities.currentTransform;

			// Find a supported composite alpha mode - one of these is guaranteed to be set
			VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
				VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
				VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
				VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
				VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
			};
			for (uint32_t i = 0; i < _countof(compositeAlphaFlags); i++) {
				if (surfCapabilities.supportedCompositeAlpha& compositeAlphaFlags[i]) {
					compositeAlpha = compositeAlphaFlags[i];
					break;
				}
			}

			VkSwapchainCreateInfoKHR info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
			info.surface = surface;
			info.minImageCount = desiredNumberOfSwapChainImages;
			info.imageFormat = surfaceFormat.format;
			info.imageExtent.width = swapchainExtent.width;
			info.imageExtent.height = swapchainExtent.height;
			info.preTransform = preTransform;
			info.compositeAlpha = compositeAlpha;
			info.imageArrayLayers = 1;
			info.presentMode = swapchainPresentMode;
			info.oldSwapchain = handle;
			info.clipped = VK_TRUE;
			info.imageColorSpace = surfaceFormat.colorSpace;
			info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			// vkCmdClearColorImage() command requires the image to use VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL layout
			// that requires  VK_IMAGE_USAGE_TRANSFER_DST_BIT to be set
			info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			info.queueFamilyIndexCount = 0;
			info.pQueueFamilyIndices = NULL;

			VkSwapchainKHR obj = VK_NULL_HANDLE;
			if (vkCreateSwapchainKHR(device, &info, NULL, &obj) != VK_SUCCESS) {
				log_error("Failed to create Vulkan swapchain");
				return false;
			}

			if (handle != VK_NULL_HANDLE)
			{
				vkDestroySwapchainKHR(device, handle, NULL);
			}
			handle = obj;

			uint32_t swapchainImageCount = 0;
			vkGetSwapchainImagesKHR(device, handle, &swapchainImageCount, nullptr);
			std::vector<VkImage> deviceImage(swapchainImageCount);
			vkGetSwapchainImagesKHR(device, handle, &swapchainImageCount, deviceImage.data());

			destroyImageViews();
			for (size_t i = 0; i < swapchainImageCount; i++)
			{
				auto img = new Image(m_device);
				images.push_back(img);
				ImageDesc desc = {ImageType::Image_2d,Util::convertFormat(surfaceFormat.format),1,1,swapchainExtent.width,swapchainExtent.height};
				img->setup(desc,deviceImage[i]);

				ImageViewDesc viewDesc = {ImageViewType::View_2d};
				auto view = img->createView(viewDesc);
				views.push_back(static_cast<ImageView*>(view));
			}

			if (!acquireSemaphore) {
				acquireSemaphore = device.createSemaphore();
			}

			return true;
		}

		~Swapchain()
		{
			VkDevice device = getDevice<Device>();
			destroyImageViews();
			delete acquireSemaphore;
			vkDestroySwapchainKHR(device, handle, NULL);
		}

		virtual bool resize() override
		{



			return true;
		}

		virtual bool present(IImage* image) override
		{
			auto& device = getDevice<Device>();

			VkPresentInfoKHR info = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
			info.pImageIndices = &imageIndex;
			info.pSwapchains = &handle;
			info.swapchainCount = 1;
			info.waitSemaphoreCount = 0;
			info.pWaitSemaphores = nullptr;

			auto result = vkQueuePresentKHR(device.getGraphicsQueue(), &info);
			if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				setup(surface);
			}
			else if (result != VK_SUCCESS)
			{
				log_error("Present failed");
				return false;
			}

			result = vkAcquireNextImageKHR(device, handle, UINT64_MAX, *acquireSemaphore, VK_NULL_HANDLE, &imageIndex);
			if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				setup(surface);
			}
			else if (result != VK_SUCCESS) 
			{
				log_error("Acquire Next Image failed");
				return false;
			}
			return true;
		}

	private:
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkSurfaceFormatKHR surfaceFormat;
		std::vector<Image*> images;
		std::vector<ImageView*> views;
		uint32_t imageIndex = 0;
		Semaphore* acquireSemaphore;

		inline void destroyImageViews()
		{
			for(auto& img : images)
			{
				delete img;
			}
			for(auto& view : views)
			{
				delete view;
			}
			images.clear();
			views.clear();
		}
	};

}
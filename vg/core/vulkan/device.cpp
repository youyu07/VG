#include "device.h"
#include "swapchain.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "util.h"
#include "renderContext.h"

namespace vg::vk
{
#pragma region Device
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

	Device::~Device()
	{
		vmaDestroyAllocator(allocator);
		vkDestroyDevice(handle, nullptr);
	}

	bool Device::setup(bool enableValidation) {
		if (!instance.setup(enableValidation, DebugMessengerCallback)) {
			return false;
		}
		if (!physicalDevice.setup(instance)) {
			return false;
		}

		float priorities = 1.0f;
		queueFamilyIndex = physicalDevice.getQueueFamily();
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

		if (vkCreateDevice(physicalDevice, &info, nullptr, &handle) != VK_SUCCESS) {
			log_error("Failed to create device!");
			return false;
		}

		{
			vkGetDeviceQueue(handle, queueFamilyIndex, 0, &graphicsQueue);
			if (graphicsQueue == VK_NULL_HANDLE) {
				log_error("Faild to get graphics queue.");
				return false;
			}

			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.physicalDevice = physicalDevice;
			allocatorInfo.device = handle;

			vmaCreateAllocator(&allocatorInfo, &allocator);

			commandPool = new CommandPool(handle,graphicsQueue,queueFamilyIndex);
			if(!commandPool->isValid()){
				return false;
			}

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
		
		return true;
	}


	ISwapchain* Device::createSwapchain(void* nativeHandle)
	{
		auto swapchain = new Swapchain(this);
		VkSurfaceKHR surface = VK_NULL_HANDLE;
#if defined(WIN32)
		VkWin32SurfaceCreateInfoKHR info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
		info.hinstance = GetModuleHandle(NULL);
		info.hwnd = reinterpret_cast<HWND>(nativeHandle);
		if (vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface) != VK_SUCCESS) {
			log_error("Faild to create win32 surface!");
			return nullptr;
		}
#endif // defined(WIN32)

		swapchain->setup(surface);
		return swapchain;
	}

	IImage* Device::createImage(const ImageDesc& desc, const void* data)
	{
		auto image = new Image(this);

		if (!image->setup(desc, allocator, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
			return nullptr;
		}

		if (data != nullptr) {
			image->update(0, desc.width * desc.height * Util::getFormatSize(desc.format), data);
		}

		return image;
	}

	void Device::transitionImageLayout(Image& img, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		auto cmd = createCommandBuffer();

		VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = img;

		barrier.subresourceRange.aspectMask = Util::getImageAspectFlags(img.getFormat());
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = img.getMipLevel();
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = img.getLayer();

		cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		vkCmdPipelineBarrier(*cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		cmd->end();

		auto queue = cmd->getPool()->getQueue();

		VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = cmd->GetHandlePtr();

		vkQueueSubmit(queue,1,&submitInfo,VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);
	}
	

	IRenderContext* Device::createRenderContext()
	{
		return new RenderContext(this);
	}


	IImageView* Device::createAttachment(const AttachmentDesc& desc)
	{
		auto image = new Image(this);

		ImageDesc imgDesc = {ImageType::Image_2d,desc.format,1,1,desc.width,desc.height};

		VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (desc.type == AttachmentType::Depth) {
			usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}

		if (!image->setup(imgDesc, allocator, usage)) {
			return nullptr;
		}
		
		ImageViewDesc viewDesc = { ImageViewType::View_2d };
		return image->createView(viewDesc);
	}

	Semaphore* Device::createSemaphore()
	{
		return new Semaphore(handle);
	}

	CommandBuffer* Device::createCommandBuffer()
	{
		return new CommandBuffer(handle,commandPool,VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	}
#pragma endregion

#pragma region Image
	bool Image::setup(const ImageDesc& desc,VmaAllocator allocator, VkImageUsageFlags usage)
	{
		setInfo(desc);

		VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.extent = { width,height,1 };
		imageInfo.format = Util::convertFormat(format);
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.arrayLayers = layer;
		imageInfo.mipLevels = mipLevel;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.imageType = Util::convertImageType(desc.type);
		imageInfo.usage = usage;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(allocator, &imageInfo, &allocCreateInfo, &handle, &allocation, nullptr);

		return true;
	}

	Image::~Image()
	{
		if (handle != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE) {
			vmaDestroyImage(allocator, handle, allocation);
		}
	}


	IImageView* Image::createView(const ImageViewDesc& desc)
	{
		auto view = new ImageView(m_device);
		if (view->setup(this,desc)) {
			return view;
		}
		delete view;
		return nullptr;
	}

	void Image::update(uint32_t offset, uint32_t size, const void* data)
	{

	}
#pragma endregion

#pragma region Image View
	bool ImageView::setup(Image* img, const ImageViewDesc& desc)
	{
		image = img;

		VkDevice device = getDevice<Device>();

		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = *img;
		viewInfo.viewType = Util::convertViewType(desc.type);
		viewInfo.format = Util::convertFormat(img->getFormat());
		viewInfo.components = VkComponentMapping();
		viewInfo.subresourceRange = { Util::getImageAspectFlags(image->getFormat()), 0, 1, 0, 1 };

		vkCreateImageView(device, &viewInfo, nullptr, &handle);

		return true;
	}
#pragma endregion
}


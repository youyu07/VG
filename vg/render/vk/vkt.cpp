#include "vkt.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <shaderc/shaderc.h>

namespace vg::vk
{

	Surface Instance_T::createSurface(const void* windowHandle) const
	{
		VkSurfaceKHR surface_;

#ifdef WIN32
		VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
		surfaceInfo.hinstance = ::GetModuleHandle(NULL);
		surfaceInfo.hwnd = HWND(windowHandle);
		VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(handle_, &surfaceInfo, nullptr, &surface_));
#endif // WIN32

		return std::make_unique<Surface_T>(this, surface_);
	}

	Device_T::Device_T(VkPhysicalDevice physicalDevice, VkDevice device) : physicalDevice_(physicalDevice), Handle_T(device)
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = physicalDevice;
		allocatorInfo.device = device;

		VK_CHECK_RESULT(vmaCreateAllocator(&allocatorInfo, &allocator_));
	}

	Buffer Device_T::createUniformBuffer(VkDeviceSize size, VkBool32 dynamic)
	{
		return std::make_unique<Buffer_T>(this, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, dynamic ? MemoryUsage::CPU_TO_GPU : MemoryUsage::GPU_ONLY);
	}

	Buffer Device_T::createVertexBuffer(VkDeviceSize size, VkBool32 dynamic)
	{
		return std::make_unique<Buffer_T>(this, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, dynamic ? MemoryUsage::CPU_TO_GPU : MemoryUsage::GPU_ONLY);
	}
	Buffer Device_T::createIndexBuffer(VkDeviceSize size, VkBool32 dynamic)
	{
		return std::make_unique<Buffer_T>(this, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, dynamic ? MemoryUsage::CPU_TO_GPU : MemoryUsage::GPU_ONLY);
	}

	void Queue_T::submit(ArrayProxy<const VkCommandBuffer> cmds, ArrayProxy<const VkSemaphore> wait, ArrayProxy<const VkSemaphore> signal, VkFence fence, VkPipelineStageFlags waitStage) 
	{
		VkSubmitInfo info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		info.commandBufferCount = cmds.size();
		info.pCommandBuffers = cmds.data();
		info.waitSemaphoreCount = wait.size();
		info.pWaitSemaphores = wait.data();
		info.signalSemaphoreCount = signal.size();
		info.pSignalSemaphores = signal.data();
		info.pWaitDstStageMask = &waitStage;
		VK_CHECK_RESULT(vkQueueSubmit(handle_, 1, &info, fence));
	}

	void Queue_T::submit(CommandBuffer& cmd) 
	{
		submit(cmd->get());
	}

	VkResult Queue_T::present(ArrayProxy<const VkSwapchainKHR> swapchain, uint32_t* imageIndex, ArrayProxy<const VkSemaphore> wait) 
	{
		VkPresentInfoKHR info = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		info.swapchainCount = swapchain.size();
		info.pSwapchains = swapchain.data();
		info.waitSemaphoreCount = wait.size();
		info.pWaitSemaphores = wait.data();
		info.pImageIndices = imageIndex;
		return vkQueuePresentKHR(handle_, &info);
	}



	// image functions
	static VkImageAspectFlags getAspect(VkFormat format) {
		switch (format)
		{
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_D32_SFLOAT:
			return VK_IMAGE_ASPECT_DEPTH_BIT;
			break;
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			break;
		}
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}

	struct BlockParams {
		uint8_t blockWidth;
		uint8_t blockHeight;
		uint8_t bytesPerBlock;
	};

	static BlockParams getBlockParams(VkFormat format) {
		switch (format) {
		case VK_FORMAT_R4G4_UNORM_PACK8:return BlockParams{ 1, 1, 1 };
		case VK_FORMAT_R4G4B4A4_UNORM_PACK16:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_B4G4R4A4_UNORM_PACK16:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R5G6B5_UNORM_PACK16:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_B5G6R5_UNORM_PACK16:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R5G5B5A1_UNORM_PACK16:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_B5G5R5A1_UNORM_PACK16:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_A1R5G5B5_UNORM_PACK16:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R8_UNORM:return BlockParams{ 1, 1, 1 };
		case VK_FORMAT_R8_SNORM:return BlockParams{ 1, 1, 1 };
		case VK_FORMAT_R8_USCALED:return BlockParams{ 1, 1, 1 };
		case VK_FORMAT_R8_SSCALED:return BlockParams{ 1, 1, 1 };
		case VK_FORMAT_R8_UINT:return BlockParams{ 1, 1, 1 };
		case VK_FORMAT_R8_SINT:return BlockParams{ 1, 1, 1 };
		case VK_FORMAT_R8_SRGB:return BlockParams{ 1, 1, 1 };
		case VK_FORMAT_R8G8_UNORM:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R8G8_SNORM:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R8G8_USCALED:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R8G8_SSCALED:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R8G8_UINT:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R8G8_SINT:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R8G8_SRGB:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R8G8B8_UNORM:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_R8G8B8_SNORM:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_R8G8B8_USCALED:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_R8G8B8_SSCALED:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_R8G8B8_UINT:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_R8G8B8_SINT:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_R8G8B8_SRGB:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_B8G8R8_UNORM:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_B8G8R8_SNORM:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_B8G8R8_USCALED:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_B8G8R8_SSCALED:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_B8G8R8_UINT:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_B8G8R8_SINT:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_B8G8R8_SRGB:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_R8G8B8A8_UNORM:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R8G8B8A8_SNORM:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R8G8B8A8_USCALED:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R8G8B8A8_SSCALED:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R8G8B8A8_UINT:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R8G8B8A8_SINT:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R8G8B8A8_SRGB:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_B8G8R8A8_UNORM:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_B8G8R8A8_SNORM:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_B8G8R8A8_USCALED:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_B8G8R8A8_SSCALED:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_B8G8R8A8_UINT:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_B8G8R8A8_SINT:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_B8G8R8A8_SRGB:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A8B8G8R8_SNORM_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A8B8G8R8_USCALED_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A8B8G8R8_UINT_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A8B8G8R8_SINT_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A8B8G8R8_SRGB_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A2R10G10B10_UNORM_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A2R10G10B10_SNORM_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A2R10G10B10_USCALED_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A2R10G10B10_UINT_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A2R10G10B10_SINT_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A2B10G10R10_SNORM_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A2B10G10R10_USCALED_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A2B10G10R10_UINT_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_A2B10G10R10_SINT_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R16_UNORM:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R16_SNORM:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R16_USCALED:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R16_SSCALED:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R16_UINT:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R16_SINT:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R16_SFLOAT:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_R16G16_UNORM:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R16G16_SNORM:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R16G16_USCALED:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R16G16_SSCALED:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R16G16_UINT:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R16G16_SINT:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R16G16_SFLOAT:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R16G16B16_UNORM:return BlockParams{ 1, 1, 6 };
		case VK_FORMAT_R16G16B16_SNORM:return BlockParams{ 1, 1, 6 };
		case VK_FORMAT_R16G16B16_USCALED:return BlockParams{ 1, 1, 6 };
		case VK_FORMAT_R16G16B16_SSCALED:return BlockParams{ 1, 1, 6 };
		case VK_FORMAT_R16G16B16_UINT:return BlockParams{ 1, 1, 6 };
		case VK_FORMAT_R16G16B16_SINT:return BlockParams{ 1, 1, 6 };
		case VK_FORMAT_R16G16B16_SFLOAT:return BlockParams{ 1, 1, 6 };
		case VK_FORMAT_R16G16B16A16_UNORM:return BlockParams{ 1, 1, 8 };
		case VK_FORMAT_R16G16B16A16_SNORM:return BlockParams{ 1, 1, 8 };
		case VK_FORMAT_R16G16B16A16_USCALED:return BlockParams{ 1, 1, 8 };
		case VK_FORMAT_R16G16B16A16_SSCALED:return BlockParams{ 1, 1, 8 };
		case VK_FORMAT_R16G16B16A16_UINT:return BlockParams{ 1, 1, 8 };
		case VK_FORMAT_R16G16B16A16_SINT:return BlockParams{ 1, 1, 8 };
		case VK_FORMAT_R16G16B16A16_SFLOAT:return BlockParams{ 1, 1, 8 };
		case VK_FORMAT_R32_UINT:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R32_SINT:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R32_SFLOAT:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_R32G32_UINT:return BlockParams{ 1, 1, 8 };
		case VK_FORMAT_R32G32_SINT:return BlockParams{ 1, 1, 8 };
		case VK_FORMAT_R32G32_SFLOAT:return BlockParams{ 1, 1, 8 };
		case VK_FORMAT_R32G32B32_UINT:return BlockParams{ 1, 1, 12 };
		case VK_FORMAT_R32G32B32_SINT:return BlockParams{ 1, 1, 12 };
		case VK_FORMAT_R32G32B32_SFLOAT:return BlockParams{ 1, 1, 12 };
		case VK_FORMAT_R32G32B32A32_UINT:return BlockParams{ 1, 1, 16 };
		case VK_FORMAT_R32G32B32A32_SINT:return BlockParams{ 1, 1, 16 };
		case VK_FORMAT_R32G32B32A32_SFLOAT:return BlockParams{ 1, 1, 16 };
		case VK_FORMAT_R64_UINT:return BlockParams{ 1, 1, 8 };
		case VK_FORMAT_R64_SINT:return BlockParams{ 1, 1, 8 };
		case VK_FORMAT_R64_SFLOAT:return BlockParams{ 1, 1, 8 };
		case VK_FORMAT_R64G64_UINT:return BlockParams{ 1, 1, 16 };
		case VK_FORMAT_R64G64_SINT:return BlockParams{ 1, 1, 16 };
		case VK_FORMAT_R64G64_SFLOAT:return BlockParams{ 1, 1, 16 };
		case VK_FORMAT_R64G64B64_UINT:return BlockParams{ 1, 1, 24 };
		case VK_FORMAT_R64G64B64_SINT:return BlockParams{ 1, 1, 24 };
		case VK_FORMAT_R64G64B64_SFLOAT:return BlockParams{ 1, 1, 24 };
		case VK_FORMAT_R64G64B64A64_UINT:return BlockParams{ 1, 1, 32 };
		case VK_FORMAT_R64G64B64A64_SINT:return BlockParams{ 1, 1, 32 };
		case VK_FORMAT_R64G64B64A64_SFLOAT:return BlockParams{ 1, 1, 32 };
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_D16_UNORM:return BlockParams{ 1, 1, 2 };
		case VK_FORMAT_X8_D24_UNORM_PACK32:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_D32_SFLOAT:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_S8_UINT:return BlockParams{ 1, 1, 1 };
		case VK_FORMAT_D16_UNORM_S8_UINT:return BlockParams{ 1, 1, 3 };
		case VK_FORMAT_D24_UNORM_S8_UINT:return BlockParams{ 1, 1, 4 };
		case VK_FORMAT_D32_SFLOAT_S8_UINT:return BlockParams{ 1, 1, 5 };
		case VK_FORMAT_BC1_RGB_UNORM_BLOCK:return BlockParams{ 4, 4, 8 };
		case VK_FORMAT_BC1_RGB_SRGB_BLOCK:return BlockParams{ 4, 4, 8 };
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:return BlockParams{ 4, 4, 8 };
		case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:return BlockParams{ 4, 4, 8 };
		case VK_FORMAT_BC2_UNORM_BLOCK:return BlockParams{ 4, 4, 16 };
		case VK_FORMAT_BC2_SRGB_BLOCK:return BlockParams{ 4, 4, 16 };
		case VK_FORMAT_BC3_UNORM_BLOCK:return BlockParams{ 4, 4, 16 };
		case VK_FORMAT_BC3_SRGB_BLOCK:return BlockParams{ 4, 4, 16 };
		case VK_FORMAT_BC4_UNORM_BLOCK:return BlockParams{ 4, 4, 16 };
		case VK_FORMAT_BC4_SNORM_BLOCK:return BlockParams{ 4, 4, 16 };
		case VK_FORMAT_BC5_UNORM_BLOCK:return BlockParams{ 4, 4, 16 };
		case VK_FORMAT_BC5_SNORM_BLOCK:return BlockParams{ 4, 4, 16 };
		}
		return BlockParams{ 0, 0, 0 };
	}

	Image_T::Image_T(const Device_T* device, VkImageCreateInfo& info, MemoryUsage memoryUsage, ViewType viewType) : device_(device), info_(info) {

		VmaAllocationCreateInfo createInfo = {};
		createInfo.usage = static_cast<VmaMemoryUsage>(memoryUsage);
		VmaAllocationInfo allocationInfo = {};
		VK_CHECK_RESULT(vmaCreateImage(device_->allocator(), &info_, &createInfo, &handle_, &allocation_, &allocationInfo));

		if(viewType != ViewType::NONE)setupView(static_cast<VkImageViewType>(viewType));
	}

	Image_T::Image_T(const Device_T* device, VkImageCreateInfo& info, VkImage image, ViewType viewType) : device_(device), info_(info), Handle_T(image)
	{
		if (viewType != ViewType::NONE)setupView(static_cast<VkImageViewType>(viewType));
	}

	Image_T::~Image_T()
	{
		if (view_) {
			vkDestroyImageView(*device_, view_, nullptr);
		}
		if (handle_ && allocation_) {
			vmaDestroyImage(device_->allocator(), handle_, allocation_);
		}
	}

	void Image_T::upload(CommandBuffer& cmd, const Buffer& staging)
	{
		VkBufferImageCopy region = {};
		region.imageExtent = info_.extent;
		region.imageSubresource = { getAspect(info_.format),0,0,1 };

		setLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		cmd->copyBufferToImage(*staging, handle_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, region);
		setLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void Image_T::upload(CommandPool& pool, Queue& queue, const void* value) {
		auto size = getBlockParams(info_.format).bytesPerBlock * info_.extent.width * info_.extent.height;
		auto staging = std::make_unique<Buffer_T>(device_, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryUsage::CPU_ONLY);
		staging->uploadLocal(value);

		auto cmd = pool->createCommandBuffer();
		cmd->begin();
		upload(cmd, staging);
		cmd->end();
		queue->submit(cmd);
		queue->waitIdle();
	}

	Image Device_T::createTexture2D(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format)
	{
		VkImageCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		info.format = format;
		info.extent = { width,height,1 };
		info.arrayLayers = 1;
		info.mipLevels = mipLevels;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		return std::make_unique<Image_T>(this, info, MemoryUsage::GPU_ONLY);
	}

	Image Device_T::createDepthStencilAttachment(uint32_t width, uint32_t height, VkSampleCountFlagBits sample, VkFormat format)
	{
		VkImageCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		info.format = format;
		info.extent = { width,height,1 };
		info.arrayLayers = 1;
		info.mipLevels = 1;
		info.samples = sample;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		return std::make_unique<Image_T>(this, info, MemoryUsage::GPU_ONLY);
	}

	Image Device_T::createColorAttachment(uint32_t width, uint32_t height, VkSampleCountFlagBits sample, VkFormat format)
	{
		VkImageCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		info.format = format;
		info.extent = { width,height,1 };
		info.arrayLayers = 1;
		info.mipLevels = 1;
		info.samples = sample;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		return std::make_unique<Image_T>(this, info, MemoryUsage::GPU_ONLY);
	}

	Image Device_T::createTransferImage(uint32_t width, uint32_t height, VkFormat format)
	{
		VkImageCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		info.format = format;
		info.extent = { width,height,1 };
		info.arrayLayers = 1;
		info.mipLevels = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.tiling = VK_IMAGE_TILING_LINEAR;
		info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		return std::make_unique<Image_T>(this, info, MemoryUsage::CPU_ONLY, ViewType::NONE);
	}

	void Image_T::setupView(VkImageViewType viewType)
	{
		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = handle_;
		viewInfo.viewType = viewType;
		viewInfo.format = info_.format;
		viewInfo.subresourceRange = { getAspect(info_.format), 0, info_.mipLevels, 0, info_.arrayLayers };
		VK_CHECK_RESULT(vkCreateImageView(*device_, &viewInfo, nullptr, &view_));
	}

	void Image_T::setLayout(CommandBuffer& cmd, VkImageLayout newLayout)
	{
		if (newLayout == layout_) return;
		auto oldLayout = layout_;
		layout_ = newLayout;

		VkImageMemoryBarrier imageMemoryBarriers = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		imageMemoryBarriers.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarriers.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarriers.oldLayout = oldLayout;
		imageMemoryBarriers.newLayout = newLayout;
		imageMemoryBarriers.image = handle_;
		imageMemoryBarriers.subresourceRange = { getAspect(info_.format), 0, info_.mipLevels, 0, info_.arrayLayers };

		auto srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		auto dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

		VkAccessFlags srcMask = 0;
		VkAccessFlags dstMask = 0;

		switch (oldLayout) {
		case VK_IMAGE_LAYOUT_GENERAL:								srcMask = VK_ACCESS_TRANSFER_WRITE_BIT; break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:				srcMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:		srcMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:		srcMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT; break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:				srcMask = VK_ACCESS_SHADER_READ_BIT; break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:					srcMask = VK_ACCESS_TRANSFER_READ_BIT; break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:					srcMask = VK_ACCESS_TRANSFER_WRITE_BIT; break;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:						srcMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT; break;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:						srcMask = VK_ACCESS_MEMORY_READ_BIT; break;
		}

		switch (newLayout) {
		case VK_IMAGE_LAYOUT_GENERAL:								dstMask = VK_ACCESS_TRANSFER_WRITE_BIT; break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:				dstMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:		dstMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:		dstMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT; break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:				dstMask = VK_ACCESS_SHADER_READ_BIT; break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:					dstMask = VK_ACCESS_TRANSFER_READ_BIT; break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:					dstMask = VK_ACCESS_TRANSFER_WRITE_BIT; break;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:						dstMask = VK_ACCESS_TRANSFER_WRITE_BIT; break;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:						dstMask = VK_ACCESS_MEMORY_READ_BIT; break;
		}

		imageMemoryBarriers.srcAccessMask = srcMask;
		imageMemoryBarriers.dstAccessMask = dstMask;
		cmd->pipelineBarrier(srcStageMask, dstStageMask, VkDependencyFlags(), {}, {}, imageMemoryBarriers);
	}

	void* Image_T::map()
	{
		void* ptr = nullptr;
		vmaMapMemory(device_->allocator(), allocation_, &ptr);
		return ptr;
	}
	void Image_T::unmap()
	{
		vmaUnmapMemory(device_->allocator(), allocation_);
	}

	VkDeviceSize Image_T::size() const
	{
		return info_.extent.width * info_.extent.height * getBlockParams(info_.format).bytesPerBlock;
	}

	VkImageSubresourceLayers Image_T::getSubresourceLayers(uint32_t mipLevel, uint32_t baseArrayLayer, uint32_t layerCount) const
	{
		return { getAspect(info_.format),mipLevel,baseArrayLayer,layerCount };
	}

	VkImageSubresourceRange Image_T::getSubresourceRange() const
	{
		return { getAspect(info_.format), 0, info_.mipLevels, 0, info_.arrayLayers };
	}

	VkImageSubresourceRange Image_T::getSubresourceRange(uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) const
	{
		return { getAspect(info_.format), baseMipLevel, levelCount, baseArrayLayer, layerCount };
	}



	//swapchain functions
	Swapchain_T::Swapchain_T(const Device_T* device, const Surface_T* surface) : device_(device),surface_(surface)
	{
		reCreate();
	}

	bool Swapchain_T::reCreate()
	{
		auto formats = device_->getSurfaceFormat(surface_->get());
		VkSurfaceFormatKHR surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM,VK_COLORSPACE_SRGB_NONLINEAR_KHR };
		colorFormat = surfaceFormat.format;

		auto capabilities = device_->getSurfaceCapabilities(surface_->get());

		extent = capabilities.currentExtent;
		if (extent.width == 0 || extent.height == 0) {
			return false;
		}

		VkSwapchainCreateInfoKHR info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		info.imageExtent = capabilities.currentExtent;
		info.preTransform = capabilities.currentTransform;
		info.surface = *surface_;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		info.minImageCount = 3;
		info.imageFormat = surfaceFormat.format;
		info.imageColorSpace = surfaceFormat.colorSpace;
		info.imageArrayLayers = 1;
		info.clipped = VK_TRUE;
		info.oldSwapchain = handle_;
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		VK_CHECK_RESULT(vkCreateSwapchainKHR(*device_, &info, nullptr, &swapchain));

		if (handle_ != VK_NULL_HANDLE) {
			destroy();
		}
		handle_ = swapchain;

		uint32_t count = 0;
		VK_CHECK_RESULT(vkGetSwapchainImagesKHR(*device_, handle_, &count, nullptr));
		std::vector<VkImage> images(count);
		VK_CHECK_RESULT(vkGetSwapchainImagesKHR(*device_, handle_, &count, images.data()));
		VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.format = info.imageFormat;
		imageInfo.extent = { extent.width,extent.height,1 };
		imageInfo.arrayLayers = 1;
		imageInfo.mipLevels = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		for (auto& image : images)
		{
			images_.emplace_back(std::make_unique<Image_T>(device_, imageInfo, image));
		}
		return true;
	}


	//buffer functions
	Buffer_T::Buffer_T(const Device_T* device,VkDeviceSize size,VkBufferUsageFlags usage, MemoryUsage memoryUsage) : device_(device),size_(size)
	{
		VkBufferCreateInfo info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		info.size = size;
		info.usage = usage;

		VmaAllocationCreateInfo createInfo = {};
		createInfo.usage = static_cast<VmaMemoryUsage>(memoryUsage);

		VK_CHECK_RESULT(vmaCreateBuffer(device_->allocator(), &info, &createInfo, &handle_, &allocation_, nullptr));
	}

	Buffer_T::~Buffer_T()
	{
		vmaDestroyBuffer(device_->allocator(), handle_, allocation_);
	}

	void Buffer_T::uploadLocal(const void* value)
	{
		void* ptr = nullptr;
		VK_CHECK_RESULT(vmaMapMemory(device_->allocator(), allocation_, &ptr));
		memcpy(ptr, value, size_);
		vmaUnmapMemory(device_->allocator(), allocation_);
		vmaFlushAllocation(device_->allocator(), allocation_, 0, size_);
	}

	void Buffer_T::upload(vk::CommandBuffer& cmd,const Buffer& staging)
	{
		VkBufferCopy copy = {0,0,size_};
		cmd->copyBuffer(staging->get(), handle_, copy);
	}

	void Buffer_T::upload(CommandPool& pool, Queue& queue, const void* value)
	{
		auto staging = std::make_unique<Buffer_T>(device_, size_, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryUsage::CPU_ONLY);
		staging->uploadLocal(value);

		auto cmd = pool->createCommandBuffer();
		cmd->begin();
		upload(cmd, staging);
		cmd->end();
		queue->submit(cmd);
		queue->waitIdle();
	}

	void* Buffer_T::map()
	{
		void* ptr = nullptr;
		VK_CHECK_RESULT(vmaMapMemory(device_->allocator(), allocation_, &ptr));
		return ptr;
	}
	void Buffer_T::unmap()
	{
		vmaUnmapMemory(device_->allocator(), allocation_);
	}
	void Buffer_T::flush()
	{
		vmaFlushAllocation(device_->allocator(), allocation_, 0, VK_WHOLE_SIZE);
	}

	CommandBuffer_T::CommandBuffer_T(const Device_T* device, const CommandPool_T* pool) : device_(device), pool_(pool) 
	{
		VkCommandBufferAllocateInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		info.commandBufferCount = 1;
		info.commandPool = pool_->get();
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(*device_, &info, &handle_));
	}
	CommandBuffer_T::~CommandBuffer_T() 
	{ 
		vkFreeCommandBuffers(*device_, pool_->get(), 1, &handle_); 
	}

	static shaderc_shader_kind getShadercKind(VkShaderStageFlagBits stage)
	{
		switch (stage)
		{
		case VK_SHADER_STAGE_VERTEX_BIT:					return shaderc_vertex_shader;
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:		return shaderc_tess_control_shader;
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:	return shaderc_tess_evaluation_shader;
		case VK_SHADER_STAGE_GEOMETRY_BIT:					return shaderc_geometry_shader;
		case VK_SHADER_STAGE_FRAGMENT_BIT:					return shaderc_fragment_shader;
		case VK_SHADER_STAGE_COMPUTE_BIT:					return shaderc_compute_shader;
		}
		return shaderc_shader_kind();
	}

	PipelineMaker& PipelineMaker::shaderGLSL(VkShaderStageFlagBits stage, const std::string& src) {
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		auto result = shaderc_compile_into_spv(
			compiler, src.c_str(), src.size(),
			getShadercKind(stage), "", "main", nullptr);

		auto status = shaderc_result_get_compilation_status(result);
		if (status != shaderc_compilation_status_success) {
			log_error("SHADER : ", shaderc_result_get_error_message(result));
		}

		auto length = shaderc_result_get_length(result);
		auto bytes = (uint32_t*)shaderc_result_get_bytes(result);

		shader(stage, length, bytes);

		shaderc_result_release(result);
		shaderc_compiler_release(compiler);

		return *this;
	}

} // namespace vg::vk

#include "vkt.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <shaderc/shaderc.h>

namespace vg::vk
{

	Surface Instance_T::createSurface(const void* windowHandle)
	{
		VkSurfaceKHR surface_;

#ifdef WIN32
		VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
		surfaceInfo.hinstance = ::GetModuleHandle(NULL);
		surfaceInfo.hwnd = HWND(windowHandle);
		VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(instance_, &surfaceInfo, nullptr, &surface_));
#endif // WIN32

		return std::make_unique<Surface_T>(this, surface_);
	}

	Device_T::Device_T(VkPhysicalDevice physicalDevice, VkDevice device) : physicalDevice_(physicalDevice), device_(device)
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = physicalDevice;
		allocatorInfo.device = device;

		VK_CHECK_RESULT(vmaCreateAllocator(&allocatorInfo, &allocator_));
	}

	Buffer Device_T::createUniformBuffer(VkDeviceSize size, VkBool32 dynamic)
	{
		return std::make_unique<Buffer_T>(this, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, dynamic ? Buffer_T::Type::CPU_TO_GPU : Buffer_T::Type::GPU_ONLY);
	}

	Buffer Device_T::createVertexBuffer(VkDeviceSize size, VkBool32 dynamic = false)
	{
		return std::make_unique<Buffer_T>(this, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, dynamic ? Buffer_T::Type::CPU_TO_GPU : Buffer_T::Type::GPU_ONLY);
	}
	Buffer Device_T::createIndexBuffer(VkDeviceSize size, VkBool32 dynamic = false)
	{
		return std::make_unique<Buffer_T>(this, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, dynamic ? Buffer_T::Type::CPU_TO_GPU : Buffer_T::Type::GPU_ONLY);
	}

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

	/// Get the details of vulkan texture formats.
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

	Image_T::Image_T(const Device_T* device, VkImageCreateInfo& info, VkImageViewType viewType) : device_(device), info_(info) {

		VmaAllocationCreateInfo createInfo = {};
		createInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		VmaAllocationInfo allocationInfo = {};

		VK_CHECK_RESULT(vmaCreateImage(device_->allocator(), &info_, &createInfo, &image_, &allocation_, &allocationInfo));

		setupView(viewType);
	}

	Image_T::Image_T(const Device_T* device, VkImageCreateInfo& info, VkImage image, VkImageViewType viewType) : device_(device), info_(info), image_(image)
	{
		setupView(viewType);
	}

	Image_T::~Image_T()
	{
		if (view_) {
			vkDestroyImageView(*device_, view_, nullptr);
		}
		if (image_ && allocation_) {
			vmaDestroyImage(device_->allocator(), image_, allocation_);
		}
	}

	void Image_T::upload(CommandBuffer& cmd, const void* value)
	{
		auto size = getBlockParams(info_.format).bytesPerBlock * info_.extent.width * info_.extent.height;
		auto staging = std::make_unique<Buffer_T>(device_, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Buffer_T::Type::CPU_ONLY);
		staging->uploadLocal(value, size);

		VkBufferImageCopy region = {};
		region.imageExtent = info_.extent;
		region.imageSubresource = { getAspect(info_.format),0,0,1 };
		cmd->copyBufferToImage(*staging, image_, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, region);
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
		return std::make_unique<Image_T>(this, info, VK_IMAGE_VIEW_TYPE_2D);
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
		return std::make_unique<Image_T>(this, info, VK_IMAGE_VIEW_TYPE_2D);
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
		info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		return std::make_unique<Image_T>(this, info, VK_IMAGE_VIEW_TYPE_2D);
	}

	void Image_T::setupView(VkImageViewType viewType)
	{
		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = image_;
		viewInfo.viewType = viewType;
		viewInfo.format = info_.format;
		viewInfo.subresourceRange = { getAspect(info_.format), 0, info_.mipLevels, 0, info_.arrayLayers };
		VK_CHECK_RESULT(vkCreateImageView(*device_, &viewInfo, nullptr, &view_));
	}

	void Image_T::setLayout(CommandBuffer_T* cmd, VkImageLayout newLayout)
	{
		if (newLayout == layout_) return;
		auto oldLayout = layout_;
		layout_ = newLayout;

		VkImageMemoryBarrier imageMemoryBarriers = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		imageMemoryBarriers.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarriers.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarriers.oldLayout = oldLayout;
		imageMemoryBarriers.newLayout = newLayout;
		imageMemoryBarriers.image = image_;
		imageMemoryBarriers.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, info_.mipLevels, 0, info_.arrayLayers };

		auto srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		auto dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

		VkAccessFlags srcMask;
		VkAccessFlags dstMask;

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


	Swapchain_T::Swapchain_T(const Device_T* device, const Surface_T* surface) : device_(device),surface_(surface)
	{
		auto formats = device_->getSurfaceFormat(*surface);
		VkSurfaceFormatKHR surfaceFormat = {VK_FORMAT_B8G8R8A8_UNORM,VK_COLORSPACE_SRGB_NONLINEAR_KHR};
		colorFormat = surfaceFormat.format;

		auto capabilities = device_->getSurfaceCapabilities(*surface);

		extent = capabilities.currentExtent;
		if (extent.width == 0 || extent.height == 0) {
			return;
		}

		VkSwapchainCreateInfoKHR info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
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
		info.oldSwapchain = swapchain_;
		VK_CHECK_RESULT(vkCreateSwapchainKHR(*device_, &info, nullptr, &swapchain_));

		uint32_t count = 0;
		VK_CHECK_RESULT(vkGetSwapchainImagesKHR(*device_, swapchain_, &count, nullptr));
		std::vector<VkImage> images(count);
		VK_CHECK_RESULT(vkGetSwapchainImagesKHR(*device_, swapchain_, &count, images.data()));
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
			images_.emplace_back(std::make_unique<Image_T>(device, imageInfo,image));
		}
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

	void PipelineMaker::shaderGLSL(VkShaderStageFlagBits stage, const std::string& src) {
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
	}

	Buffer_T::Buffer_T(const Device_T* device,VkDeviceSize size,VkBufferUsageFlags usage, Type type) : device_(device),size_(size)
	{
		VkBufferCreateInfo info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		info.size = size;
		info.usage = usage;

		VmaAllocationCreateInfo createInfo = {};
		createInfo.usage = static_cast<VmaMemoryUsage>(type);

		VK_CHECK_RESULT(vmaCreateBuffer(device_->allocator(), &info, &createInfo, &buffer_, &allocation_, nullptr));
	}

	Buffer_T::~Buffer_T()
	{
		vmaDestroyBuffer(device_->allocator(), buffer_, allocation_);
	}

	void Buffer_T::uploadLocal(const void* value, VkDeviceSize size)
	{
		void* ptr = nullptr;
		vmaMapMemory(device_->allocator(), allocation_, &ptr);
		memcpy(ptr, value, size);
		vmaFlushAllocation(device_->allocator(), allocation_, 0, VK_WHOLE_SIZE);
		vmaUnmapMemory(device_->allocator(), allocation_);
	}

	void Buffer_T::upload(vk::CommandBuffer& cmd, const void* value, VkDeviceSize size)
	{
		auto staging = std::make_unique<Buffer_T>(device_,size,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,Type::CPU_ONLY);
		staging->uploadLocal(value, size);

		VkBufferCopy copy = {0,0,size};
		cmd->copyBuffer(*staging, buffer_, copy);
	}

	void* Buffer_T::map()
	{
		void* ptr = nullptr;
		vmaMapMemory(device_->allocator(), allocation_, &ptr);
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

} // namespace vg::vk

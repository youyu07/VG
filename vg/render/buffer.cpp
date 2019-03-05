#include "buffer.h"
#include "device.h"
#include "commandBuffer.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace vg::vk
{
	static const VkImageAspectFlags getImageAspectFlags(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_D32_SFLOAT:
			return VK_IMAGE_ASPECT_DEPTH_BIT;
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		default:
			return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}

	Image::Image(Device* device, const ImageInfo& info) : device(device),info(info)
	{
		VkImageCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		createInfo.extent = { info.width,info.height,1 };
		createInfo.format = info.format;
		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		createInfo.arrayLayers = info.layers;
		createInfo.mipLevels = info.mipLevels;
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		createInfo.imageType = info.type;
		createInfo.usage = info.usage;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(device->getAllocator(), &createInfo, &allocCreateInfo, &handle, &allocation, nullptr);
	}


	void Image::transitionImageLayout(VkImage img, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = img;

		barrier.subresourceRange.aspectMask = getImageAspectFlags(info.format);
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = info.mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = info.layers;

		// TODO
		CommandBuffer* cmd;
		cmd->begin();
		vkCmdPipelineBarrier(*cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		cmd->end();
		cmd->submit();
	}
}
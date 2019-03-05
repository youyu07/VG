#pragma once

#include "common.h"
#include <map>

#include "util.h"

namespace vg::vk
{
    
    class IBuffer
    {
        inline void setIsDynamic(VkBool32 value)
        {
            isDynamic = value;
        }
    protected:
        VkBool32 isDynamic = VK_FALSE;
        uint32_t size = 0;
    };

    class VertexBuffer : public IBuffer, public __VkObject<VkBuffer>
    {
    public:

		void update(uint32_t size,const void* data)
		{

		}

		void setBindingAndAttribute(uint32_t binding, const std::vector<VkFormat>& formats)
		{
			bindings[binding] = formats;
		}

		std::vector<VkVertexInputBindingDescription> getBindings()
		{
			std::vector<VkVertexInputBindingDescription> r;
			for (auto& it : bindings)
			{
				uint16_t stride = 0;
				for (auto& f : it.second)
				{
					stride += util::getFormatSize(f);
				}
				r.push_back({ it.first,stride,VK_VERTEX_INPUT_RATE_VERTEX });
			}
			return std::move(r);
		}

		std::vector<VkVertexInputAttributeDescription> getAttributes()
		{
			std::vector<VkVertexInputAttributeDescription> r;
			uint32_t location = 0;
			for (auto& it : bindings)
			{
				uint16_t offset = 0;
				for (auto& f : it.second)
				{
					r.push_back({ location, it.first,f,offset });
					offset += util::getFormatSize(f);
				}
			}
			return std::move(r);
		}
	private:
		std::map<uint32_t, std::vector<VkFormat>> bindings;
    };

	struct SamplerInfo
	{

	};

	struct ImageInfo
	{
		uint32_t width;
		uint32_t height;
		VkFormat format = VK_FORMAT_UNDEFINED;
		uint32_t mipLevels = 1;
		uint32_t layers = 1;
		VkImageType type = VK_IMAGE_TYPE_2D;
		VkImageUsageFlags usage;
	};

	class Image : public IBuffer, public __VkObject<VkImage>
	{
	public:
		SamplerInfo sampler = {};
		ImageInfo info = {};
		VmaAllocation allocation = VK_NULL_HANDLE;

		Image(class Device* device,const ImageInfo& info);
	private:
		void transitionImageLayout(VkImage img, VkImageLayout oldLayout, VkImageLayout newLayout);


		class Device* device = nullptr;
	};
} // vg
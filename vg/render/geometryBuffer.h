#pragma once
#include "device.h"
#include "util.h"
#include "geometryInfo.h"

namespace vg
{

	struct VertexInfo
	{
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;
	};

	struct GeometryBuffer
	{
		vk::Buffer* position = nullptr;
		vk::Buffer* normal = nullptr;
		vk::Buffer* texcoord = nullptr;
		vk::Buffer* indices = nullptr;
		VkFormat positionFormat, normalFormat, texcoordFormat;
		VkIndexType indexType = VK_INDEX_TYPE_UINT16;
		uint32_t count = 0;

		void draw(vk::CommandBuffer* cmd)
		{
			std::vector<VkBuffer> buffers;
			if (position) {
				buffers.push_back(*position);
			}
			if (normal) {
				buffers.push_back(*normal);
			}
			if (texcoord) {
				buffers.push_back(*texcoord);
			}

			const auto offset = std::vector<VkDeviceSize>(buffers.size(),0);
			cmd->bindVertex(buffers, offset);
			if (indices) {
				cmd->bindIndex(*indices, indexType);
				cmd->drawIndex(count, 1);
			}
			else {
				cmd->draw(count, 1);
			}
		}

		VertexInfo getBindingInfo()
		{
			std::vector<VkVertexInputBindingDescription> b;
			std::vector<VkVertexInputAttributeDescription> a;
			uint32_t binding = 0;
			if (position) {
				b.push_back({ binding, vk::util::getFormatSize(positionFormat), VK_VERTEX_INPUT_RATE_VERTEX });
				a.push_back({ binding,binding,positionFormat,0 });
				binding++;
			}
			if (normal) {
				b.push_back({ binding,vk::util::getFormatSize(normalFormat), VK_VERTEX_INPUT_RATE_VERTEX });
				a.push_back({ binding,binding,normalFormat,0 });
				binding++;
			}
			if (texcoord) {
				b.push_back({ binding,vk::util::getFormatSize(texcoordFormat), VK_VERTEX_INPUT_RATE_VERTEX });
				a.push_back({ binding,binding,texcoordFormat,0 });
				binding++;
			}

			return { std::move(b), std::move(a) };
		}

		static GeometryBuffer* create(vk::Device* device,const GeometryBufferInfo& geometry) {
			auto getDeviceFormat = [](GeometryBufferInfo::Format format) {
				switch (format)
				{
				case vg::GeometryBufferInfo::Format::r16:
					return VK_FORMAT_R16_SFLOAT;
				case vg::GeometryBufferInfo::Format::rg32:
					return VK_FORMAT_R32G32_SFLOAT;
				case vg::GeometryBufferInfo::Format::rgb32:
					return VK_FORMAT_R32G32B32_SFLOAT;
				case vg::GeometryBufferInfo::Format::rgba32:
					return VK_FORMAT_R32G32B32A32_SFLOAT;
				}
				return VK_FORMAT_UNDEFINED;
			};

			auto createBuffer = [&](uint32_t size, VkBufferUsageFlags usage, const void* ptr) -> vk::Buffer * {
				const vk::BufferInfo info = { size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vk::MemoryUsage::GPU };
				auto buffer = device->createBuffer(info);
				buffer->update(size, ptr);
				return buffer;
			};

			auto result = new GeometryBuffer();

			if (geometry.hasType(GeometryBufferInfo::Type::position)) {
				auto data = geometry.getData(GeometryBufferInfo::Type::position);
				auto format = getDeviceFormat(data.format);
				result->position = createBuffer(data.count * vk::util::getFormatSize(format), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data.ptr);
				result->positionFormat = format;
				result->count = data.count;
			}

			if (geometry.hasType(GeometryBufferInfo::Type::normal)) {
				auto data = geometry.getData(GeometryBufferInfo::Type::normal);
				auto format = getDeviceFormat(data.format);
				result->normal = createBuffer(data.count * vk::util::getFormatSize(format), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data.ptr);
				result->normalFormat = format;
			}

			if (geometry.hasType(GeometryBufferInfo::Type::texcoord)) {
				auto data = geometry.getData(GeometryBufferInfo::Type::texcoord);
				auto format = getDeviceFormat(data.format);
				result->texcoord = createBuffer(data.count * vk::util::getFormatSize(format), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data.ptr);
				result->texcoordFormat = format;
			}

			if (geometry.hasType(GeometryBufferInfo::Type::index)) {
				auto data = geometry.getData(GeometryBufferInfo::Type::index);
				auto format = getDeviceFormat(data.format);
				result->indices = createBuffer(data.count * vk::util::getFormatSize(format), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, data.ptr);
				if (format = VK_FORMAT_R16_SFLOAT)result->indexType = VK_INDEX_TYPE_UINT16;
				result->count = data.count;
			}

			return result;
		}
	};

}
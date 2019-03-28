#pragma once
#include "context.h"
#include "geometryInfo.h"

namespace vg
{
	struct GeometryBuffer
	{
		vku::VertexBuffer vertexBuffer;
		vku::IndexBuffer indexBuffer;

		std::vector<vk::VertexInputBindingDescription> bindings;
		std::vector<vk::VertexInputAttributeDescription> attributes;

		vk::IndexType indexType = vk::IndexType::eUint16;
		uint32_t count = 0;

		GeometryBuffer() {}

		GeometryBuffer(const Context& ctx,const GeometryBufferInfo& info) {
			vertexBuffer = vku::VertexBuffer(ctx, ctx.getMemoryProperties(), info.vertexSize);
			vertexBuffer.upload(ctx, ctx.getMemoryProperties(), ctx.getCommandPool(), ctx.getGraphicsQueue(), info.vertex, info.vertexSize);

			indexBuffer = vku::IndexBuffer(ctx, ctx.getMemoryProperties(), info.indexSize);
			indexBuffer.upload(ctx, ctx.getMemoryProperties(), ctx.getCommandPool(), ctx.getGraphicsQueue(), info.index, info.indexSize);

			uint32_t offset = 0;
			if ((info.flags & VertexType::position) == VertexType::position) {
				attributes.emplace_back(0, 0, vk::Format::eR32G32B32Sfloat, offset);
				offset += sizeof(float) * 3;
			}
			if ((info.flags & VertexType::normal) == VertexType::normal) {
				attributes.emplace_back(1, 0, vk::Format::eR32G32B32Sfloat, offset);
				offset += sizeof(float) * 3;
			}
			if ((info.flags & VertexType::texcoord) == VertexType::texcoord) {
				attributes.emplace_back(2, 0, vk::Format::eR32G32Sfloat, offset);
				offset += sizeof(float) * 2;
			}

			bindings.emplace_back(0, offset);

			if (info.indexType == IndexType::u32) {
				indexType = vk::IndexType::eUint32;
				count = info.indexSize >> 2;
			}
			else {
				count = info.indexSize >> 1;
			}
		}

		void draw(vk::CommandBuffer cmd)
		{
			vk::DeviceSize offset = 0;
			cmd.bindVertexBuffers(0, vertexBuffer.buffer(), offset);
			cmd.bindIndexBuffer(indexBuffer.buffer(), 0, indexType);
			cmd.drawIndexed(count, 1, 0, 0, 0);
		}

		std::vector<vk::VertexInputBindingDescription> getBindingInfo()
		{
			return bindings;
		}
		std::vector<vk::VertexInputAttributeDescription> getAttributeInfo()
		{
			return attributes;
		}
	};

}
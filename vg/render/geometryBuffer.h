#pragma once
#include "context.h"
#include "geometryInfo.h"
#include <functional>

namespace vg
{
	struct GeometryBuffer
	{
		vk::Buffer vertexBuffer;
		vk::Buffer indexBuffer;

		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;

		VkIndexType indexType = VK_INDEX_TYPE_UINT16;
		uint32_t count = 0;

		GeometryBuffer() {}

		GeometryBuffer(const Context& ctx,const GeometryBufferInfo& info) {
			vertexBuffer = ctx->getDevice()->createVertexBuffer(info.vertexSize);
			vertexBuffer->upload(ctx->getCommandPool(), ctx->getGraphicsQueue(), info.vertex);
			indexBuffer = ctx->getDevice()->createIndexBuffer(info.vertexSize);
			indexBuffer->upload(ctx->getCommandPool(), ctx->getGraphicsQueue(), info.index);

			uint32_t offset = 0;
			if ((info.flags & VertexType::position) == VertexType::position) {
				attributes.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offset });
				offset += sizeof(float) * 3;
			}
			if ((info.flags & VertexType::normal) == VertexType::normal) {
				attributes.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offset });
				offset += sizeof(float) * 3;
			}
			if ((info.flags & VertexType::texcoord) == VertexType::texcoord) {
				attributes.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, offset });
				offset += sizeof(float) * 2;
			}

			bindings.push_back({ 0, offset, VK_VERTEX_INPUT_RATE_VERTEX });

			if (info.indexType == IndexType::u32) {
				indexType = VK_INDEX_TYPE_UINT32;
				count = info.indexSize >> 2;
			}
			else {
				count = info.indexSize >> 1;
			}
		}
	};


	class GeometryManager
	{
		std::unordered_map<uint32_t, GeometryBuffer> geometries;

	public:
		void addGeometry(const Context& ctx, uint32_t id, const GeometryBufferInfo& info) {
			if (geometries.find(id) == geometries.end()) {
				geometries[id] = GeometryBuffer(ctx, info);
			}
			else {
				log_error("Geometry id is exist : ", id);
			}
		}

		void draw(vk::CommandBuffer& cmd)
		{
			for (auto& g : geometries)
			{
				auto& l = g.second;

				VkDeviceSize offset = { 0 };
				cmd->bindVertexBuffer(0, l.vertexBuffer->get(), offset);
				cmd->bindIndexBuffer(l.indexBuffer->get(), 0, l.indexType);
				cmd->drawIndexd(l.count, 1);
			}
		}

		void draw(std::function<void(uint32_t,const GeometryBuffer&)> callback) 
		{
			for (auto& g : geometries)
			{
				callback(g.first, g.second);
			}
		}
	};
}
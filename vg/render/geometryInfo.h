#pragma once

#include <unordered_map>

namespace vg
{
	enum VertexType
	{
		none = 0x00,
		position = 0x01,
		normal = 0x02,
		texcoord = 0x04,
		PNT = position | normal | texcoord
	};

	enum class IndexType
	{
		u16,
		u32
	};

	struct GeometryBufferInfo
	{
		VertexType flags;
		IndexType indexType;

		void* vertex = nullptr;
		void* index = nullptr;
		uint32_t vertexSize = 0;
		uint32_t indexSize = 0;

		void vertexData(uint32_t size, void* data, VertexType flag = VertexType::position)
		{
			vertex = data;
			vertexSize = size;
			flags = flag;
		}

		void indexData(uint32_t size, void* data, IndexType type = IndexType::u16) {
			index = data;
			indexSize = size;
			indexType = type;
		}
	};
}
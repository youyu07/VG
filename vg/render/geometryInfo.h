#pragma once

#include <unordered_map>

namespace vg
{
	struct GeometryBufferInfo
	{
		enum class IndexType
		{
			u16,
			u32
		};

		enum Flag
		{
			position = 0x01,
			normal = 0x02,
			texcoord = 0x04
		};

		void vertexData(uint32_t size, void* data, Flag flag = Flag::position)
		{
			vertexData = data;
			vertexSize = size;
			flags = flag;
		}
	private:
		void* vertexData = nullptr;
		void* indexData = nullptr;
		uint32_t vertexSize = 0;
		uint32_t indexSize = 0;
		IndexType indexType = IndexType::u16;
		Flag flags = 0;
		
	};
}
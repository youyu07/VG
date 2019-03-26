#pragma once

#include <unordered_map>

namespace vg
{
	struct GeometryBufferInfo
	{
		enum class Format
		{
			undefined,
			r16,
			rg32,
			rgb32,
			rgba32
		};

		enum class Type
		{
			position,
			normal,
			texcoord,
			index
		};

		struct Data
		{
			Format format = Format::undefined;
			uint32_t count = 0;
			void* ptr = nullptr;
		};

		GeometryBufferInfo(const Data& position)
		{
			data[Type::position] = position;
		}

		GeometryBufferInfo(const Data& position,const Data& index) : GeometryBufferInfo(position)
		{
			data[Type::index] = index;
		}

		GeometryBufferInfo(const Data& position, const Data& normal, const Data& index) : GeometryBufferInfo(position,index)
		{
			data[Type::normal] = normal;
		}

		const Data& getData(Type type) const {
			return data.at(type);
		}

		bool hasType(Type type) const
		{
			return data.find(type) != data.end();
		}
	private:
		std::unordered_map<Type, Data> data;
	};
}
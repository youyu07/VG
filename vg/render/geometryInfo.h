#pragma once

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

		GeometryBufferInfo(const Data& position,const Data& index)
		{
			data[Type::position] = position;
			data[Type::index] = index;
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
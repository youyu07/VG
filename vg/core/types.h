#pragma once
#include <iostream>
#include <map>

namespace vg
{
    
    enum class Format : uint8_t
    {
        Undefined,
        R8,
        Rg8,
        Rgb8,
        Rgba8,
		Bgra8,
		D32,
		D24_s8,
        R16f,
        Rg16f,
        Rgb16f,
        Rgba16f,
        R32f,
        Rg32f,
        Rgb32f,
        Rgba32f,
        R16u,
        Rg16u,
        Rgb16u,
        Rgba16u,
        R32u,
        Rg32u,
        Rgb32u,
        Rgba32u,
        R16i,
        Rg16i,
        Rgb16i,
        Rgba16i,
        R32i,
        Rg32i,
        Rgb32i,
        Rgba32i,
    };

    struct VertexAttribute
    {
        std::map<uint32_t,Format> formats;
    };
} // name

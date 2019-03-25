#pragma once
#include <cstdint>
#include "format.h"

namespace vg
{
    enum
    {
        MAX_ATTRIBUTE = 16,
    };

    struct Shader;
    struct Buffer;

    enum class MemoryUsage
    {
        UNKNOWN = 0,
        GPU_ONLY,
        CPU_ONLY,
        CPU_TO_GPU,
        GPU_TO_CPU,
        MAX_ENUM = 0x7FFFFFFF
    };

    enum class DescriptorType
    {
        UNDEFINED,
        SAMPLER,
        TEXTURE,
        UNIFORM,
        VERTEX,
        INDEX,
        INDIRECT
    };

    enum ShaderStage
    {
        SHADER_NONE = 0,
        SHADER_VERT = 0x00000001,
        SHADER_TESC = 0x00000002,
        SHADER_TESE = 0x00000004,
        SHADER_GEOM = 0x00000008,
        SHADER_FRAG = 0x00000010,
        SHADER_COMP = 0x00000020
    };

    struct VertexLayout
    {
        uint32_t count = 0;
        struct Attribute
        {
            uint32_t binding = 0;
            uint32_t location = 0;
            Format format;
        }attributes[MAX_ATTRIBUTE];
    };

    struct GraphicsPipelineDesc
    {
        ShaderStage stage;
        Shader* shaders;
        VertexLayout vertexLayout;
    };

    struct PipelineDesc
    {
        enum class Type
        {
            Graphics,
            Computer
        }type;
        union
        {
            GraphicsPipelineDesc graphics;
        };
    };
    
    struct BufferDesc
    {
        uint32_t size = 0;
        MemoryUsage memoryUsage;
        DescriptorType type;
    };
} // vg

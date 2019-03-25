#pragma once
#include <cstdint>
#include <string>
#include "format.h"

namespace vg
{
    enum
    {
        MAX_ATTRIBUTE = 16,
        MAX_PUSH_CONSTANT_RANGE = 4
    };

    struct Shader;
    struct Buffer;
    struct PipelineLayout;

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
        SHADER_COMP = 0x00000020,
    };

    enum class ShaderType
    {
        SPV,
		GLSL,
		SPV_FILE,
		GLSL_FILE
    };

    struct ShaderDesc
    {
        ShaderStage stage;
        std::string source;
		ShaderType type = ShaderType::SPV_FILE;
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

    enum class PrimitiveType
    {
        Point_List,
        Line_List,
        Line_Strip,
        Triangles_List,
        Triangles_Strip,
        Triangles_Fan
    };

    struct GraphicsPipelineDesc
    {
        ShaderStage stage;
        Shader* shaders[5] = {};
        VertexLayout vertexLayout;
        PipelineLayout* layout;
        PrimitiveType primitiveType = PrimitiveType::Triangles_List;
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

    struct PipelineLayoutDesc
    {
        uint32_t setLayoutCount = 0;
        DescriptorSetLayout* layouts;
        struct PushConstant
        {
            ShaderStage stage;
            uint32_t offset = 0;
            uint32_t size = 0;
        }pushConstant[MAX_PUSH_CONSTANT_RANGE];
    };
    
    struct BufferDesc
    {
        uint32_t size = 0;
        MemoryUsage memoryUsage;
        DescriptorType type;
    };
} // vg

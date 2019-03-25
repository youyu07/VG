#pragma once

#include "type.h"

#include <vulkan/vulkan.h>

namespace vg
{
    struct Shader
    {
        ShaderStage stage;
        VkShaderModule module;
    };

    struct DescriptorSetLayout
    {
        VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    };

    struct PipelineLayout
    {
        VkPipelineLayout layout = VK_NULL_HANDLE;
    };

    struct Pipeline
    {
        VkPipeline pipeline = VK_NULL_HANDLE;
    };
} // vg

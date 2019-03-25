#include "type.h"
#include "vktypes.h"
#include <vector>

namespace vg
{
    static VkShaderStageFlagBits getShaderStage(ShaderStage stage)
    {
        switch (stage)
        {
            case SHADER_VERT:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case SHADER_TESC:
                return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            case SHADER_TESE:
                return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            case SHADER_GEOM:
                return VK_SHADER_STAGE_GEOMETRY_BIT;
            case SHADER_FRAG:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            case SHADER_COMP:
                return VK_SHADER_STAGE_COMPUTE_BIT;
        }
        return VK_SHADER_STAGE_ALL;
    }

    static VkShaderStageFlags getShaderFlag(ShaderStage stage)
    {
        VkShaderStageFlags flags;
        for(size_t i = 0; i < 6; i++)
        {
            if(stage & (1 >> i)){
                flags |= getShaderStage(ShaderStage(i));
            }
        }
        return flags;
    }

    void createPipelineLayout(VkDevice device,const PipelineLayoutDesc& desc)
    {
        VkPipelineLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

        std::vector<VkDescriptorSetLayout> layouts;
        for(size_t i = 0; i < desc.setLayoutCount; i++){
            layouts.push_back(desc.layouts[i].setLayout);
        }

		createInfo.setLayoutCount = desc.setLayoutCount;
		createInfo.pSetLayouts = layouts.data();

        std::vector<VkPushConstantRange> pushConstant;
        for(size_t i = 0; i < MAX_PUSH_CONSTANT_RANGE; i++){
            const auto& range = desc.pushConstant[i];
            pushConstant.push_back({getShaderFlag(range.stage),range.offset,range.size});
        }

		createInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstant.size());
		createInfo.pPushConstantRanges = pushConstant.data();

        VkPipelineLayout layout = VK_NULL_HANDLE;
		vkCreatePipelineLayout(device, &createInfo, nullptr, &layout);
    }

    static Pipeline* createGraphicsPipeline(VkDevice device,const GraphicsPipelineDesc& desc)
    {
        std::vector<VkPipelineShaderStageCreateInfo> stages;
        for(size_t i = 0; i < 5; i++)
        {
            const auto shader = desc.shaders[i];
            if(shader->module){
                VkPipelineShaderStageCreateInfo info = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
                info.module = shader->module;
                info.stage = getShaderStage(shader->stage);
                stages.push_back(info);
            }
        }

        VkPipelineVertexInputStateCreateInfo vertexState = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        for(size_t i = 0; i < desc.vertexLayout.count; i++){
            
        }
        
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;
		inputAssemblyState.topology = static_cast<VkPrimitiveTopology>(desc.primitiveType);

		VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;
		depthStencilState.minDepthBounds = 0.0f;
		depthStencilState.maxDepthBounds = 1.0f;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

		VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {
			VK_FALSE,
			VK_BLEND_FACTOR_SRC_ALPHA,VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,VK_BLEND_FACTOR_ZERO,VK_BLEND_OP_ADD,
			0xf
		};
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &colorBlendAttachment;

		VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		std::vector<VkDynamicState> dynamics = { VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR };
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamics.size());
		dynamicState.pDynamicStates = dynamics.data();

        VkGraphicsPipelineCreateInfo info = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        info.layout = desc.layout->layout;
        info.renderPass = info.renderPass;

        info.stageCount = static_cast<uint32_t>(stages.size());
        info.pStages = stages.data();

		info.pVertexInputState = &vertexState;
		info.pInputAssemblyState = &inputAssemblyState;
		info.pTessellationState = nullptr;
		info.pViewportState = &viewportState;
		info.pRasterizationState = &rasterizationState;
		info.pMultisampleState = &multisampleState;
		info.pDepthStencilState = &depthStencilState;
		info.pColorBlendState = &colorBlendState;
		info.pDynamicState = &dynamicState;

        VkPipeline pipeline = VK_NULL_HANDLE;
		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline);

        return new Pipeline{pipeline};
    }

    Pipeline* createPipeline(const PipelineDesc& desc)
    {
        switch (desc.type)
        {
            case PipelineDesc::Type::Graphics:
                return createGraphicsPipeline(VK_NULL_HANDLE,desc.graphics);
        }
        return nullptr;
    }

    void destroyPipeline(Pipeline* pipeline)
    {

    }
} // vg
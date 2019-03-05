#pragma once
#include "device.h"
#include <map>
#include "util.h"

namespace vg::vk
{
	struct DescriptorBufferViewDesc
	{
		std::map<uint32_t, Buffer*> buffers;
		std::map<uint32_t, ImageView*> imageViews;

		bool operator==(const DescriptorBufferViewDesc& desc)
		{
			return buffers == desc.buffers && imageViews == desc.imageViews;
		}
	};

	class DescriptorSetLayout : public __VkObject<VkDescriptorSetLayout>
	{
		VkDevice device = VK_NULL_HANDLE;
	public:
		DescriptorSetLayout(VkDevice dev, const DescriptorBufferViewDesc& desc) : device(dev)
		{
			bindings = calcBindings(desc);
			VkDescriptorSetLayoutCreateInfo info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			info.bindingCount = static_cast<uint32_t>(bindings.size());
			info.pBindings = bindings.data();
			vkCreateDescriptorSetLayout(device, &info, nullptr, &handle);
		}

		inline static std::vector<VkDescriptorSetLayoutBinding> calcBindings(const DescriptorBufferViewDesc& desc)
		{
			std::vector<VkDescriptorSetLayoutBinding> r;
			for (auto& it : desc.buffers) {
				r.push_back({ it.first,it.second->getDescriptorType(),1,VK_SHADER_STAGE_ALL_GRAPHICS,nullptr });
			}
			for (auto& it : desc.imageViews) {
				r.push_back({ it.first,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,VK_SHADER_STAGE_ALL_GRAPHICS,nullptr });
			}
			return std::move(r);
		}

		bool operator==(const DescriptorBufferViewDesc& desc)
		{
			return bindings == calcBindings(desc);
		}
	private:
		std::vector<VkDescriptorSetLayoutBinding> bindings;
	};

	class DescriptorSet : public __VkObject<VkDescriptorSet>
	{
		VkDevice device = VK_NULL_HANDLE;
	public:
		DescriptorSet(VkDevice dev, VkDescriptorPool pool, const DescriptorSetLayout& layout, const DescriptorBufferViewDesc& desc) : device(dev), bufferViewDesc(desc)
		{
			VkDescriptorSetAllocateInfo info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
			info.descriptorSetCount = 1;
			info.pSetLayouts = layout.getHandlePtr();
			info.descriptorPool = pool;

			vkAllocateDescriptorSets(device, &info, &handle);

			std::vector<VkWriteDescriptorSet> writes;
			VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			write.descriptorCount = 1;
			write.dstArrayElement = 1;
			write.dstSet = handle;

			for (auto& it : desc.buffers) {
				write.dstBinding = it.first;
				write.pBufferInfo = it.second->getDescriptorInfo();
				write.descriptorType = it.second->getDescriptorType();
				writes.push_back(write);
			}

			for (auto& it : desc.imageViews) {
				write.dstBinding = it.first;
				write.pImageInfo = it.second->getDescriptorInfo();
				write.descriptorType = it.second->getDescriptorType();
				writes.push_back(write);
			}

			vkUpdateDescriptorSets(device, 1, nullptr, 0, nullptr);
		}

		bool operator==(const DescriptorBufferViewDesc& desc)
		{
			return bufferViewDesc == desc;
		}

	private:
		DescriptorBufferViewDesc bufferViewDesc;
	};

	class PipelineLayout : public __VkObject<VkPipelineLayout>
	{
		VkDevice device = VK_NULL_HANDLE;
	public:
		PipelineLayout(VkDevice dev, const std::vector<VkDescriptorSetLayout> layouts) : device(dev)
		{
			VkPipelineLayoutCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
			info.setLayoutCount = static_cast<uint32_t>(layouts.size());
			info.pSetLayouts = layouts.data();
			vkCreatePipelineLayout(device, &info, nullptr, &handle);
		}
	};

	struct ShaderSource
	{
		ShaderLanguageType lang;
		std::string src;
	};

	struct PipelineDesc
	{
		std::map<ShaderType, ShaderSource> shaders;
		std::vector<std::vector<Format>> vertexFormat;
		PrimitiveType primitiveType = PrimitiveType::Triangles;
	};

	class Pipeline : public IPipeline, public __VkObject<VkPipeline>
	{
		using IPipeline::IPipeline;
	public:
		void setup(VkRenderPass renderPass, VkPipelineLayout layout)
		{
			std::vector<VkPipelineShaderStageCreateInfo> stages;
			for (auto& it : shaders)
			{
				VkPipelineShaderStageCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
				info.stage = it.first;
				info.module = it.second;
				stages.push_back(info);
			}

			std::vector<VkVertexInputBindingDescription> vertexBindings;
			std::vector<VkVertexInputAttributeDescription> vertexAttributes;
			VkPipelineVertexInputStateCreateInfo vertexInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
			{
				for (uint32_t binding = 0; binding < desc.vertexAttributes.size(); binding++)
				{
					uint32_t stride = 0;
					for (auto& f : desc.vertexAttributes.at(binding))
					{
						vertexAttributes.push_back({ static_cast<uint32_t>(vertexAttributes.size()),binding,Util::convertFormat(f),stride });
						stride += Util::getFormatSize(f);
					}
					vertexBindings.push_back({ binding,stride,VK_VERTEX_INPUT_RATE_VERTEX });
				}

				vertexInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindings.size());
				vertexInfo.pVertexBindingDescriptions = vertexBindings.data();
				vertexInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size());
				vertexInfo.pVertexAttributeDescriptions = vertexAttributes.data();
			}

			VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
			inputAssemblyState.primitiveRestartEnable = VK_FALSE;
			inputAssemblyState.topology = Util::convertPrimitiveType(desc.primitiveType);

			VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
			viewportState.viewportCount = 1;
			viewportState.scissorCount = 1;

			VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
			rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizationState.lineWidth = 1.0f;

			VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

			VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
			depthStencilState.depthTestEnable = VK_FALSE;
			depthStencilState.depthWriteEnable = VK_FALSE;
			depthStencilState.stencilTestEnable = VK_FALSE;

			VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
			VkPipelineColorBlendAttachmentState colorBlendAttachment = {
				VK_FALSE,
				VK_BLEND_FACTOR_ZERO,VK_BLEND_FACTOR_ONE,VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,VK_BLEND_FACTOR_DST_ALPHA,VK_BLEND_OP_ADD,
				0xf
			};
			colorBlendState.attachmentCount = 1;
			colorBlendState.pAttachments = &colorBlendAttachment;

			VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
			std::vector<VkDynamicState> dynamics = { VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR };
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamics.size());
			dynamicState.pDynamicStates = dynamics.data();

			VkGraphicsPipelineCreateInfo info = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
			info.layout = layout;
			info.renderPass = renderPass;
			info.stageCount = static_cast<uint32_t>(stages.size());
			info.pStages = stages.data();
			info.pVertexInputState = &vertexInfo;
			info.pInputAssemblyState = &inputAssemblyState;
			info.pTessellationState = nullptr;
			info.pViewportState = &viewportState;
			info.pRasterizationState = &rasterizationState;
			info.pMultisampleState = &multisampleState;
			info.pDepthStencilState = &depthStencilState;
			info.pColorBlendState = &colorBlendState;
			info.pDynamicState = &dynamicState;

			auto& device = getDevice<Device>();
			vkCreateGraphicsPipelines(device, VkPipelineCache(), 1, &info, nullptr, &handle);

			for (auto& stage : stages)
			{
				vkDestroyShaderModule(device, stage.module, nullptr);
			}
		}

		virtual void setShader(const ShaderType& type, const std::string& src, ShaderLanguageType language = ShaderLanguageType::SPRV) override
		{
			auto& device = getDevice<Device>();
			auto stage = Util::convertShaderType(type);

			VkShaderModuleCreateInfo shaderInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			shaderInfo.codeSize = src.size();
			shaderInfo.pCode = (uint32_t*)src.data();

			VkShaderModule obj = VK_NULL_HANDLE;
			vkCreateShaderModule(device, &shaderInfo, nullptr, &obj);

			shaders[stage] = obj;
		}
	private:
		std::map<VkShaderStageFlagBits, VkShaderModule> shaders;
	};

	class RenderContext : public IRenderContext
	{
		using IRenderContext::IRenderContext;
	public:
		virtual void bindVertexBuffers(uint32_t count, IBuffer** buf) override
		{
			vertexBuffers.clear();
			for (uint32_t i = 0; i < count; i++) {
				vertexBuffers.push_back(static_cast<Buffer*>(buf[i]));
			}
		}

		virtual void bindIndexBuffer(IBuffer* buf) override
		{
			indexBuffer = static_cast<Buffer*>(buf);
		}

		virtual void bindBuffer(uint32_t bind, IBuffer* buf) override
		{
			bufferViewDesc.buffers[bind] = static_cast<Buffer*>(buf);
		}

		virtual void bindImageView(uint32_t bind, IImageView* view) override
		{
			bufferViewDesc.imageViews[bind] = static_cast<ImageView*>(view);
		}

		virtual void bindShader(const ShaderType& type, const std::string& src, ShaderLanguageType language = ShaderLanguageType::SPRV) override
		{

		}

		virtual bool setColorAttachment(uint32_t location, IImageView* view) override
		{
			colorAttachments[location] = static_cast<ImageView*>(view);
			return true;
		}

		virtual bool setDepthAttachment(IImageView* view) override
		{
			depthAttachment = static_cast<ImageView*>(view);
		}

		virtual void setVertexAttribute(uint32_t binding, const std::vector<Format>& formats) override
		{
			pipelineDesc.vertexFormat[binding] = formats;
		}

		virtual void draw(const DrawAttribute& attribute) override
		{

		}

		virtual bool submit() override
		{

		}

	private:
		std::map<uint32_t,ImageView*> colorAttachments;
		ImageView* depthAttachment;
		VkRenderPass renderpass = VK_NULL_HANDLE;
		CommandBuffer commandBuffer;

		std::vector<Buffer*> vertexBuffers;
		Buffer* indexBuffer;

		DescriptorBufferViewDesc bufferViewDesc;
		PipelineDesc pipelineDesc;
	};
}
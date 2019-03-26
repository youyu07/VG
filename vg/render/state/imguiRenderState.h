#pragma once

#include "../context.h"
#include <imgui/imgui.h>

namespace vg
{
	class ImguiRenderState
	{
	public:
		vk::UniqueSampler sampler;

		vk::UniqueDescriptorSetLayout setLayout;
		vk::UniquePipelineLayout layout;
		vk::UniquePipeline pipeline;

		vku::TextureImage2D tex;
		vk::ImageView* texView = nullptr;
		vk::DescriptorSet* descriptorSet = nullptr;

		vk::Buffer* vertexBuffer = nullptr;
		vk::Buffer* indexBuffer = nullptr;

		ImguiRenderState(const Context& ctx, vk::RenderPass renderPass)
		{
			setupPipeline(ctx,renderPass);

			ImGuiIO& io = ImGui::GetIO();

			unsigned char* pixels;
			int width, height;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
			size_t upload_size = width * height * 4 * sizeof(char);

			tex = vku::TextureImage2D{ ctx.getDevice(),ctx.getMemoryProperties(),static_cast<uint32_t>(width),static_cast<uint32_t>(height) };
			tex.upload(ctx.getDevice(), std::vector<uint8_t>(pixels, pixels + upload_size), ctx.getCommandPool(), ctx.getMemoryProperties(), ctx.getGraphicsQueue());

			

			descriptorSet = device->createDescriptorSet(setLayout->getPtr());

			VkDescriptorImageInfo descriptorImageInfo = { *sampler,*texView,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
			const vk::DescriptorSet::WriteInfo writeInfo = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,&descriptorImageInfo,nullptr };
			descriptorSet->update({ writeInfo });

			// Store our identifier
			io.Fonts->TexID = (ImTextureID)(intptr_t)tex;
		}

		void setupPipeline(const Context& ctx, vk::RenderPass renderPass)
		{
			const std::string vert =
				"#version 450 core\n"
				"layout(location = 0) in vec2 aPos;\n"
				"layout(location = 1) in vec2 aUV;\n"
				"layout(location = 2) in vec4 aColor;\n"

				"layout(push_constant) uniform uPushConstant {\n"
				"	vec2 uScale;\n"
				"	vec2 uTranslate;\n"
				"} pc;\n"

				"out gl_PerVertex{\n"
				"	vec4 gl_Position;\n"
				"};\n"

				"layout(location = 0) out struct {\n"
				"	vec4 Color;\n"
				"	vec2 UV;\n"
				"} Out;\n"

				"void main()\n"
				"{\n"
				"	Out.Color = aColor;\n"
				"	Out.UV = aUV;\n"
				"	gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);\n"
				"}\n";

			const std::string frag =
				"#version 450 core\n"
				"layout(location = 0) out vec4 fColor;\n"

				"layout(set = 0, binding = 0) uniform sampler2D sTexture;\n"

				"layout(location = 0) in struct {\n"
				"	vec4 Color;\n"
				"	vec2 UV;\n"
				"} In;\n"

				"void main()\n"
				"{\n"
				"	fColor = In.Color * texture(sTexture, In.UV.st);\n"
				"}\n";

			std::vector<VkVertexInputBindingDescription> b = {
				{ 0,4 * 5,VK_VERTEX_INPUT_RATE_VERTEX }
			};
			std::vector<VkVertexInputAttributeDescription> a = {
				{ 0,0,VK_FORMAT_R32G32_SFLOAT,0 },
				{ 1,0,VK_FORMAT_R32G32_SFLOAT,8 },
				{ 2,0,VK_FORMAT_R8G8B8A8_UNORM,16 }
			};

			{
				vku::SamplerMaker sm{};
				vk::UniqueSampler sampler = sm.createUnique(ctx.getDevice());
			}
			{
				vku::DescriptorSetLayoutMaker dsm;
				dsm.image(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1);
				setLayout = dsm.createUnique(ctx.getDevice());
			}
			{
				vku::PipelineLayoutMaker plm{};
				plm.descriptorSetLayout(setLayout.get());
				plm.pushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, 16);
				layout = plm.createUnique(ctx.getDevice());
			}
			{
				auto vertData = ctx.compileGLSLToSpv(vk::ShaderStageFlagBits::eVertex, vert);
				auto fragData = ctx.compileGLSLToSpv(vk::ShaderStageFlagBits::eVertex, frag);
				auto vertShader = vku::ShaderModule(ctx.getDevice(), vertData.begin(), vertData.end());
				auto fragShader = vku::ShaderModule(ctx.getDevice(), fragData.begin(), fragData.end());

				//auto vert = vku::ShaderModule(ctx.getDevice(),)
				//vku::ShaderModule vert_{ ctx.getDevice(), BINARY_DIR "texture.vert.spv" };
				//vku::ShaderModule frag_{ ctx.getDevice(), BINARY_DIR "texture.frag.spv" };

				vku::PipelineMaker pm{ ctx.getExtent().width, ctx.getExtent().height };
				pm.shader(vk::ShaderStageFlagBits::eVertex, vertShader);
				pm.shader(vk::ShaderStageFlagBits::eFragment, fragShader);
			}
			const vk::PipelineInfo info = {
				*renderPass,
				*layout,
				{{vert,VK_SHADER_STAGE_VERTEX_BIT,vk::ShaderType::GLSL},{frag,VK_SHADER_STAGE_FRAGMENT_BIT,vk::ShaderType::GLSL}},
				b, a,
				VK_FALSE, VK_FALSE,
				VK_TRUE
			};

			pipeline = device->createPipeline(info);
		}

		void createOrResizeBuffer(ImDrawData* data)
		{
			// Create the Vertex and Index buffers:
			uint32_t vertex_size = data->TotalVtxCount * sizeof(ImDrawVert);
			uint32_t index_size = data->TotalIdxCount * sizeof(ImDrawIdx);
			if (!vertexBuffer || vertexBuffer->getInfo().size < vertex_size)
			{
				if(vertexBuffer)delete vertexBuffer;

				const vk::BufferInfo info = { vertex_size,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,vk::MemoryUsage::CPU_GPU };
				vertexBuffer = device->createBuffer(info);
			}
			if (!indexBuffer || indexBuffer->getInfo().size < index_size)
			{
				if (indexBuffer)delete indexBuffer;

				const vk::BufferInfo info = { index_size,VK_BUFFER_USAGE_INDEX_BUFFER_BIT,vk::MemoryUsage::CPU_GPU };
				indexBuffer = device->createBuffer(info);
			}

			// Upload Vertex and index Data:
			{
				uint32_t vertexOffset = 0;
				uint32_t indexOffset = 0;
				
				std::vector<char> vertexData(vertex_size);
				std::vector<char> indexData(index_size);
				for (int n = 0; n < data->CmdListsCount; n++)
				{
					const ImDrawList* cmd_list = data->CmdLists[n];
					uint32_t vs = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
					uint32_t is = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);

					memcpy(vertexData.data() + vertexOffset, cmd_list->VtxBuffer.Data, vs);
					memcpy(indexData.data() + indexOffset, cmd_list->IdxBuffer.Data, is);

					vertexOffset += vs;
					indexOffset += is;
				}
				vertexBuffer->update(vertex_size, vertexData.data());
				indexBuffer->update(index_size, indexData.data());
			}
		}

		void draw(vk::CommandBuffer* cmd)
		{
			auto data = ImGui::GetDrawData();
			int width = (int)(data->DisplaySize.x * data->FramebufferScale.x);
			int height = (int)(data->DisplaySize.y * data->FramebufferScale.y);

			if (width >0 && height > 0 && data->TotalVtxCount > 0) {
				createOrResizeBuffer(data);
				cmd->bindPipeline(*pipeline);

				std::vector<VkDeviceSize> offsets = { 0 };
				cmd->bindVertex({ *vertexBuffer }, offsets);
				cmd->bindIndex(*indexBuffer, VK_INDEX_TYPE_UINT16);

				cmd->bindDescriptorSet(*layout, { *descriptorSet }, {});

				float pushData[4];
				pushData[0] = 2.0f / data->DisplaySize.x;
				pushData[1] = 2.0f / data->DisplaySize.y;
				pushData[2] = -1.0f - data->DisplayPos.x * pushData[0];
				pushData[3] = -1.0f - data->DisplayPos.y * pushData[1];
				vkCmdPushConstants(*cmd, *layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 4, pushData);

				// Will project scissor/clipping rectangles into framebuffer space
				ImVec2 clip_off = data->DisplayPos;         // (0,0) unless using multi-viewports
				ImVec2 clip_scale = data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

				int vtx_offset = 0;
				int idx_offset = 0;
				for (int n = 0; n < data->CmdListsCount; n++)
				{
					const ImDrawList* cmd_list = data->CmdLists[n];
					for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
					{
						const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
						if (pcmd->UserCallback)
						{
							pcmd->UserCallback(cmd_list, pcmd);
						}
						else
						{
							// Project scissor/clipping rectangles into framebuffer space
							ImVec4 clip_rect;
							clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
							clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
							clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
							clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

							if (clip_rect.x < width && clip_rect.y < height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
							{
								cmd->setScissor(clip_rect.x, clip_rect.y, clip_rect.z - clip_rect.x, clip_rect.w - clip_rect.y);

								cmd->drawIndex(pcmd->ElemCount, 1, idx_offset, vtx_offset, 0);
							}
						}
						idx_offset += pcmd->ElemCount;
					}
					vtx_offset += cmd_list->VtxBuffer.Size;
				}
			}
		}
	};
}
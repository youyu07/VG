#pragma once

#include "../context.h"
#include <imgui/imgui.h>

namespace vg
{
	class ImguiRenderState
	{
		vk::Sampler sampler;
		vk::Image tex;
		vk::DescriptorSetLayout setLayout;
		vk::PipelineLayout layout;

		vk::Pipeline pipeline;

		vk::DescriptorSet descriptorSet;

		vk::Buffer vertexBuffer;
		vk::Buffer indexBuffer;
	public:
		ImguiRenderState() {}

		ImguiRenderState(const Context& ctx)
		{
			{
				vk::SamplerMaker sm;
				sampler = sm.create(ctx->getDevice());
			}

			{
				ImGuiIO& io = ImGui::GetIO();

				unsigned char* pixels;
				int width, height;
				io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
				size_t upload_size = width * height * 4 * sizeof(char);

				tex = ctx->getDevice()->createTexture2D(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
				tex->upload(ctx->getCommandPool(), ctx->getGraphicsQueue(), pixels);
				io.Fonts->TexID = (ImTextureID)(intptr_t)&tex;
			}
			
			{
				vk::DescriptorSetLayoutMaker dsm;
				dsm.binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
				setLayout = dsm.create(ctx->getDevice());
			}
			{
				vk::PipelineLayoutMaker plm;
				plm.pushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, 16);
				plm.setLayout(setLayout);
				layout = plm.create(ctx->getDevice());
			}

			{
				std::array<VkDescriptorSetLayout, 1> sets = { *setLayout };
				descriptorSet = ctx->getDescriptorPool()->createDescriptorSet(sets);

				vk::DescriptorSetUpdater update;
				update.beginDescriptorSet(descriptorSet);

				update.beginImages(0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				update.image(sampler, tex->view(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				update.update(ctx->getDevice());
			}

			setupPipeline(ctx);
		}

		void setupPipeline(const Context& ctx)
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

			{
				auto pm = vk::PipelineMaker(ctx->getDevice());
				pm.shaderGLSL(VK_SHADER_STAGE_VERTEX_BIT, vert);
				pm.shaderGLSL(VK_SHADER_STAGE_FRAGMENT_BIT, frag);
				pm.vertexBinding(0, sizeof(float) * 5);
				pm.vertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
				pm.vertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 2);
				pm.vertexAttribute(2, 0, VK_FORMAT_R8G8B8A8_UNORM, sizeof(float) * 4);
				pm.dynamicState(VK_DYNAMIC_STATE_VIEWPORT);
				pm.dynamicState(VK_DYNAMIC_STATE_SCISSOR);
				pm.blendBegin(VK_TRUE);
				pm.rasterizationSamples(ctx->getSampleCount());
				pipeline = pm.create(layout, ctx->getRenderPass());
			}
		}

		void createOrResizeBuffer(const Context& ctx,ImDrawData* data)
		{
			// Create the Vertex and Index buffers:
			uint32_t vertex_size = data->TotalVtxCount * sizeof(ImDrawVert);
			uint32_t index_size = data->TotalIdxCount * sizeof(ImDrawIdx);
			if (!vertexBuffer || vertexBuffer->size() < vertex_size)
			{
				vertexBuffer = ctx->getDevice()->createVertexBuffer(vertex_size, VK_TRUE);
			}
			if (!indexBuffer || indexBuffer->size() < index_size)
			{
				indexBuffer = ctx->getDevice()->createIndexBuffer(index_size, VK_TRUE);
			}

			// Upload Vertex and index Data:
			{
				uint32_t vertexOffset = 0;
				uint32_t indexOffset = 0;
				
				auto vp = (char*)vertexBuffer->map();
				auto ip = (char*)indexBuffer->map();
				for (int n = 0; n < data->CmdListsCount; n++)
				{
					const ImDrawList* cmd_list = data->CmdLists[n];
					uint32_t vs = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
					uint32_t is = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);

					memcpy(vp + vertexOffset, cmd_list->VtxBuffer.Data, vs);
					memcpy(ip + indexOffset, cmd_list->IdxBuffer.Data, is);

					vertexOffset += vs;
					indexOffset += is;
				}
				vertexBuffer->unmap();
				indexBuffer->unmap();
				vertexBuffer->flush();
				indexBuffer->flush();
			}
		}

		void draw(const Context& ctx, vk::CommandBuffer& cmd)
		{
			auto data = ImGui::GetDrawData();
			int width = (int)(data->DisplaySize.x * data->FramebufferScale.x);
			int height = (int)(data->DisplaySize.y * data->FramebufferScale.y);

			if (width >0 && height > 0 && data->TotalVtxCount > 0) {
				createOrResizeBuffer(ctx,data);
				cmd->bindPipeline(pipeline);

				std::vector<VkDeviceSize> offsets = { 0 };
				cmd->bindVertexBuffer(0, vertexBuffer->get(), offsets);
				cmd->bindIndexBuffer(indexBuffer->get(), 0);
				cmd->bindDescriptorSet(layout, 0, descriptorSet->get());

				std::vector<float> pushData(4);
				pushData[0] = 2.0f / data->DisplaySize.x;
				pushData[1] = 2.0f / data->DisplaySize.y;
				pushData[2] = -1.0f - data->DisplayPos.x * pushData[0];
				pushData[3] = -1.0f - data->DisplayPos.y * pushData[1];
				cmd->pushContants(layout, VK_SHADER_STAGE_VERTEX_BIT, 0, static_cast<uint32_t>(sizeof(float) * 4), pushData.data());

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
								cmd->scissor(clip_rect.x, clip_rect.y, clip_rect.z - clip_rect.x, clip_rect.w - clip_rect.y);
								cmd->drawIndexd(pcmd->ElemCount, 1, idx_offset, vtx_offset);
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
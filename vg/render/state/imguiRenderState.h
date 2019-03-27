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
		vk::UniqueDescriptorSet descriptorSet;

		vku::GenericBuffer vertexBuffer;
		vku::GenericBuffer indexBuffer;

		ImguiRenderState() {}

		ImguiRenderState(const Context& ctx, vk::RenderPass renderPass)
		{
			setupPipeline(ctx,renderPass);

			{
				vku::SamplerMaker sm{};
				sampler = sm.createUnique(ctx.getDevice());
			}

			{
				ImGuiIO& io = ImGui::GetIO();

				unsigned char* pixels;
				int width, height;
				io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
				size_t upload_size = width * height * 4 * sizeof(char);

				tex = vku::TextureImage2D{ ctx.getDevice(),ctx.getMemoryProperties(),static_cast<uint32_t>(width),static_cast<uint32_t>(height) };
				tex.upload(ctx.getDevice(), std::vector<uint8_t>(pixels, pixels + upload_size), ctx.getCommandPool(), ctx.getMemoryProperties(), ctx.getGraphicsQueue());

				// Store our identifier
				io.Fonts->TexID = (ImTextureID)(intptr_t)&tex;
			}
			
			{
				vku::DescriptorSetMaker dsm;
				dsm.layout(setLayout.get());
				descriptorSet = std::move(dsm.createUnique(ctx.getDevice(), ctx.getDescriptorPool()).at(0));

				vku::DescriptorSetUpdater update;
				update.beginDescriptorSet(descriptorSet.get());

				// Set initial sampler value
				update.beginImages(0, 0, vk::DescriptorType::eCombinedImageSampler);
				update.image(sampler.get(), tex.imageView(), vk::ImageLayout::eShaderReadOnlyOptimal);

				update.update(ctx.getDevice());
			}
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
				auto vertShader = ctx.compileGLSLToSpv(vk::ShaderStageFlagBits::eVertex, vert);
				auto fragShader = ctx.compileGLSLToSpv(vk::ShaderStageFlagBits::eFragment, frag);
				auto pm = vku::PipelineMaker{ ctx.getExtent().width,ctx.getExtent().height };
				pm.shader(vk::ShaderStageFlagBits::eVertex, vertShader);
				pm.shader(vk::ShaderStageFlagBits::eFragment, fragShader);
				pm.vertexBinding(0,sizeof(float)*5);
				pm.vertexAttribute(0, 0, vk::Format::eR32G32Sfloat,0);
				pm.vertexAttribute(1, 0, vk::Format::eR32G32Sfloat, sizeof(float) * 2);
				pm.vertexAttribute(2, 0, vk::Format::eR8G8B8A8Unorm, sizeof(float) * 4);
				pm.dynamicState(vk::DynamicState::eScissor);
				pm.blendBegin(VK_TRUE);

				pipeline = pm.createUnique(ctx.getDevice(), vk::PipelineCache(), layout.get(), renderPass);
			}
		}

		void createOrResizeBuffer(const Context& ctx,ImDrawData* data)
		{
			// Create the Vertex and Index buffers:
			uint32_t vertex_size = data->TotalVtxCount * sizeof(ImDrawVert);
			uint32_t index_size = data->TotalIdxCount * sizeof(ImDrawIdx);
			if (vertexBuffer.size() < vertex_size)
			{
				vertexBuffer = vku::GenericBuffer(ctx, ctx.getMemoryProperties(),
					vk::BufferUsageFlagBits::eVertexBuffer,vertex_size,
					vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			}
			if (indexBuffer.size() < index_size)
			{
				indexBuffer = vku::GenericBuffer(ctx, ctx.getMemoryProperties(),
					vk::BufferUsageFlagBits::eIndexBuffer, index_size,
					vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			}

			// Upload Vertex and index Data:
			{
				uint32_t vertexOffset = 0;
				uint32_t indexOffset = 0;
				
				auto vp = (char*)vertexBuffer.map(ctx);
				auto ip = (char*)indexBuffer.map(ctx);
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
				vertexBuffer.unmap(ctx);
				indexBuffer.unmap(ctx);
				vertexBuffer.flush(ctx);
				indexBuffer.flush(ctx);
			}
		}

		void draw(const Context& ctx, vk::CommandBuffer cmd)
		{
			auto data = ImGui::GetDrawData();
			int width = (int)(data->DisplaySize.x * data->FramebufferScale.x);
			int height = (int)(data->DisplaySize.y * data->FramebufferScale.y);

			if (width >0 && height > 0 && data->TotalVtxCount > 0) {
				createOrResizeBuffer(ctx,data);
				cmd.bindPipeline(vk::PipelineBindPoint::eGraphics,pipeline.get());

				std::vector<VkDeviceSize> offsets = { 0 };
				cmd.bindVertexBuffers(0, vertexBuffer.buffer(), offsets);
				cmd.bindIndexBuffer(indexBuffer.buffer(), 0, vk::IndexType::eUint16);
				cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout.get(), 0, descriptorSet.get(), {});

				std::vector<float> pushData(4);
				pushData[0] = 2.0f / data->DisplaySize.x;
				pushData[1] = 2.0f / data->DisplaySize.y;
				pushData[2] = -1.0f - data->DisplayPos.x * pushData[0];
				pushData[3] = -1.0f - data->DisplayPos.y * pushData[1];
				cmd.pushConstants(layout.get(), vk::ShaderStageFlagBits::eVertex, 0U, static_cast<uint32_t>(sizeof(float)*4),pushData.data());

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
								auto scissor = vk::Rect2D{ {static_cast<int32_t>(clip_rect.x), static_cast<int32_t>(clip_rect.y)}, 
									{static_cast<uint32_t>(clip_rect.z - clip_rect.x), static_cast<uint32_t>(clip_rect.w - clip_rect.y)} };
								cmd.setScissor(0, scissor);
								cmd.drawIndexed(pcmd->ElemCount, 1, idx_offset, vtx_offset, 0);
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
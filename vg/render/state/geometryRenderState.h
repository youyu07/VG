#pragma once

#include "../context.h"
#include "../geometryBuffer.h"

namespace vg
{

	class GeometryRenderState
	{
		vk::PipelineLayout layout;

		struct
		{
			vk::Pipeline line;
			vk::Pipeline fill;
		}pipeline;
		

		SelectInfo selectInfo = {};
	public:
		GeometryRenderState() {}

		GeometryRenderState(const Context& ctx, vk::DescriptorSetLayout& cameraSetLayout)
		{
			auto plm = vk::PipelineLayoutMaker();
			plm.setLayout(cameraSetLayout);
			plm.pushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, 16);
			layout = plm.create(ctx->getDevice());

			setupPipeline(ctx);
		}

		void setupPipeline(const Context& ctx)
		{
			const std::string vert =
				"#version 450\n"
				"layout(location=0)in vec3 position;\n"
				"layout(location=1)in vec3 normal;\n"
				"layout(location=2)in vec2 texcoord;\n"
				"layout(set=0,binding=0) uniform CameraMatrix {\n"
				"	mat4 projection;\n"
				"	mat4 view;\n"
				"} matrix;\n"

				"layout(location=0)out vec3 v_normal;\n"
				"void main(){\n"
				"	gl_Position = matrix.projection * matrix.view * vec4(position,1.0);\n"
				"	v_normal = normal;\n"
				"}";

			const std::string frag =
				"#version 450\n"
				"layout(location=0)in vec3 v_normal;\n"
				"layout(location=0)out vec4 color;\n"
				"layout(push_constant) uniform PushConstant {\n"
				"	uint objectIndex;\n"
				"	uint primitive;\n"
				"	uint color;\n"
				"	uint padding;\n"
				"} pc;\n"
				"void main(){\n"
				"	float r = float((0x000000ff & pc.color) >> 0) / 255.0f;\n"
				"	float g = float((0x0000ff00 & pc.color) >> 8) / 255.0f;\n"
				"	float b = float((0x00ff0000 & pc.color) >> 16) / 255.0f;\n"
				"	float a = float((0xff000000 & pc.color) >> 24) / 255.0f;\n"
				"	color = vec4(r,g,b,a);\n"
				"	if(pc.primitive == gl_PrimitiveID + 1) color = mix(color, vec4(1.0,0.0,0.0,1.0),0.5);\n"
				"}";

			auto pm = vk::PipelineMaker(ctx->getDevice()).defaultBlend(VK_FALSE).defaultDynamic();
			pm.shaderGLSL(VK_SHADER_STAGE_VERTEX_BIT, vert);
			pm.shaderGLSL(VK_SHADER_STAGE_FRAGMENT_BIT, frag);
			pm.vertexBinding(0,32);
			pm.vertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
			pm.vertexAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12);
			pm.vertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, 24);
			pm.depthTestEnable(VK_TRUE);
			pm.depthWriteEnable(VK_TRUE);
			pm.rasterizationSamples(ctx->getSampleCount());
			pipeline.fill = pm.create(layout, ctx->getRenderPass());

			pm.polygonMode(VK_POLYGON_MODE_LINE);
			pipeline.line = pm.create(layout, ctx->getRenderPass());
		}

		void setSelect(const SelectInfo& sel) {
			selectInfo = sel;
		}
		
		void draw(const Context& ctx, vk::CommandBuffer& cmd, vk::DescriptorSet& cameraSet,GeometryManager& geometries)
		{
			uint32_t setOffset = { 0 };
			cmd->bindDescriptorSet(layout, 0, cameraSet->get(), setOffset);

			struct
			{
				uint32_t objectID;
				uint32_t PirmID;
				glm::u8vec4 color;
				uint32_t padding;
			}pc;

			auto doDraw = [&](const glm::u8vec4& color) {
				geometries.draw([&](uint32_t id, const GeometryBuffer & g) {
					if (selectInfo.ObjectID == id + 1) {
						pc = { selectInfo.ObjectID,selectInfo.PrimID,color,0 };
						cmd->pushContants(layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, pc);
					}
					else {
						pc = { 0,0,color,0 };
						cmd->pushContants(layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, pc);
					}

					VkDeviceSize offset = { 0 };
					cmd->bindVertexBuffer(0, g.vertexBuffer->get(), offset);
					cmd->bindIndexBuffer(g.indexBuffer->get(), 0, g.indexType);
					cmd->drawIndexd(g.count, 1);
					});
			};

			cmd->bindPipeline(pipeline.fill);
			doDraw(glm::u8vec4(128,128,128,255));
			cmd->bindPipeline(pipeline.line);
			doDraw(glm::u8vec4(64, 64, 64, 255));
		}
	};

}
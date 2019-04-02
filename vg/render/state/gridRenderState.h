#pragma once
#include "../context.h"
#include <glm/glm.hpp>

namespace vg
{

	class GridRenderState
	{
		vk::PipelineLayout layout;
		vk::Pipeline pipeline;
		vk::Buffer vertexBuffer;
		uint32_t mainLineCount = 0;
		uint32_t lineCount = 0;
	public:
		GridRenderState() {}

		GridRenderState(const Context& ctx,vk::DescriptorSetLayout& cameraSetLayout)
		{
			auto plm = vk::PipelineLayoutMaker();
			plm.setLayout(cameraSetLayout);
			layout = plm.create(ctx->getDevice());

			setupPipeline(ctx);
			setupResource(ctx);
		}

		void setupResource(const Context& ctx)
		{
			auto rectFunc = [](std::vector<glm::vec2>& data,float length, float scale) {
				data.emplace_back(length,1.0f * scale );
				data.emplace_back( -length,1.0f * scale );

				data.emplace_back( 1.0f * scale, length);
				data.emplace_back( 1.0f * scale,-length);
			};

			std::vector<glm::vec2> position;
			rectFunc(position,50.0f, 0.0f);
			mainLineCount = static_cast<uint32_t>(position.size());

			for (float i = -50.0f; i <= 50.0f; i += 1.0f)
			{
				if (i != 0.0f) {
					rectFunc(position, 50.0f, i);
				}
			}
			lineCount = static_cast<uint32_t>(position.size());

			vertexBuffer = ctx->getDevice()->createVertexBuffer(sizeof(glm::vec2) * position.size());
			vertexBuffer->upload(ctx->getCommandPool(),ctx->getGraphicsQueue(),position.data());
		}

		void setupPipeline(const Context& ctx)
		{
			const std::string vert =
				"#version 450 core\n"
				"layout(location = 0) in vec2 aPos;\n"

				"layout(set=0,binding=0) uniform CameraMatrix {\n"
				"	mat4 projection;\n"
				"	mat4 view;\n"
				"} matrix;\n"

				"out gl_PerVertex{\n"
				"	vec4 gl_Position;\n"
				"};\n"

				"void main()\n"
				"{\n"
				"	gl_Position = matrix.projection * matrix.view * vec4(aPos.x, 0.0, aPos.y, 1.0);\n"
				"}\n";

			const std::string frag =
				"#version 450 core\n"
				"layout(location = 0) out vec4 fColor;\n"

				"void main()\n"
				"{\n"
				"	fColor = vec4(0.3,0.7,0.4,1.0);\n"
				"}\n";

			auto pm = vk::PipelineMaker(ctx->getDevice()).defaultBlend(VK_FALSE).defaultDynamic(VK_DYNAMIC_STATE_LINE_WIDTH);
			pm.shaderGLSL(VK_SHADER_STAGE_VERTEX_BIT, vert);
			pm.shaderGLSL(VK_SHADER_STAGE_FRAGMENT_BIT, frag);
			pm.vertexBinding(0, sizeof(glm::vec2));
			pm.vertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
			pm.depthTestEnable(VK_TRUE);
			pm.depthWriteEnable(VK_TRUE);
			pm.topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
			pm.rasterizationSamples(ctx->getSampleCount());
			pipeline = pm.create(layout, ctx->getRenderPass());
		}

		void draw(const Context& ctx, vk::CommandBuffer& cmd, vk::DescriptorSet& cameraSet)
		{
			VkDeviceSize offset = { 0 };
			cmd->bindVertexBuffer(0, vertexBuffer->get(), offset);
			cmd->bindPipeline(pipeline);
			uint32_t setOffset = { 0 };
			cmd->bindDescriptorSet(layout, 0, cameraSet->get(), setOffset);

			cmd->lineWidth(3.0f);
			cmd->draw(mainLineCount, 1);
			cmd->lineWidth(1.0f);
			cmd->draw(lineCount - mainLineCount, 1, mainLineCount);
		}
	};

}
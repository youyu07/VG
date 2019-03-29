#pragma once
#include "../context.h"
#include <glm/glm.hpp>

namespace vg
{

	class GridRenderState
	{
		vk::UniquePipelineLayout layout;
		vk::UniquePipeline pipeline;
		vku::VertexBuffer vertexBuffer;
		uint32_t mainLineCount = 0;
		uint32_t lineCount = 0;
	public:
		GridRenderState() {}

		GridRenderState(const Context& ctx, vk::RenderPass renderPass,vk::DescriptorSetLayout cameraSetLayout)
		{
			auto plm = vku::PipelineLayoutMaker();
			plm.descriptorSetLayout(cameraSetLayout);
			layout = plm.createUnique(ctx);

			setupPipeline(ctx, renderPass);
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

			vertexBuffer = vku::VertexBuffer(ctx, ctx.getMemoryProperties(), sizeof(position));
			vertexBuffer.upload(ctx, ctx.getMemoryProperties(), ctx.getCommandPool(), ctx.getGraphicsQueue(), position);
		}

		void setupPipeline(const Context& ctx, vk::RenderPass renderPass)
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

			auto vertShader = ctx.compileGLSLToSpv(vk::ShaderStageFlagBits::eVertex, vert);
			auto fragShader = ctx.compileGLSLToSpv(vk::ShaderStageFlagBits::eFragment, frag);
			auto pm = vku::PipelineMaker{ ctx.getExtent().width,ctx.getExtent().height };
			pm.shader(vk::ShaderStageFlagBits::eVertex, vertShader);
			pm.shader(vk::ShaderStageFlagBits::eFragment, fragShader);
			pm.vertexBinding(0, sizeof(glm::vec2));
			pm.vertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0);
			pm.topology(vk::PrimitiveTopology::eLineList);
			pm.depthTestEnable(VK_TRUE);
			pm.depthWriteEnable(VK_TRUE);
			pm.lineWidth(1.0f);
			pm.dynamicState(vk::DynamicState::eLineWidth);
			pm.dynamicState(vk::DynamicState::eViewport);
			pm.dynamicState(vk::DynamicState::eScissor);
			pm.rasterizationSamples(Context::getSample());
			pipeline = pm.createUnique(ctx,vk::PipelineCache(),layout.get(),renderPass);
		}

		void draw(const Context& ctx, vk::CommandBuffer cmd, vk::DescriptorSet cameraSet)
		{
			vk::DeviceSize offset = { 0 };

			cmd.bindVertexBuffers(0, vertexBuffer.buffer(), offset);
			cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get());
			uint32_t setOffset = { 0 };
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout.get(), 0, cameraSet, setOffset);

			cmd.setLineWidth(3.0f);
			cmd.draw(mainLineCount, 1, 0, 0);
			cmd.setLineWidth(1.0f);
			cmd.draw(lineCount - mainLineCount, 1, mainLineCount, 0);
		}
	};

}
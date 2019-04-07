#pragma once


namespace vg
{

	class AxisRenderState
	{
		vk::Pipeline pipeline;
		vk::PipelineLayout layout;

	public:
		AxisRenderState() {}

		AxisRenderState(const Context& ctx, vk::DescriptorSetLayout& cameraSetLayout) {

		}

	private:

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
	};

}
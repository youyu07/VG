#pragma once


namespace vg
{

	class PickRenderState
	{
		struct PixelInfo {
			float ObjectID = 0.0f;
			float DrawID = 0.0f;
			float PrimID = 0.0f;
		};

		vk::PipelineLayout layout;
		vk::Pipeline pipeline;
		vk::RenderPass renderPass;
	public:

		void setupRenderPass(const Context& ctx)
		{
			vk::RenderpassMaker rm;
			rm.attachmentBegin(VK_FORMAT_R8G8B8A8_SNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			rm.attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
			rm.attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE);

			rm.attachmentBegin(VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			rm.attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
			rm.attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE);

			rm.subpassBegin();
			rm.subpassColorAttachment(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0);
			rm.subpassDepthStencilAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
			renderPass = rm.create(ctx->getDevice());
		}

		void setupPipeline(const Context& ctx) {
			const std::string vert =
				"#version 450\n"
				"layout(location=0)in vec3 position;\n"
				"layout(set=0,binding=0) uniform CameraMatrix {\n"
				"	mat4 projection;\n"
				"	mat4 view;\n"
				"} matrix;\n"

				"void main(){\n"
				"	gl_Position = matrix.projection * matrix.view * vec4(position,1.0);\n"
				"}";

			const std::string frag =
				"#version 450\n"
				"layout(push_constant) uniform uPushConstant {\n"
				"	uint drawIndex;\n"
				"	uint objectIndex;\n"
				"} pc;\n"

				"layout(location=0)out vec4 color;\n"
				"void main(){\n"
				"	color = vec4(float(gObjectIndex),float(gDrawIndex),float(gl_PrimitiveID + 1),1.0);\n"
				"}";

			auto pm = vk::PipelineMaker(ctx->getDevice()).defaultBlend(VK_FALSE).defaultDynamic();
			pm.shaderGLSL(VK_SHADER_STAGE_VERTEX_BIT, vert);
			pm.shaderGLSL(VK_SHADER_STAGE_FRAGMENT_BIT, frag);
			pm.vertexBinding(0,sizeof(float) * 3);
			pm.vertexAttribute(0,0,VK_FORMAT_R32G32B32_SFLOAT,0);
			pm.depthTestEnable(VK_TRUE);
			pm.depthWriteEnable(VK_TRUE);
			pipeline = pm.create(layout, renderPass);
		}

	};

}
#pragma once

#include "../context.h"
#include "../geometryBuffer.h"

namespace vg
{

	class GeometryRenderState
	{
		vk::PipelineLayout layout;
		vk::Pipeline pipeline;

		std::unordered_map<uint64_t, GeometryBuffer> geometries;
	public:
		GeometryRenderState() {}

		GeometryRenderState(const Context& ctx, vk::DescriptorSetLayout& cameraSetLayout)
		{
			auto plm = vk::PipelineLayoutMaker();
			plm.setLayout(cameraSetLayout);
			layout = plm.create(ctx->getDevice());
		}

		void addGeometry(const Context& ctx, uint64_t id, const GeometryBufferInfo& info) {
			if (geometries.find(id) == geometries.end()) {
				geometries[id] = GeometryBuffer(ctx, info);
			}
			else {
				log_error("Geometry id is exist : ", id);
			}
		}

		void setupPipeline(const Context& ctx,vk::RenderPass& renderPass)
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
				"void main(){\n"
				"	vec3 light = vec3(1.0,1.0,1.0);\n"
				"	float intensity = dot(light,v_normal);\n"
				"	color = vec4(vec3(intensity),1.0);\n"
				"}";

			auto pm = vk::PipelineMaker(ctx->getDevice()).defaultBlend(VK_FALSE).defaultDynamic();
			pm.shaderGLSL(VK_SHADER_STAGE_VERTEX_BIT, vert);
			pm.shaderGLSL(VK_SHADER_STAGE_FRAGMENT_BIT, frag);
			pm.vertexBinding(geometries.begin()->second.bindings);
			pm.vertexAttribute(geometries.begin()->second.attributes);
			pm.depthTestEnable(VK_TRUE);
			pm.depthWriteEnable(VK_TRUE);
			pm.rasterizationSamples(ctx->getSampleCount());
			pipeline = pm.create(layout, ctx->getRenderPass());
		}

		
		void draw(const Context& ctx, vk::CommandBuffer& cmd, vk::RenderPass& renderPass, vk::DescriptorSet& cameraSet)
		{
			if (geometries.size() > 0) {
				if (!pipeline) {
					setupPipeline(ctx, renderPass);
				}

				cmd->bindPipeline(pipeline);
				uint32_t setOffset = { 0 };
				cmd->bindDescriptorSet(layout, 0, cameraSet->get(), setOffset);

				for (auto& g : geometries)
				{
					VkDeviceSize offset = { 0 };
					cmd->bindVertexBuffer(0, g.second.vertexBuffer->get(),offset);
					cmd->bindIndexBuffer(g.second.indexBuffer->get(), 0, g.second.indexType);
					cmd->drawIndexd(g.second.count, 1);
				}
			}
		}
		
	};

}
#pragma once

#include "../context.h"
#include "../geometryBuffer.h"

namespace vg
{

	class GeometryRenderState
	{
		vk::UniquePipelineLayout layout;
		vk::UniquePipeline pipeline;

		std::unordered_map<uint64_t, GeometryBuffer> geometries;
	public:
		GeometryRenderState() {}

		GeometryRenderState(const Context& ctx, vk::DescriptorSetLayout cameraSetLayout)
		{
			auto plm = vku::PipelineLayoutMaker();
			plm.descriptorSetLayout(cameraSetLayout);
			layout = plm.createUnique(ctx);
		}

		void addGeometry(const Context& ctx, uint64_t id, const GeometryBufferInfo& info) {
			if (geometries.find(id) == geometries.end()) {
				geometries[id] = GeometryBuffer(ctx, info);
			}
			else {
				log_error("Geometry id is exist : ", id);
			}
		}

		void setupPipeline(const Context& ctx,vk::RenderPass renderPass)
		{
			const std::string vert =
				"#version 450\n"
				"layout(location=0)in vec3 position;\n"
				"layout(location=1)in vec3 normal;\n"

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

			auto vertShader = ctx.compileGLSLToSpv(vk::ShaderStageFlagBits::eVertex, vert);
			auto fragShader = ctx.compileGLSLToSpv(vk::ShaderStageFlagBits::eFragment, frag);
			auto pm = vku::PipelineMaker{ ctx.getExtent().width,ctx.getExtent().height };
			pm.shader(vk::ShaderStageFlagBits::eVertex, vertShader);
			pm.shader(vk::ShaderStageFlagBits::eFragment, fragShader);
			
			auto bindings = geometries.begin()->second.getBindingInfo();
			auto attributs = geometries.begin()->second.getAttributeInfo();
			for (auto& b : bindings) {
				pm.vertexBinding(b);
			}
			for (auto& a : attributs) {
				pm.vertexAttribute(a);
			}

			pm.topology(vk::PrimitiveTopology::eTriangleList);
			pm.depthTestEnable(VK_TRUE);
			pm.depthWriteEnable(VK_TRUE);
			pm.rasterizationSamples(Context::getSample());
			pm.dynamicState(vk::DynamicState::eViewport);
			pm.dynamicState(vk::DynamicState::eScissor);
			pipeline = pm.createUnique(ctx, vk::PipelineCache(), layout.get(), renderPass);
		}

		
		void draw(const Context& ctx, vk::CommandBuffer cmd, vk::RenderPass renderPass, vk::DescriptorSet cameraSet)
		{
			if (geometries.size() > 0) {
				if (!pipeline) {
					setupPipeline(ctx, renderPass);
				}

				cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get());
				uint32_t setOffset = { 0 };
				cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout.get(), 0, cameraSet, setOffset);


				for (auto& g : geometries)
				{
					g.second.draw(cmd);
				}
			}
		}
		
	};

}
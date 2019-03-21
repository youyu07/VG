#pragma once

namespace vg
{

	class GeometryRenderState : public RenderState
	{
	public:
		vk::PipelineLayout* layout = nullptr;
		vk::Pipeline* pipeline = nullptr;

		GeometryRenderState(vk::Device* device,vk::RenderPass* renderPass) : RenderState(device)
		{
			setupPipeline(renderPass);
		}

		void setupPipeline(vk::RenderPass* renderPass)
		{
			layout = device->createPipelineLayout({});

			const std::string vert =
				"#version 450\n"
				"layout(location=0)in vec3 position;\n"
				"layout(location=1)in vec3 normal;\n"
				"layout(location=0)out vec3 v_normal;\n"
				"void main(){\n"
				"	gl_Position = vec4(position,1.0);\n"
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

			std::vector<VkVertexInputBindingDescription> b = {
				{ 0,4 * 3,VK_VERTEX_INPUT_RATE_VERTEX },
				{ 1,4 * 3,VK_VERTEX_INPUT_RATE_VERTEX }
			};
			std::vector<VkVertexInputAttributeDescription> a = {
				{ 0,0,VK_FORMAT_R32G32B32_SFLOAT,0 },
				{ 1,1,VK_FORMAT_R32G32B32_SFLOAT,4*3 }
			};

			const vk::PipelineInfo info = {
				*renderPass,
				*layout,
				{{vert,VK_SHADER_STAGE_VERTEX_BIT,vk::ShaderType::GLSL},{frag,VK_SHADER_STAGE_FRAGMENT_BIT,vk::ShaderType::GLSL}},
				b,
				a,
				VK_TRUE,VK_TRUE
			};

			pipeline = device->createPipeline(info);
		}

		template<typename Type> void draw(vk::CommandBuffer* cmd,const std::vector<DeviceHandle>& gs)
		{
			cmd->bindPipeline(*pipeline);

			for (auto& g : gs)
			{
				auto geometry = static_cast<Type*>(g);
				geometry->draw(cmd);
			}
		}
	};

}
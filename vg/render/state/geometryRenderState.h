#pragma once

namespace vg
{

	class GeometryRenderState : public RenderState
	{
	public:
		vk::RenderPass* renderPass = nullptr;

		vk::Image* colorImage = nullptr;
		vk::ImageView* colorImageView = nullptr;
		vk::Image* depthImage = nullptr;
		vk::ImageView* depthImageView = nullptr;

		vk::FrameBuffer* frameBuffer;
		vk::CommandBuffer* commandBuffer;

		vk::PipelineLayout* layout = nullptr;
		vk::Pipeline* pipeline = nullptr;

		VkExtent2D extent = {};

		GeometryRenderState(vk::Device* device) : RenderState(device)
		{
			commandBuffer = device->createCommandBuffer();
		}

		void setup(VkExtent2D extent)
		{
			this->extent = extent;

			VkAttachmentDescription colorDescription = {
				0,
				VK_FORMAT_B8G8R8A8_UNORM,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};

			VkAttachmentDescription depthDescription = {
				0,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			};

			VkAttachmentReference colorAttachment = { 0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			VkAttachmentReference depthAttachment = { 1,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

			VkSubpassDescription subpass = {
				0,									//flag
				VK_PIPELINE_BIND_POINT_GRAPHICS,	//pipelineBindPoint
				0,									//inputAttachmentCount
				nullptr,							//pInputAttachments
				1,									//colorAttachmentCount
				&colorAttachment,					//pColorAttachments
				nullptr,							//pResolveAttachments
				&depthAttachment,					//pDepthStencilAttachment
				0,									//preserveAttachmentCount
				nullptr								//pPreserveAttachments
			};

			const vk::RenderPassInfo renderPassInfo = { {colorDescription,depthDescription},{subpass}, {} };
			renderPass = device->createRenderPass(renderPassInfo);

			setupPipeline();

			vk::ImageInfo colorImageInfo = { extent.width,extent.height,colorDescription.format,1,1,VK_IMAGE_TYPE_2D,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT };
			colorImage = device->createImage(colorImageInfo);
			colorImageView = colorImage->createView();

			const vk::ImageInfo depthImageInfo = { extent.width,extent.height,depthDescription.format,1,1,VK_IMAGE_TYPE_2D,VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT };
			depthImage = device->createImage(depthImageInfo);
			depthImageView = depthImage->createView();

			std::vector<VkImageView> attachment = { *colorImageView, *depthImageView };
			const vk::FrameBufferInfo frameBufferInfo = { extent.width,extent.height,*renderPass,attachment };
			frameBuffer = device->createFrameBuffer(frameBufferInfo);
		}

		void setupPipeline()
		{
			layout = device->createPipelineLayout({});

			const std::string vert =
				"#version 450\n"
				"layout(location=0)in vec3 position;\n"
				"layout(location=1)in vec3 normal;"
				"layout(location=0)out vec3 v_normal;"
				"void main(){\n"
				"	gl_Position = vec4(position,1.0);\n"
				"}";

			const std::string frag =
				"#version 450\n"
				"layout(location=0)in vec3 v_normal;"
				"layout(location=0)out vec4 color;\n"
				"void main(){\n"
				"	vec3 light = vec3(1.0,1.0,1.0);"
				"	float intensity = dot(light,v_normal);"
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
				a
			};

			pipeline = device->createPipeline(info);
		}

		template<typename Type> vk::CommandBuffer*& draw(const std::vector<DeviceHandle>& gs)
		{
			commandBuffer->begin();

			const std::vector<VkClearValue> clearValue = {
				{0.0f,0.0f,0.0f,1.0f},
				{1.0,0}
			};

			const VkRect2D renderArea = { {},extent };
			commandBuffer->beginRenderPass(*renderPass, *frameBuffer, renderArea, clearValue);

			commandBuffer->setViewport(extent.width, extent.height);
			commandBuffer->setScissor(0, 0, extent.width, extent.height);

			commandBuffer->bindPipeline(*pipeline);

			for (auto& g : gs)
			{
				auto geometry = static_cast<Type*>(g);
				geometry->draw(commandBuffer);
			}

			commandBuffer->endRenderPass();
			commandBuffer->end();

			return commandBuffer;
		}
	};

}
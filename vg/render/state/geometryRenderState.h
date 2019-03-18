#pragma once

namespace vg
{

	class GeometryRenderState
	{
		vk::Device* device = nullptr;
		vk::Swapchain* swapchain = nullptr;
	public:
		vk::RenderPass* renderPass = nullptr;

		vk::Image* colorImage = nullptr;
		vk::ImageView* colorImageView = nullptr;
		vk::Image* depthImage = nullptr;
		vk::ImageView* depthImageView = nullptr;

		vk::FrameBuffer* frameBuffer;
		vk::CommandBuffer* commandBuffer;

		vk::Semaphore* semaphore = nullptr;
		vk::Fence* fence = nullptr;

		vk::PipelineLayout* layout = nullptr;
		vk::Pipeline* pipeline = nullptr;

		GeometryRenderState(vk::Device* device, vk::Swapchain* swapchain) : device(device), swapchain(swapchain)
		{
			VkAttachmentDescription colorDescription = {
				0,
				swapchain->getColorFormat(),
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

			vk::ImageInfo colorImageInfo = { swapchain->getWidth(),swapchain->getHeight(),colorDescription.format,1,1,VK_IMAGE_TYPE_2D,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL | VK_IMAGE_USAGE_TRANSFER_DST_BIT };
			colorImage = device->createImage(colorImageInfo);
			colorImageView = colorImage->createView();

			const vk::ImageInfo depthImageInfo = { swapchain->getWidth(),swapchain->getHeight(),depthDescription.format,1,1,VK_IMAGE_TYPE_2D,VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT };
			depthImage = device->createImage(depthImageInfo);
			depthImageView = depthImage->createView();

			std::vector<VkImageView> attachment = { *colorImageView, *depthImageView };
			const vk::FrameBufferInfo frameBufferInfo = { swapchain->getWidth(),swapchain->getHeight(),*renderPass,attachment };
			frameBuffer = device->createFrameBuffer(frameBufferInfo);

			layout = device->createPipelineLayout({});

			const std::string vert =
				"#version 450\n"
				"layout(location=0)in vec3 position;\n"
				"void main(){\n"
				"	gl_Position = vec4(position,1.0);\n"
				"}";

			const std::string frag =
				"#version 450\n"
				"layout(location=0)out vec4 color;\n"
				"void main(){\n"
				"	color = vec4(1.0,0.0,0.0,1.0);\n"
				"}";

			std::vector<VkVertexInputBindingDescription> b = {
				{ 0,4 * 3,VK_VERTEX_INPUT_RATE_VERTEX }
			};
			std::vector<VkVertexInputAttributeDescription> a = {
				{ 0,0,VK_FORMAT_R32G32B32_SFLOAT,0 }
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
	};

}
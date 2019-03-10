#include "renderer.h"
#include "device.h"
#include "commandBuffer.h"

namespace vg
{

	struct RenderState
	{
		vk::RenderPass* renderPass = nullptr;

		std::vector<vk::FrameBuffer*> frameBuffers;
		std::vector<vk::CommandBuffer*> commandBuffers;

		vk::Semaphore* semaphore = nullptr;
		vk::Fence* fence = nullptr;

		vk::PipelineLayout* layout = nullptr;
		vk::Pipeline* pipeline = nullptr;

		void initPipeline(vk::Device* device)
		{
			layout = device->createPipelineLayout({});

			const std::string vert =
				"#version 450\n"
				"vec2 position[3] = {vec2(0.0,0.5),vec2(-0.5,-0.5),vec2(0.5,-0.5)};\n"
				"void main(){\n"
				"	gl_Position = vec4(position[gl_VertexIndex],0.0,1.0);\n"
				"}";

			const std::string frag =
				"#version 450\n"
				"layout(location=0)out vec4 color;\n"
				"void main(){\n"
				"	color = vec4(1.0,0.0,0.0,1.0);\n"
				"}";
			
			const vk::PipelineInfo info = { *renderPass,*layout,{{vert,VK_SHADER_STAGE_VERTEX_BIT,vk::ShaderType::GLSL},{frag,VK_SHADER_STAGE_FRAGMENT_BIT,vk::ShaderType::GLSL}} };

			//const vk::PipelineInfo info = { *renderPass,*layout,{{"D:/github/VG/assets/vert.spv",VK_SHADER_STAGE_VERTEX_BIT},{"D:/github/VG/assets/frag.spv",VK_SHADER_STAGE_FRAGMENT_BIT}} };
			pipeline = device->createPipeline(info);
		}
	};

	void Renderer::setup(HWND windowHandle)
	{
		device = new vk::Device();
		renderState = new RenderState();

		auto surface = device->createSurface(windowHandle);
		const vk::SwapchainInfo swapchainInfo = { surface };
		swapchain = device->createSwapchain(swapchainInfo);

		//create render pass
		VkAttachmentDescription colorDescription = {
			0,
			swapchain->getColorFormat(),
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		VkAttachmentReference colorAttachment = { 0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpass = {
			0,									//flag
			VK_PIPELINE_BIND_POINT_GRAPHICS,	//pipelineBindPoint
			0,									//inputAttachmentCount
			nullptr,							//pInputAttachments
			1,									//colorAttachmentCount
			&colorAttachment,					//pColorAttachments
			nullptr,							//pResolveAttachments
			nullptr,							//pDepthStencilAttachment
			0,									//preserveAttachmentCount
			nullptr								//pPreserveAttachments
		};

		const vk::RenderPassInfo renderPassInfo = { {colorDescription},{subpass}, {} };
		renderState->renderPass = device->createRenderPass(renderPassInfo);

		//create frame buffer
		for (auto& v : swapchain->getImageViews())
		{
			std::vector<VkImageView> attachment = {*v};
			const vk::FrameBufferInfo frameBufferInfo = { swapchain->getWidth(),swapchain->getHeight(),*renderState->renderPass,attachment };
			renderState->frameBuffers.push_back(device->createFrameBuffer(frameBufferInfo));

			renderState->commandBuffers.push_back(device->createCommandBuffer());
		}

		renderState->semaphore = device->createSemaphore();
		renderState->fence = device->createFence();

		renderState->initPipeline(device);
	}

	void Renderer::draw()
	{
		const auto extent = swapchain->getExtent();

		renderState->fence->wait();

		swapchain->acquireNextImage();
		const auto index = swapchain->getFrameIndex();

		auto& cmd = renderState->commandBuffers[index];
		cmd->begin();

		const std::vector<VkClearValue> clearValue = {
			{0.0f,0.0f,0.0f,1.0f}
		};
		const VkRect2D renderArea = { {},extent };
		cmd->beginRenderPass(*renderState->renderPass, *renderState->frameBuffers[index], renderArea, clearValue);

		cmd->setViewport(extent.width, extent.height);
		cmd->setScissor(0, 0, extent.width, extent.height);

		cmd->bindPipeline(*renderState->pipeline);
		cmd->draw(3, 1);

		cmd->endRenderPass();
		cmd->end();
		cmd->submit(swapchain->getAcquireSemaphore(),*renderState->semaphore, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,*renderState->fence);

		swapchain->present(*renderState->semaphore);
	}
}
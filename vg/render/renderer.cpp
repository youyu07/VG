#include "renderer.h"
#include "device.h"
#include "commandBuffer.h"
#include "geometryBuffer.h"
#include "state/renderState.h"

namespace vg
{
	class RenderImpl
	{
	public:
		vk::Device* device = nullptr;
		vk::Swapchain* swapchain = nullptr;
		std::vector<DeviceHandle> geometries;

		vk::RenderPass* renderPass = nullptr;
		std::vector<vk::FrameBuffer*> frameBuffers;
		std::vector<vk::CommandBuffer*> commandBuffers;

		ImguiRenderState* imguiState = nullptr;
		vk::Fence* renderCompleteFence = nullptr;
	
		RenderImpl(HWND windowHandle)
		{
			device = new vk::Device();
			auto surface = device->createSurface(windowHandle);
			const vk::SwapchainInfo swapchainInfo = { surface };
			swapchain = device->createSwapchain(swapchainInfo);

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
			renderPass = device->createRenderPass(renderPassInfo);

			//create frame buffer
			for (auto& v : swapchain->getImageViews())
			{
				std::vector<VkImageView> attachment = { *v };
				const vk::FrameBufferInfo frameBufferInfo = { swapchain->getWidth(),swapchain->getHeight(),*renderPass,attachment };
				frameBuffers.push_back(device->createFrameBuffer(frameBufferInfo));

				commandBuffers.push_back(device->createCommandBuffer());
			}

			imguiState = new ImguiRenderState(device, renderPass);
			renderCompleteFence = device->createFence();
		}

		void draw()
		{
			renderCompleteFence->wait();
			swapchain->acquireNextImage();
			const auto index = swapchain->getFrameIndex();
			const auto extent = swapchain->getExtent();

			auto& cmd = commandBuffers[index];
			cmd->begin();

			const std::vector<VkClearValue> clearValue = {
				{0.0f,0.0f,0.0f,1.0f}
			};

			const VkRect2D renderArea = { {},extent };
			cmd->beginRenderPass(*renderPass, *frameBuffers[index], renderArea, clearValue);

			cmd->setViewport(extent.width, extent.height);
			cmd->setScissor(0, 0, extent.width, extent.height);

			imguiState->draw(cmd);

			cmd->endRenderPass();
			cmd->end();
			cmd->submit(swapchain->getAcquireSemaphore(), *imguiState->semaphore, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, *renderCompleteFence);
			swapchain->present(*imguiState->semaphore);
		}

		void setupRenderMeshState()
		{
			
		}
	};

	void Renderer::setup(HWND windowHandle)
	{
		impl = new RenderImpl(windowHandle);
		impl->setupRenderMeshState();
		
	}

	void Renderer::draw()
	{
		impl->draw();
	}

	DeviceHandle Renderer::createGeometry(const GeometryBufferInfo& geometry)
	{
		auto result = GeometryBuffer::create(impl->device, geometry);
		impl->geometries.push_back(result);
		return result;
	}
}
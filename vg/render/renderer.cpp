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
		struct
		{
			vk::Image* image = nullptr;
			vk::ImageView* view = nullptr;
		}depth;
		std::vector<vk::FrameBuffer*> frameBuffers;
		std::vector<vk::CommandBuffer*> commandBuffers;
		
		vk::Semaphore* renderCompleteSemaphore = nullptr;
		vk::Fence* renderCompleteFence = nullptr;

		ImguiRenderState* imguiState = nullptr;
		GeometryRenderState* geometryState = nullptr;
	
		RenderImpl(HWND windowHandle)
		{
			device = new vk::Device();
			auto surface = device->createSurface(windowHandle);
			const vk::SwapchainInfo swapchainInfo = { surface };
			swapchain = device->createSwapchain(swapchainInfo);

			initRenderPassAndFrameBuffer();

			renderCompleteSemaphore = device->createSemaphore();
			renderCompleteFence = device->createFence();

			imguiState = new ImguiRenderState(device, renderPass);
			geometryState = new GeometryRenderState(device,renderPass);
		}

		void initRenderPassAndFrameBuffer()
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
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
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

			const auto extent = swapchain->getExtent();

			const vk::ImageInfo imageInfo = { extent.width,extent.height,depthDescription.format,1,1,VK_IMAGE_TYPE_2D,VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT };
			depth.image = device->createImage(imageInfo);
			depth.view = depth.image->createView();

			for (auto& view : swapchain->getImageViews())
			{
				std::vector<VkImageView> attachment = { *view, *depth.view };
				const vk::FrameBufferInfo frameBufferInfo = { extent.width,extent.height,*renderPass,attachment };
				frameBuffers.push_back(device->createFrameBuffer(frameBufferInfo));
				commandBuffers.push_back(device->createCommandBuffer());
			}
			
		}

		void draw()
		{
			renderCompleteFence->wait();
			swapchain->acquireNextImage();
			const auto index = swapchain->getFrameIndex();
			
			auto& cmd = commandBuffers[index];
			auto& frameBuffer = frameBuffers[index];
			cmd->begin();

			const std::vector<VkClearValue> clearValue = {
				{0.0f,0.0f,0.0f,0.0f},
				{0.0f,0}
			};
			const auto extent = swapchain->getExtent();
			const VkRect2D renderArea = { {},extent };
			cmd->beginRenderPass(*renderPass, *frameBuffer, renderArea, clearValue);


			cmd->setViewport(extent.width, extent.height,true);
			cmd->setScissor(0, 0, extent.width, extent.height);
			geometryState->draw<GeometryBuffer>(cmd,geometries);


			cmd->setViewport(extent.width, extent.height);
			imguiState->draw(cmd);


			cmd->endRenderPass();
			cmd->end();
			cmd->submit(swapchain->getAcquireSemaphore(), *renderCompleteSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,*renderCompleteFence);
			swapchain->present(*renderCompleteSemaphore);
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
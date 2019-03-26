#include "renderer.h"

#include <chrono>

#include "context.h"
#include <vku.hpp>
#include <core/log.h>

#include "state/renderState.h"

namespace vg
{
	class RendererImpl
	{
		Context context;

		vk::UniqueRenderPass renderPass;

		std::vector<vk::UniqueFramebuffer> frameBuffers;
		std::vector<vk::UniqueCommandBuffer> drawCommandBuffers;

		vk::UniqueSemaphore acquireSemaphore;
		vk::UniqueSemaphore drawCompleteSemaphore;
		vk::UniqueFence drawFence;
	public:
		RendererImpl(const void* windowHandle) {
			context = Context{ windowHandle };

			{
				vku::RenderpassMaker rm;
				rm.attachmentBegin(context.getSwapchainFormat());
				rm.attachmentLoadOp(vk::AttachmentLoadOp::eClear);
				rm.attachmentStoreOp(vk::AttachmentStoreOp::eStore);
				rm.attachmentFinalLayout(vk::ImageLayout::ePresentSrcKHR);

				rm.subpassBegin(vk::PipelineBindPoint::eGraphics);
				rm.subpassColorAttachment(vk::ImageLayout::eColorAttachmentOptimal, 0);
				renderPass = rm.createUnique(context.getDevice());
			}

			{
				for (uint32_t i = 0; i < context.getSwapchainImageCount(); i++)
				{
					auto frameInfo = vk::FramebufferCreateInfo({}, renderPass.get(), 1, &context.getSwapchainImageView(i), context.getExtent().width, context.getExtent().height, 1);
					frameBuffers.emplace_back(context.getDevice().createFramebufferUnique(frameInfo));
				}
			}

			drawCommandBuffers = context.getDevice().allocateCommandBuffersUnique({context.getCommandPool(),vk::CommandBufferLevel::ePrimary,static_cast<uint32_t>(frameBuffers.size()) });
		}

		void buildCommandBuffer(uint32_t index)
		{
			auto& cmd = drawCommandBuffers.at(index);

			cmd->begin({});

			std::array<float, 4> clearColorValue{ 0.75f, 0.75f, 0.75f, 1 };
			vk::ClearDepthStencilValue clearDepthValue{ 1.0f, 0 };
			std::array<vk::ClearValue, 2> clearColours{ vk::ClearValue{clearColorValue}, clearDepthValue };
			vk::RenderPassBeginInfo beginInfo;
			beginInfo.renderPass = renderPass.get();
			beginInfo.framebuffer = frameBuffers.at(index).get();
			beginInfo.renderArea = vk::Rect2D{ {0, 0}, context.getExtent() };
			beginInfo.clearValueCount = (uint32_t)clearColours.size();
			beginInfo.pClearValues = clearColours.data();
			cmd->beginRenderPass(beginInfo,vk::SubpassContents::eInline);

			cmd->endRenderPass();
			cmd->end();
		}

		void draw()
		{
			const auto& device = context.getDevice();

			uint32_t imageIndex = 0;
			auto result = device.acquireNextImageKHR(context.getSwapchain(), std::numeric_limits<uint64_t>::max(), acquireSemaphore.get(), vk::Fence());
			imageIndex = result.value;

			device.waitForFences(drawFence.get(),VK_TRUE, std::numeric_limits<uint64_t>::max());
			device.resetFences(drawFence.get());

			buildCommandBuffer(imageIndex);

			vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

			vk::SubmitInfo submit;
			submit.waitSemaphoreCount = 1;
			submit.pWaitSemaphores = &acquireSemaphore.get();
			submit.pWaitDstStageMask = &waitStages;
			submit.commandBufferCount = 1;
			submit.pCommandBuffers = &drawCommandBuffers.at(imageIndex).get();
			submit.signalSemaphoreCount = 1;
			submit.pSignalSemaphores = &drawCompleteSemaphore.get();
			context.getGraphicsQueue().submit(1, &submit, drawFence.get());

			vk::PresentInfoKHR presentInfo;
			presentInfo.pSwapchains = &context.getSwapchain();
			presentInfo.swapchainCount = 1;
			presentInfo.pImageIndices = &imageIndex;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &drawCompleteSemaphore.get();
			context.getGraphicsQueue().presentKHR(presentInfo);
		}
	};

	void Renderer::setup(const void* windowHandle)
	{
		impl = new RendererImpl(windowHandle);
		//impl->setupRenderMeshState();
		
	}

	void Renderer::draw()
	{
		impl->draw();
	}

	DeviceHandle Renderer::createGeometry(const GeometryBufferInfo& geometry)
	{
		//auto result = GeometryBuffer::create(impl->device, geometry);
		//impl->geometries.push_back(result);
		//return result;

		return nullptr;
	}
}
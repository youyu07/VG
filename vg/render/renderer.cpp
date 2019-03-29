#include "renderer.h"

#include <chrono>

#include "context.h"
#include <core/log.h>
#include <glm/ext.hpp>

#include "state/renderState.h"

#include "geometryBuffer.h"

namespace vg
{
	struct CameraMatrix
	{
		struct
		{
			glm::mat4 projection;
			glm::mat4 view;
		}data;

		vku::HostUniformBuffer buffer;
		vk::UniqueDescriptorSetLayout setLayout;
		vk::UniqueDescriptorSet set;

		CameraMatrix() {}

		CameraMatrix(const Context& ctx) {
			buffer = vku::HostUniformBuffer(ctx, ctx.getMemoryProperties(), sizeof(data));
			vku::DescriptorSetLayoutMaker dlm;
			dlm.buffer(0, vk::DescriptorType::eUniformBufferDynamic, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 1);
			setLayout = dlm.createUnique(ctx);

			vku::DescriptorSetMaker dm;
			dm.layout(setLayout.get());
			set = std::move(dm.createUnique(ctx, ctx.getDescriptorPool()).at(0));

			vku::DescriptorSetUpdater updater;
			updater.beginDescriptorSet(set.get());
			updater.beginBuffers(0, 0, vk::DescriptorType::eUniformBufferDynamic);
			updater.buffer(buffer.buffer(), 0, sizeof(data));
			updater.update(ctx);
		}

		void update(glm::mat4 p, glm::mat4 v) {
			data.projection = p;
			data.view = v;
		}

		void update(const Context& ctx) {
			buffer.updateLocal(ctx, data);
		}
	};


	class RendererImpl
	{
		Context ctx;
		vk::UniqueRenderPass renderPass;
		vku::DepthStencilImage depth;

		struct
		{
			vku::ColorAttachmentImage color;
			vku::DepthStencilImage depth;
		}multiSample;

		std::vector<vk::UniqueFramebuffer> frameBuffers;
		std::vector<vk::UniqueCommandBuffer> drawCommandBuffers;

		vk::UniqueSemaphore acquireSemaphore;
		vk::UniqueSemaphore drawCompleteSemaphore;
		vk::UniqueFence drawFence;

		ImguiRenderState imguiState;
		GridRenderState gridState;
		GeometryRenderState geometryState;
	public:
		CameraMatrix matrix;

		bool prepared = false;
	public:
		RendererImpl(const void* windowHandle) {
			ctx = Context{ windowHandle };

			auto colorFormat = Context::getSwapchainFormat();
			auto depthFormat = Context::getDepthFormat();
			{
				vku::RenderpassMaker rm;
				rm.attachmentBegin(colorFormat);
				rm.attachmentSamples(Context::getSample());
				rm.attachmentLoadOp(vk::AttachmentLoadOp::eClear);
				rm.attachmentStoreOp(vk::AttachmentStoreOp::eStore);
				rm.attachmentStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				rm.attachmentStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
				rm.attachmentFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

				rm.attachmentBegin(colorFormat);
				rm.attachmentLoadOp(vk::AttachmentLoadOp::eDontCare);
				rm.attachmentStoreOp(vk::AttachmentStoreOp::eStore);
				rm.attachmentStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				rm.attachmentStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
				rm.attachmentFinalLayout(vk::ImageLayout::ePresentSrcKHR);

				rm.attachmentBegin(depthFormat);
				rm.attachmentSamples(Context::getSample());
				rm.attachmentLoadOp(vk::AttachmentLoadOp::eClear);
				rm.attachmentStoreOp(vk::AttachmentStoreOp::eDontCare);
				rm.attachmentStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				rm.attachmentStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
				rm.attachmentFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

				rm.attachmentBegin(depthFormat);
				rm.attachmentLoadOp(vk::AttachmentLoadOp::eDontCare);
				rm.attachmentStoreOp(vk::AttachmentStoreOp::eStore);
				rm.attachmentStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				rm.attachmentStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
				rm.attachmentFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

				rm.subpassBegin(vk::PipelineBindPoint::eGraphics);
				rm.subpassColorAttachment(vk::ImageLayout::eColorAttachmentOptimal, 0);
				rm.subpassResolveAttachment(vk::ImageLayout::eColorAttachmentOptimal, 1);
				rm.subpassDepthStencilAttachment(vk::ImageLayout::eDepthStencilAttachmentOptimal, 2);
				renderPass = rm.createUnique(ctx.getDevice());
			}

			drawCommandBuffers = ctx.getDevice().allocateCommandBuffersUnique({ ctx.getCommandPool(),vk::CommandBufferLevel::ePrimary,ctx.getSwapchainImageCount() });

			matrix = CameraMatrix(ctx);

			imguiState = ImguiRenderState(ctx, renderPass.get());
			gridState = GridRenderState(ctx, renderPass.get(),matrix.setLayout.get());
			geometryState = GeometryRenderState(ctx, matrix.setLayout.get());

			acquireSemaphore = ctx.getDevice().createSemaphoreUnique({});
			drawCompleteSemaphore = ctx.getDevice().createSemaphoreUnique({});
			drawFence = ctx.getDevice().createFenceUnique({vk::FenceCreateFlagBits::eSignaled});
			setupFrameBuffer();

			prepared = true;
		}

		void setupFrameBuffer()
		{
			depth = vku::DepthStencilImage(ctx, ctx.getMemoryProperties(), ctx.getExtent().width, ctx.getExtent().height, Context::getDepthFormat());
			multiSample.color = vku::ColorAttachmentImage(ctx, ctx.getMemoryProperties(), ctx.getExtent().width, ctx.getExtent().height, Context::getSwapchainFormat(), Context::getSample());
			multiSample.depth = vku::DepthStencilImage(ctx, ctx.getMemoryProperties(), ctx.getExtent().width, ctx.getExtent().height, Context::getDepthFormat(), Context::getSample());

			for (uint32_t i = 0; i < ctx.getSwapchainImageCount(); i++)
			{
				vk::ImageView attachment[4] = { multiSample.color.imageView(), ctx.getSwapchainImageView(i),multiSample.depth.imageView(), depth.imageView() };
				auto frameInfo = vk::FramebufferCreateInfo({}, renderPass.get(), 4, attachment, ctx.getExtent().width, ctx.getExtent().height, 1);
				frameBuffers.emplace_back(ctx.getDevice().createFramebufferUnique(frameInfo));
			}
		}

		void resize(bool force = false) {
			if (!force && !prepared) {
				return;
			}
			prepared = false;
			ctx.getDevice().waitIdle();

			if (!ctx.createSwapchain()) {
				return;
			}
			frameBuffers.swap(std::vector<vk::UniqueFramebuffer>());
			setupFrameBuffer();

			prepared = true;
		}

		void buildCommandBuffer(uint32_t index)
		{
			matrix.update(ctx);

			auto& cmd = drawCommandBuffers.at(index).get();

			cmd.begin(vk::CommandBufferBeginInfo());

			std::array<float, 4> clearColorValue{ 0.2f, 0.2f, 0.2f, 1 };
			vk::ClearDepthStencilValue clearDepthValue{ 1.0f, 0 };
			std::array<vk::ClearValue, 3> clearColours{ vk::ClearValue{clearColorValue},vk::ClearValue{clearColorValue}, clearDepthValue };
			vk::RenderPassBeginInfo beginInfo;
			beginInfo.renderPass = renderPass.get();
			beginInfo.framebuffer = frameBuffers.at(index).get();
			beginInfo.renderArea = vk::Rect2D{ {0, 0}, ctx.getExtent() };
			beginInfo.clearValueCount = (uint32_t)clearColours.size();
			beginInfo.pClearValues = clearColours.data();
			cmd.beginRenderPass(beginInfo,vk::SubpassContents::eInline);

			vk::Viewport viewport = {0.0f,static_cast<float>(ctx.getExtent().height),static_cast<float>(ctx.getExtent().width),-static_cast<float>(ctx.getExtent().height),0.0f,1.0f };
			cmd.setViewport(0, viewport);
			vk::Rect2D scissor = { {},ctx.getExtent() };
			cmd.setScissor(0, scissor);

			gridState.draw(ctx, cmd, matrix.set.get());
			geometryState.draw(ctx, cmd, renderPass.get(), matrix.set.get());

			viewport = { 0.0f,0.0f,static_cast<float>(ctx.getExtent().width),static_cast<float>(ctx.getExtent().height),0.0f,1.0f };
			cmd.setViewport(0, viewport);
			imguiState.draw(ctx, cmd);

			cmd.endRenderPass();
			cmd.end();

			
		}

		void draw()
		{
			const auto& device = ctx.getDevice();

			device.waitForFences(drawFence.get(), VK_TRUE, std::numeric_limits<uint64_t>::max());
			device.resetFences(drawFence.get());

			uint32_t imageIndex = 0;
			vk::Result result;
			do {
				result = device.acquireNextImageKHR(ctx.getSwapchain(), std::numeric_limits<uint64_t>::max(), acquireSemaphore.get(), vk::Fence(), &imageIndex);
				if (result == vk::Result::eErrorOutOfDateKHR) {
					// demo->swapchain is out of date (e.g. the window was resized) and
					// must be recreated:
					resize();
				}
				else if (result == vk::Result::eSuboptimalKHR) {
					// swapchain is not as optimal as it could be, but the platform's
					// presentation engine will still present the image correctly.
					break;
				}
				else {
					assert(result == vk::Result::eSuccess);
				}
			} while (result != vk::Result::eSuccess);

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
			ctx.getGraphicsQueue().submit(1, &submit, drawFence.get());

			vk::PresentInfoKHR presentInfo;
			presentInfo.pSwapchains = &ctx.getSwapchain();
			presentInfo.swapchainCount = 1;
			presentInfo.pImageIndices = &imageIndex;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &drawCompleteSemaphore.get();
			result = ctx.getGraphicsQueue().presentKHR(&presentInfo);

			if (result == vk::Result::eErrorOutOfDateKHR) {
				// swapchain is out of date (e.g. the window was resized) and
				// must be recreated:
				resize();
			}
			else if (result == vk::Result::eSuboptimalKHR) {
				// swapchain is not as optimal as it could be, but the platform's
				// presentation engine will still present the image correctly.
			}
			else {
				assert(result == vk::Result::eSuccess);
			}
		}

		float getAspect()
		{
			if (ctx.getExtent().height == 0) {
				return 1.0f;
			}
			else {
				return static_cast<float>(ctx.getExtent().width) / static_cast<float>(ctx.getExtent().height);
			}
		}

		void addGeometry(uint64_t id, const GeometryBufferInfo& info)
		{
			geometryState.addGeometry(ctx, id, info);
		}
	};

	void Renderer::setup(const void* windowHandle)
	{
		impl = new RendererImpl(windowHandle);
		//impl->setupRenderMeshState();
		
	}

	void Renderer::draw()
	{
		if (impl->prepared) {
			impl->draw();
		}
	}

	void Renderer::resize()
	{
		impl->resize(true);
	}

	void Renderer::bindCamera(const Camera& camera)
	{
		impl->matrix.update(camera.getProjectionMatrix(impl->getAspect()), camera.getViewMatrix());
	}

	void Renderer::addGeometry(uint64_t id, const GeometryBufferInfo& info)
	{
		impl->addGeometry(id, info);
	}
}
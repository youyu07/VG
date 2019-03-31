#include "renderer.h"

#include <chrono>

#include "context.h"
#include <core/log.h>
#include <glm/ext.hpp>

//#include "state/renderState.h"

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

		vk::Fence renderFence;
		vk::Semaphore acquireSemaphore;
		vk::Semaphore drawSemaphore;

		//ImguiRenderState imguiState;
		//GridRenderState gridState;
		//GeometryRenderState geometryState;
	public:
		CameraMatrix matrix;

		bool prepared = false;
	public:
		RendererImpl(const void* windowHandle) {
			ctx = createContext(windowHandle);

			
			matrix = CameraMatrix(ctx);

			//imguiState = ImguiRenderState(ctx, renderPass.get());
			//gridState = GridRenderState(ctx, renderPass.get(),matrix.setLayout.get());
			//geometryState = GeometryRenderState(ctx, matrix.setLayout.get());

			

			prepared = true;
		}

		void resize(bool force = false) {
			if (!force && !prepared) {
				return;
			}
			prepared = false;

			ctx->getDevice()->waitIdle();

			if (ctx->resize()) {
				prepared = true;
			}
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
			ctx->getDevice()->waitForFences(VkFence(*renderFence));

			uint32_t imageIndex = 0;
			VkResult result;
			do {
				result = ctx->getDevice()->acquireNextImage(*ctx->getSwapchain(), &imageIndex, *acquireSemaphore);
				if (result == VK_ERROR_OUT_OF_DATE_KHR) {
					resize();
				}
				else if (result == VK_SUBOPTIMAL_KHR) {
					// swapchain is not as optimal as it could be, but the platform's
					// presentation engine will still present the image correctly.
					break;
				}
				else {
					VK_CHECK_RESULT(result);
				}
			} while (result != VK_SUCCESS);

			buildCommandBuffer(imageIndex);
			
			result = ctx->getGraphicsQueue()->present(ctx->getSwapchain()->get(), &imageIndex, drawSemaphore->get());
			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				resize();
			}
			else if (result == VK_SUBOPTIMAL_KHR) {
				// swapchain is not as optimal as it could be, but the platform's
				// presentation engine will still present the image correctly.
			}
			else {
				VK_CHECK_RESULT(result);
			}
		}

		float getAspect()
		{
			if (ctx->getExtent().height == 0) {
				return 1.0f;
			}
			else {
				return static_cast<float>(ctx->getExtent().width) / static_cast<float>(ctx->getExtent().height);
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
#include "renderer.h"

#include <chrono>

#include "context.h"
#include <core/log.h>
#include <glm/ext.hpp>

#include "state/renderState.h"

//#include "geometryBuffer.h"

namespace vg
{
	struct CameraMatrix
	{
		struct
		{
			glm::mat4 projection;
			glm::mat4 view;
		}data;

		vk::Buffer buffer;
		vk::DescriptorSetLayout setLayout;

		vk::DescriptorSet set;

		CameraMatrix() {}

		CameraMatrix(const Context& ctx) {
			buffer = ctx->getDevice()->createUniformBuffer(sizeof(data), true);
			vk::DescriptorSetLayoutMaker dlm;
			dlm.binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
			setLayout = dlm.create(ctx->getDevice());

			//vku::DescriptorSetMaker dm;
			//dm.layout(setLayout.get());
			//set = std::move(dm.createUnique(ctx, ctx.getDescriptorPool()).at(0));

			//vku::DescriptorSetUpdater updater;
			//updater.beginDescriptorSet(set.get());
			//updater.beginBuffers(0, 0, vk::DescriptorType::eUniformBufferDynamic);
			//updater.buffer(buffer.buffer(), 0, sizeof(data));
			//updater.update(ctx);
		}

		void update(glm::mat4 p, glm::mat4 v) {
			data.projection = p;
			data.view = v;
		}

		void update() {
			buffer->uploadLocal(data);
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
		//CameraMatrix matrix;

		bool prepared = false;
	public:
		RendererImpl(const void* windowHandle) {
			ctx = createContext(windowHandle);

			
			//matrix = CameraMatrix(ctx);

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
			//matrix.update();

			auto& cmd = ctx->getCommandBuffer(index);
			cmd->begin();
			auto extent = ctx->getExtent();

			VkRect2D area = { {},extent };
			std::array<VkClearValue, 3> clearValue = { VkClearColorValue{0.0f},VkClearColorValue{0.0f},{1.0f,0} };
			cmd->beginRenderPass(ctx->getRenderPass(), area, clearValue);

			/*
			vk::Viewport viewport = {0.0f,static_cast<float>(ctx.getExtent().height),static_cast<float>(ctx.getExtent().width),-static_cast<float>(ctx.getExtent().height),0.0f,1.0f };
			cmd.setViewport(0, viewport);
			vk::Rect2D scissor = { {},ctx.getExtent() };
			cmd.setScissor(0, scissor);

			gridState.draw(ctx, cmd, matrix.set.get());
			geometryState.draw(ctx, cmd, renderPass.get(), matrix.set.get());

			viewport = { 0.0f,0.0f,static_cast<float>(ctx.getExtent().width),static_cast<float>(ctx.getExtent().height),0.0f,1.0f };
			cmd.setViewport(0, viewport);
			imguiState.draw(ctx, cmd);
			*/

			cmd->endRenderPass();
			cmd->end();
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
			//geometryState.addGeometry(ctx, id, info);
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
		//impl->matrix.update(camera.getProjectionMatrix(impl->getAspect()), camera.getViewMatrix());
	}

	void Renderer::addGeometry(uint64_t id, const GeometryBufferInfo& info)
	{
		impl->addGeometry(id, info);
	}
}
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

		vk::Buffer buffer;
		vk::DescriptorSetLayout setLayout;

		vk::DescriptorSet set;

		CameraMatrix() {}

		CameraMatrix(const Context& ctx) {
			buffer = ctx->getDevice()->createUniformBuffer(sizeof(data), true);
			vk::DescriptorSetLayoutMaker dlm;
			dlm.binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
			setLayout = dlm.create(ctx->getDevice());

			set = ctx->getDescriptorPool()->createDescriptorSet(setLayout->get());
			auto updater = vk::DescriptorSetUpdater();
			updater.beginDescriptorSet(set);
			updater.beginBuffers(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
			updater.buffer(buffer, 0, sizeof(data));
			updater.update(ctx->getDevice());
		}

		void update(glm::mat4 p, glm::mat4 v) {
			data.projection = p;
			data.projection[1][1] *= -1;
			data.view = v;
		}

		void update() {
			buffer->uploadLocal(&data);
		}
	};


	class RendererImpl
	{
		Context ctx;

		vk::Fence drawFence;
		vk::Semaphore acquireSemaphore;
		vk::Semaphore drawSemaphore;

		struct
		{
			ImguiRenderState imgui;
			GridRenderState grid;
			GeometryRenderState geometry;
			PickRenderState pick;
		}stat;
		
		GeometryManager geometries;
	public:
		CameraMatrix matrix;

		bool prepared = false;
	public:
		RendererImpl(const void* windowHandle) {
			ctx = createContext(windowHandle);
			drawFence = ctx->getDevice()->createFence();
			acquireSemaphore = ctx->getDevice()->createSemaphore();
			drawSemaphore = ctx->getDevice()->createSemaphore();
			
			matrix = CameraMatrix(ctx);

			stat.imgui = ImguiRenderState(ctx);
			stat.grid = GridRenderState(ctx,matrix.setLayout);
			stat.geometry = GeometryRenderState(ctx, matrix.setLayout);
			stat.pick = PickRenderState(ctx, matrix.setLayout);

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

		void select(glm::uvec2 point) {
			auto sel = stat.pick.select(ctx, matrix.set, geometries,point);
			stat.geometry.setSelect(sel);
		}

		void buildCommandBuffer(uint32_t index)
		{
			matrix.update();

			auto& cmd = ctx->getCommandBuffer(index);
			cmd->begin();
			auto extent = ctx->getExtent();

			VkRect2D area = { {},extent };
			std::array<VkClearValue, 3> clearValue = { VkClearColorValue{0.0f},VkClearColorValue{0.0f},{1.0f,0} };
			cmd->beginRenderPass(ctx->getRenderPass(),ctx->getFrameBuffer(index), area, clearValue);

			cmd->viewport(0, 0, extent.width, extent.height);
			cmd->scissor(0, 0, extent.width, extent.height);

			stat.grid.draw(ctx, cmd, matrix.set);

			stat.geometry.draw(ctx, cmd, matrix.set,geometries);

			cmd->viewport(0, 0, extent.width, extent.height);
			stat.imgui.draw(ctx, cmd);
			

			cmd->endRenderPass();
			cmd->end();

			//stat.pick.select(ctx, matrix.set, geometries, glm::uvec2());
		}

		void draw()
		{
			ctx->getDevice()->waitForFences(drawFence->get());

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
			
			auto& cmd = ctx->getCommandBuffer(imageIndex);

			ctx->getGraphicsQueue()->submit(cmd->get(), acquireSemaphore->get(), drawSemaphore->get(), drawFence->get(),VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

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
			auto extent = ctx->getExtent();
			if (extent.height == 0) {
				return 1.0f;
			}
			else {
				return static_cast<float>(extent.width) / static_cast<float>(extent.height);
			}
		}

		void addGeometry(uint32_t id, const GeometryBufferInfo& info)
		{
			geometries.addGeometry(ctx, id, info);
		}
	};

	void Renderer::setup(const void* windowHandle)
	{
		impl = new RendererImpl(windowHandle);
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

	void Renderer::addGeometry(uint32_t id, const GeometryBufferInfo& info)
	{
		impl->addGeometry(id, info);
	}

	void Renderer::click(glm::uvec2 point)
	{
		impl->select(point);
	}
}
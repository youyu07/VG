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
		
		vk::Fence* renderCompleteFence = nullptr;

		ImguiRenderState* imguiState = nullptr;
		GeometryRenderState* geometryState = nullptr;
	
		RenderImpl(HWND windowHandle)
		{
			device = new vk::Device();
			auto surface = device->createSurface(windowHandle);
			const vk::SwapchainInfo swapchainInfo = { surface };
			swapchain = device->createSwapchain(swapchainInfo);

			renderCompleteFence = device->createFence();


			imguiState = new ImguiRenderState(device, swapchain);
			geometryState = new GeometryRenderState(device);
			geometryState->setup(swapchain->getExtent());
		}

		void draw()
		{
			renderCompleteFence->wait();
			swapchain->acquireNextImage();
			const auto index = swapchain->getFrameIndex();
			
			auto& gs_cmd = geometryState->draw<GeometryBuffer>(geometries);
			auto& gui_cmd = imguiState->draw(index);
			

			gs_cmd->submit(swapchain->getAcquireSemaphore(), *geometryState->semaphore, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
			gui_cmd->submit(*geometryState->semaphore, *imguiState->semaphore, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, *renderCompleteFence);
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
#pragma once


namespace vg
{
	class RenderState
	{
	protected:
		vk::Device* device = nullptr;
	public:
		vk::Semaphore* semaphore = nullptr;

		RenderState(vk::Device* device) : device(device)
		{
			semaphore = device->createSemaphore();
		}
	};
}

#include "geometryRenderState.h"
#include "imguiRenderState.h"
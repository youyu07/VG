#pragma once


namespace vg
{
	class RenderState
	{
	protected:
		vk::Device* device = nullptr;
	public:
		RenderState(vk::Device* device) : device(device)
		{
			
		}
	};
}

#include "geometryRenderState.h"
#include "imguiRenderState.h"
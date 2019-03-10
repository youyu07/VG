#pragma once
#if defined(WIN32)
#include <Windows.h>
#endif

namespace vg
{

	namespace vk
	{
		class Device;
		class Swapchain;
	}

	class __declspec(dllexport) Renderer
	{
	public:
		void setup(HWND windowHandle);

		void draw();
	private:
		vk::Device* device = nullptr;
		vk::Swapchain* swapchain = nullptr;
		struct RenderState* renderState = nullptr;
	};

}
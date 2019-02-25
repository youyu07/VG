#pragma once
#include "vg.h"

#if defined(WIN32)
#include <Windows.h>
#endif

namespace vg
{

	struct WindowInfo
	{
		uint32_t width = 800;
		uint32_t height = 600;
		const char* title = "vg";
#if defined(WIN32)
		HWND hWnd;
#endif
	};


    class __declspec(dllexport) Entry
    {
    public:
		virtual void init() = 0;
		virtual void update() = 0;
		virtual void draw() = 0;

		void start();

		void setWindowSize(uint32_t width, uint32_t height)
		{
			windowInfo.width = width;
			windowInfo.height = height;
		}

		void setWindowTitle(const char* title)
		{
			windowInfo.title = title;
		}

    private:
        IDevice* device = nullptr;
		WindowInfo windowInfo;

		inline bool setupContext()
		{
			device = IDevice::create();
			return true;
		}
    };

}
#pragma once

#include <iostream>

#if defined(WIN32)
#include <Windows.h>
#endif

namespace vg
{

	struct WindowInfo
	{
		uint32_t width = 1280;
		uint32_t height = 960;
		const char* title = "vg";
#if defined(WIN32)
		HWND handle;
#endif
	};


    class __declspec(dllexport) Entry
    {
    public:
		virtual void init() = 0;
		virtual void update() = 0;
		virtual void draw() = 0;

		void start();

		inline void setWindowSize(uint32_t width, uint32_t height)
		{
			windowInfo.width = width;
			windowInfo.height = height;
		}

		inline void setWindowTitle(const char* title)
		{
			windowInfo.title = title;
		}

		inline const WindowInfo& getInfo() const
		{
			return windowInfo;
		}

		virtual void mouseWheel(float x,float y) {}
    private:
		WindowInfo windowInfo;
    };

}
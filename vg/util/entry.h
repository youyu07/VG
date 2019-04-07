#pragma once

#include <iostream>

#if defined(WIN32)
#include <Windows.h>
#endif

#include "key.h"

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
		struct MouseEvent
		{
			enum class Type
			{
				LeftDown,
				LeftUp,
				RightDown,
				RightUp,
				MiddleDown,
				MiddleUp,
				Wheel,
				Move
			}type;
			float x, y;
		};

		struct WindowEvent
		{
			enum class Type
			{
				Minimized,
				Maximized,
				Restored,
				MaxShow,
				MaxHide
			}type;
			int width, height;
		};

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

		virtual void mouseEvent(const MouseEvent& event) {}
		virtual void windowEvent(const WindowEvent& event) {}

		void setKeyState(Key key,bool isDown) {
			keyDown[static_cast<uint32_t>(key)] = isDown;
		}

		bool getKeyState(Key key) {
			return keyDown[static_cast<uint32_t>(key)];
		}
    private:
		WindowInfo windowInfo;
		bool keyDown[512] = {};
    };

}
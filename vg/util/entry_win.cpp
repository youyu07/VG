#include "entry.h"
#include <core/log.h>

#if defined(WIN32)

namespace vg
{
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	}


	void Entry::start()
	{
		if (!setupContext())
		{
			log_error("Faild to setup graphics context!");
			return;
		}

		LPCSTR clsName = "vg";
		WNDCLASS wc = {};
		wc.style = CS_HREDRAW | CS_VREDRAW; // 设置style: 当窗口改变大小时就重新绘制窗口
		wc.lpfnWndProc = (WNDPROC)WindowProc; // 设定Window Procedure
		wc.cbClsExtra = 0; // 用来储存Class Structure后的额外的数据，这里不需要
		wc.cbWndExtra = 0; // 用来储存Window Instance后的额外的数据，这里不需要
		wc.hInstance = GetModuleHandle(NULL); // Window Procedure所在的Instance
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); // class的图标
		wc.hCursor = LoadCursor(NULL, IDC_ARROW); // class的光标
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // 背景刷
		wc.lpszClassName = clsName; // 应用程序的名字

		::RegisterClass(&wc);

		// 注册Window Class后，WinMain就调用CreateWindow函数来创建应用程序的Window
		windowInfo.hWnd = ::CreateWindow(clsName,
			windowInfo.title,
			WS_OVERLAPPEDWINDOW, // Window风格
			CW_USEDEFAULT, // Window起点的X坐标
			CW_USEDEFAULT, // Window起点的Y坐标
			windowInfo.width, // Window的宽度
			windowInfo.height, // Window的高度
			HWND_DESKTOP, // 父窗口的handle
			NULL, // 菜单的handle
			wc.hInstance, // 应用程序instance的handle
			NULL // window-creation数据的指针
		);

		// 以下两条语句用来显示Window
		ShowWindow(windowInfo.hWnd, SW_SHOWDEFAULT);
		UpdateWindow(windowInfo.hWnd);

		init();

		MSG msg = {};
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				continue;
			}

			update();
			draw();
		}

		DestroyWindow(windowInfo.hWnd);
		UnregisterClass("vg", wc.hInstance);
	}
}


#endif
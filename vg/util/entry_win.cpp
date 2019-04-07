#include "entry.h"
#include <core/log.h>

#if defined(WIN32)

#include <imgui/imgui_win32.h>

namespace vg
{
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		ImGui::win32_WndProcHandler(hwnd, message, wParam, lParam);

		auto entry = (Entry*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

		switch (message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_MOUSEWHEEL:
			entry->mouseEvent({ Entry::MouseEvent::Type::Wheel, 0, (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA });
			break;
		case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
		{
			auto type = Entry::MouseEvent::Type::LeftDown;
			if (message == WM_RBUTTONDOWN || message == WM_RBUTTONDBLCLK) { type = Entry::MouseEvent::Type::RightDown; }
			if (message == WM_MBUTTONDOWN || message == WM_MBUTTONDBLCLK) { type = Entry::MouseEvent::Type::MiddleDown; }
			entry->mouseEvent({ type,(float)LOWORD(lParam), (float)HIWORD(lParam) });
			break;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			auto type = Entry::MouseEvent::Type::LeftUp;
			if (message == WM_RBUTTONUP) { type = Entry::MouseEvent::Type::RightUp; }
			if (message == WM_MBUTTONUP) { type = Entry::MouseEvent::Type::MiddleUp; }
			entry->mouseEvent({ type,(float)LOWORD(lParam), (float)HIWORD(lParam) });
			break;
		}
		case WM_MOUSEMOVE:
			entry->mouseEvent({ Entry::MouseEvent::Type::Move,(float)LOWORD(lParam), (float)HIWORD(lParam) });
			break;
		case WM_SIZE:
			if (entry) {
				int width = LOWORD(lParam);
				int height = HIWORD(lParam);
				switch (wParam)
				{
				case SIZE_MINIMIZED:entry->windowEvent({ Entry::WindowEvent::Type::Minimized,width,height }); break;
				case SIZE_MAXIMIZED:entry->windowEvent({ Entry::WindowEvent::Type::Maximized,width,height }); break;
				case SIZE_RESTORED:entry->windowEvent({ Entry::WindowEvent::Type::Restored,width,height }); break;
				case SIZE_MAXSHOW:entry->windowEvent({ Entry::WindowEvent::Type::MaxShow,width,height }); break;
				case SIZE_MAXHIDE:entry->windowEvent({ Entry::WindowEvent::Type::MaxHide,width,height }); break;
				}
			}
			break;
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			entry->setKeyState(static_cast<Key>(wParam), true);
			break;
		case WM_SYSKEYUP:
		case WM_KEYUP:
			entry->setKeyState(static_cast<Key>(wParam), false);
			break;
		case WM_CHAR:
			break;
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	}


	void Entry::start()
	{
		LPCSTR clsName = "vg";
		WNDCLASS wc = {};
		wc.style = CS_HREDRAW | CS_VREDRAW; // ����style: �����ڸı��Сʱ�����»��ƴ���
		wc.lpfnWndProc = (WNDPROC)WindowProc; // �趨Window Procedure
		wc.cbClsExtra = 0; // ��������Class Structure��Ķ�������ݣ����ﲻ��Ҫ
		wc.cbWndExtra = 0; // ��������Window Instance��Ķ�������ݣ����ﲻ��Ҫ
		wc.hInstance = GetModuleHandle(NULL); // Window Procedure���ڵ�Instance
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); // class��ͼ��
		wc.hCursor = LoadCursor(NULL, IDC_ARROW); // class�Ĺ��
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // ����ˢ
		wc.lpszClassName = clsName; // Ӧ�ó��������

		::RegisterClass(&wc);

		// ע��Window Class��WinMain�͵���CreateWindow����������Ӧ�ó����Window
		windowInfo.handle = ::CreateWindow(clsName,
			windowInfo.title,
			WS_OVERLAPPEDWINDOW, // Window���
			CW_USEDEFAULT, // Window����X����
			CW_USEDEFAULT, // Window����Y����
			windowInfo.width, // Window�Ŀ��
			windowInfo.height, // Window�ĸ߶�
			HWND_DESKTOP, // �����ڵ�handle
			NULL, // �˵���handle
			wc.hInstance, // Ӧ�ó���instance��handle
			NULL // window-creation���ݵ�ָ��
		);

		// �����������������ʾWindow
		::ShowWindow(windowInfo.handle, SW_SHOWDEFAULT);
		::UpdateWindow(windowInfo.handle);
		::SetWindowLongPtr(windowInfo.handle, GWLP_USERDATA, (LONG_PTR)this);
		ImGui::win32_Init(windowInfo.handle);

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
			ImGui::win32_NewFrame();

			update();

			draw();
		}

		ImGui::win32_Shutdown();

		DestroyWindow(windowInfo.handle);
		UnregisterClass("vg", wc.hInstance);
	}
}


#endif
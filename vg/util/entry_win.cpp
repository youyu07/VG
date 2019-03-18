#include "entry.h"
#include <core/log.h>

#if defined(WIN32)

#include <imgui/imgui_win32.h>

namespace vg
{
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		ImGui::win32_WndProcHandler(hwnd, message, wParam, lParam);

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
		ShowWindow(windowInfo.handle, SW_SHOWDEFAULT);
		UpdateWindow(windowInfo.handle);

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
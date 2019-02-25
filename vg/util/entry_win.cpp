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
		windowInfo.hWnd = ::CreateWindow(clsName,
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
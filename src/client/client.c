#include <stdio.h>
#include <client.h>
#include <winsock2.h> /* sockets */
#include <ws2tcpip.h> /* networking */
#include <debug.h> /* logging */
#include <stdbool.h> /* bool */
#include <process.h> /* _beginthread */
#include <windows.h> /* WIN sockets */

#include <socket_utils.h> /* create * socket */
#include <mainwindow.h>

/* Default address and port of the server */
#define SERVER_ADDR "192.168.0.184"
#define SERVER_PORT "8899"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef UNICODE
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, LPWSTR lpCmdLine, int nCmdShow)
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, LPSTR lpCmdLine, int nCmdShow)
#endif
{
	/* TODO: Init winsock */
	HWND hwnd; /* Handle to main window */

	if (!INIT_DEBUG())
		return 1;

	if (!register_mw_class(hInstance, WindowProc)) {
                DBG_FATAL(TEXT("Failed to register window class\n"));
		return 1;
	}

	hwnd = create_main_window(hInstance, "RadikChat");

	if (hwnd == NULL) {
                DBG_FATAL(TEXT("Failed to create window (%d)\n"), GetLastError());
		return 1;
	}

	ShowWindow(hwnd, nCmdShow);

	/* Handle messages from the system */
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint.

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

            EndPaint(hwnd, &ps);
        }
        return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

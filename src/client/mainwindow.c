#include <mainwindow.h>
#include <windows.h>
#include <commdlg.h>

#define MW_CLASS_NAME TEXT("MainWindow")

bool register_mw_class(HINSTANCE hInstance, LRESULT (CALLBACK *lpfnWndProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam))
{
        WNDCLASS wc = { 0 }; /* Window class structure members must be zeroed */

	wc.lpfnWndProc = lpfnWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = MW_CLASS_NAME;

	if (!RegisterClass(&wc))
		return false;
	return true;
}


HWND create_main_window(HINSTANCE hInstance, TCHAR *window_name)
{
        return CreateWindow(
			MW_CLASS_NAME,
			window_name,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, /* Initial position of the window */
			CW_USEDEFAULT, CW_USEDEFAULT, /* Initial size of the window */
			NULL,
			NULL,
			hInstance,
			NULL
	);
}

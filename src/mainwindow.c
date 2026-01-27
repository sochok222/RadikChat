#include <mainwindow.h>


HWND create_main_window(char *window_name, char *class_name, HINSTANCE hInstance)
{
	return CreateWindowA(
			class_name,
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

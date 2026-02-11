#include <windows.h> /* win32 */
#include <stdbool.h>

/* Create main window and return it handle.	*
 * Function requires handle to a instance of	*
 * current application				*
 * And pointer to window procedure		*/
bool register_mw_class(HINSTANCE hInstance, LRESULT (CALLBACK *lpfnWndProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam));

/* Create main window and return it handle.	*
 * Function requires handle to a instance of	*
 * current application 				*/
HWND create_main_window(HINSTANCE hInstance, TCHAR *window_name);

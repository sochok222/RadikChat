#include <Debug.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <windows.h>

/************************************************ 
 * This translation unit created for colorful	*
 * debugging					                *
 ************************************************/

#define PRIVATE static
#define PUBLIC

#define DEBUG_TO_CONSOLE /* If defined debug messages would be written to console */
#define DEBUG_OUTPUT_FILENAME "logs.txt" /* Filename for debug output if 
					    DEBUG_TO_CONSOLE is not defined */

PRIVATE	HANDLE console_handler;
PRIVATE CONSOLE_SCREEN_BUFFER_INFO console_info;

bool initDebug(void)
{
#ifdef DEBUG_TO_CONSOLE
	AllocConsole();
	freopen("CONIN$", "rb", stdin);
	freopen("CONOUT$", "wb", stdout);
	freopen("CONOUT$", "wb", stderr);

	if (stdin == NULL || stdout == NULL || stderr == NULL)
		return false;

	console_handler = GetStdHandle(STD_OUTPUT_HANDLE);
	return true;
#elif
	FILE *f_dbg;
	if ((f_dbg = fopen(DEBUG_OUTPUT_FILENAME, "w")) = NULL)
		return false;

	stdin = f_dbg;
	stdout = f_dbg;
	stderr = f_dbg;
	return true;
#endif
}

/* Log wsa error code converted to string with colorful output */
void logWsaError(int error_code)
{
	wchar_t *s = NULL; /* Pointer to string with error message */

#ifdef DEBUG_TO_CONSOLE
	GetConsoleScreenBufferInfo(console_handler, &console_info); /* Save console state */
#endif

	/* Get error text description from error code */
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, 
		error_code, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		(LPWSTR)&s, 
		0, 
		NULL
		); 

	if (s != NULL) { 
		fprintf(stderr, "Error message: "); 

#ifdef DEBUG_TO_CONSOLE
		SetConsoleTextAttribute(console_handler, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif

		fprintf(stderr, "%S", s); 

#ifdef DEBUG_TO_CONSOLE
		SetConsoleTextAttribute(console_handler, console_info.wAttributes); /* Restore console text colors */
#endif

		if (LocalFree(s) != NULL) {
                        DBG_ERROR("LocalFree() failed (%d)\n", GetLastError());
		} 
	} 
	else { 
                DBG_ERROR("Can't find error message for error code (%d)\n", error_code);
	} 
}

/* Log message with color corresponding to level */
void logMessage(int mode, const TCHAR *format, ...)
{
	int i;
	va_list args;
	FILE *out;

#ifdef DEBUG_TO_CONSOLE
	GetConsoleScreenBufferInfo(console_handler, &console_info); /* Save console state */
#endif

#ifdef DEBUG_TO_CONSOLE
	switch (mode) {
		case DBG_MODE_FATAL:
			SetConsoleTextAttribute(console_handler, FOREGROUND_RED);
			out = stderr;
			break;
		case DBG_MODE_ERROR:
			SetConsoleTextAttribute(console_handler, FOREGROUND_RED);
			out = stderr;
			break;
		case DBG_MODE_WARNING:
			SetConsoleTextAttribute(console_handler, FOREGROUND_RED);
			out = stdout;
			break;
		case DBG_MODE_INFO:
			SetConsoleTextAttribute(console_handler, FOREGROUND_GREEN);
			out = stdout;
			break;
		default:
			out = stdout;
			break;
	}
#endif
	/* Printing args */
	va_start(args, format);
        fprintf(out, format, args);
	va_end(args);

#ifdef DEBUG_TO_CONSOLE
	SetConsoleTextAttribute(console_handler, console_info.wAttributes); /* Restore console text colors */
#endif
}

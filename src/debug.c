#include <debug.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

/************************************************ 
 * This module created for colorful debugging	*
 ************************************************/

#define PRIVATE static
#define PUBLIC

PRIVATE	HANDLE console_handler;
PRIVATE CONSOLE_SCREEN_BUFFER_INFO console_info;

void log_wsa_error(int error_code)
{
	wchar_t *s = NULL;

	console_handler = GetStdHandle(STD_OUTPUT_HANDLE); // Console handler
	GetConsoleScreenBufferInfo(console_handler, &console_info); // Saving console text colors

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

		SetConsoleTextAttribute(console_handler, FOREGROUND_RED | FOREGROUND_INTENSITY);
		fprintf(stderr, "%S", s); 
		SetConsoleTextAttribute(console_handler, console_info.wAttributes); // Restoring console text colors

		if (LocalFree(s) != NULL) { 
			DBG_ERROR("LocalFree() failed (%d)\n", GetLastError()); 
		} 
	} 
	else { 
		DBG_ERROR("Can't find error message for error code (%d)\n", error_code); 
	} 
}

void log_message(int mode, const char *format, ...)
{
	int i;
	va_list args;
	FILE *out;
	console_handler = GetStdHandle(STD_OUTPUT_HANDLE); // Console handler
	GetConsoleScreenBufferInfo(console_handler, &console_info); // Saving console text colors

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

	// Printing args
	va_start(args, format);
	vfprintf(out, format, args);
	va_end(args);

	SetConsoleTextAttribute(console_handler, console_info.wAttributes); // Restoring console text colors
}



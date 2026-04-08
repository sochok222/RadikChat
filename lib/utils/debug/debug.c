#include <debug.h>
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

PRIVATE	HANDLE console_handler;
PRIVATE CONSOLE_SCREEN_BUFFER_INFO console_info;
PRIVATE FILE *fout = NULL;


void initDebug(const char *logFile)
{
    if (logFile == NULL)
        console_handler = GetStdHandle(STD_OUTPUT_HANDLE);
    else {
        fout = fopen(logFile, "a");
    }
#ifdef LOG_TO_FILE
#define stderr fout
#define stdout fout
#endif
}

/* Log wsa error code converted to string with colorful output */
void logWsaError(int error_code)
{
	wchar_t *s = NULL; /* Pointer to string with error message */

	GetConsoleScreenBufferInfo(console_handler, &console_info); /* Save console state */

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

		SetConsoleTextAttribute(console_handler, FOREGROUND_RED | FOREGROUND_INTENSITY);

		fprintf(stderr, "%S", s); 

		SetConsoleTextAttribute(console_handler, console_info.wAttributes); /* Restore console text colors */

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

	GetConsoleScreenBufferInfo(console_handler, &console_info); /* Save console state */

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
	/* Printing args */
	va_start(args, format);
        vfprintf(out, format, args);
	va_end(args);

	SetConsoleTextAttribute(console_handler, console_info.wAttributes); /* Restore console text colors */
}

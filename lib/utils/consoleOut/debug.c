#include <debug.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <windows.h>

/************************************************ 
 * This translation unit created for colorful	*
 * debugging					                *
 ************************************************/

#define ESC "\x1b"
#define CSI "\x1b["
#define PRIVATE static
#define PUBLIC

PRIVATE FILE *fOut, *mStdout, *mStderr;
PRIVATE bool toConsole;

bool initDebug(const char *logFile)
{
    fOut = NULL;
    toConsole = logFile == NULL;

    if (logFile != NULL) {
        if ((fOut = fopen(logFile, "a")) == NULL) {
            printf("Can't open log file\n");
            return false;
        }
        mStdout = fOut;
        mStderr = fOut;
        return true;
    }

    mStdout = stdout;
    mStderr = stderr;
    return true;
}

/* Log wsa error code converted to string with colorful output */
void logWsaError(unsigned long error_code)
{
	wchar_t *s = NULL;

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
		fprintf(mStderr, "Error message: ");

	    // Red text, default bg
	    setTextColor(fgRed | bgDefault);
	    fprintf(mStderr, "%S", s);
	    setTextColor(formatDefault);

		LocalFree(s);
	}
	else { 
        DBG_ERROR("Can't find error message for error code (%d)\n", error_code);
	} 
}

void logWinError(unsigned long error_code)
{
    wchar_t *lpMsgBuf = NULL;

    if (FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&lpMsgBuf,
        0, NULL) == 0) {
        MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_OK);
        ExitProcess(error_code);
    }

    setTextColor(fgRed | bgDefault);
    fprintf(mStderr, "[WINERROR]: %S", lpMsgBuf);
    setTextColor(formatDefault);
    fflush(mStderr);

    LocalFree(lpMsgBuf);
}

/* Log message with color corresponding to level */
void logMessage(int mode, const char *format, ...)
{
	va_list args;
	FILE *out;

	switch (mode) {
		case DBG_MODE_FATAL:
	        setTextColor(formatError);
			out = mStderr;
			break;
		case DBG_MODE_ERROR:
	        setTextColor(formatError);
			out = mStderr;
			break;
		case DBG_MODE_WARNING:
	        setTextColor(formatError);
			out = mStderr;
			break;
		case DBG_MODE_INFO:
	        setTextColor(formatNotification);
			out = mStdout;
			break;
		default:
			out = mStdout;
			break;
	}
	/* Printing args */
	va_start(args, format);
        vfprintf(out, format, args);
	va_end(args);
    fflush(out);

    setTextColor(formatDefault);
}

void setTextColor(TextFormat color)
{
    static char buffer[25];
    memset(buffer, 0, 25);

    strcpy(buffer, CSI);
    if (color & fgRed)
        strcat(buffer, ";31");
    if (color & fgGreen)
        strcat(buffer, ";32");
    if (color & fgDefault)
        strcat(buffer, ";39");
    if (color & bgRed)
        strcat(buffer, ";41");
    if (color & bgGreen)
        strcat(buffer, ";42");
    if (color & bgDefault)
        strcat(buffer, ";49");

    strcat(buffer, "m");

    printf(buffer);
}

void printTimeElapsed(const char *m, time_t start, time_t stop)
{
    double elapsed = ((double)stop - start) / CLOCKS_PER_SEC;
    DBG_INFO("%s %lf\n", m, elapsed);
}
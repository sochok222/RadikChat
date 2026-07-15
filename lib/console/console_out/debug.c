#include <debug.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

#define ESC "\x1b"
#define CSI "\x1b["
#define PRIVATE static
#define PUBLIC

PRIVATE FILE *f_out = nullptr, *m_stdout = nullptr, *m_stderr = nullptr;
PRIVATE bool to_console;
PRIVATE HANDLE h_out = INVALID_HANDLE_VALUE;

PRIVATE int get_color_string(char *buffer, TextFormat color);

bool init_debug(const char *log_file)
{
    f_out = NULL;
    to_console = log_file == NULL;
    h_out = GetStdHandle(STD_OUTPUT_HANDLE);

    if (log_file != NULL) {
        if ((f_out = fopen(log_file, "a")) == NULL) {
            printf("Can't open log file");
            return false;
        }
        m_stdout = f_out;
        m_stderr = f_out;
        return true;
    }

    m_stdout = stdout;
    m_stderr = stderr;
    return true;
}

void deinit_debug()
{
    if (f_out != NULL) {
        fclose(f_out);
    }
}

/* Log wsa error code converted to string with colorful output */
void log_wsa_error(unsigned long error_code)
{
	wchar_t *error_string = NULL;

	/* Get error text description from error code */
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&error_string,
		0,
		NULL
		);

	if (error_string != NULL) {
	    DBG_ERROR("[WSAERROR]: (%d) %S\n", error_code, error_string);
		LocalFree(error_string);
	}
	else {
        DBG_ERROR("Can't find error message for error code (%d)", error_code);
	}
}

void log_win_error(unsigned long error_code)
{
    wchar_t *error_string = NULL;

    if (FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&error_string,
        0, NULL) == 0) {
        MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_OK);
        ExitProcess(error_code);
    }

    DBG_ERROR("[WINERROR]: (%d) %S\n", error_code, error_string);

    LocalFree(error_string);
}

/* Log message with color corresponding to level */
void log_message(int mode, const char *format, ...)
{
    char buffer[256] = {0};
	va_list args;
	FILE *out;

    int offset = 0;
	switch (mode) {
		case DBG_MODE_FATAL:
	        offset += get_color_string(buffer, formatError);
			out = m_stderr;
			break;
		case DBG_MODE_ERROR:
	        offset += get_color_string(buffer, formatError);
			out = m_stderr;
			break;
		case DBG_MODE_WARNING:
	        offset += get_color_string(buffer, formatError);
			out = m_stderr;
			break;
		case DBG_MODE_INFO:
	        offset += get_color_string(buffer, formatNotification);
			out = m_stdout;
			break;
		default:
	        offset += get_color_string(buffer, formatDefault);
			out = m_stdout;
			break;
	}
	/* Printing args */
    // TODO Replace vfprintf with WriteConsole()
    // Because call on printf from multiple thread can cause av.
    // WriteConsole is buffered
	va_start(args, format);
        vsprintf(buffer + offset, format, args);
	va_end(args);
    WriteConsoleA(h_out, buffer, strlen(buffer), NULL, NULL);
    // fprintf(m_stderr, "%s", buffer);
    // fflush(h_out);
}

void set_text_color(TextFormat color)
{
    char buffer[25];
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

    printf("%s", buffer);
}

PRIVATE int get_color_string(char *buffer, TextFormat color)
{
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
    return strlen(buffer);
}

void print_time_elapsed(const char *m, time_t start, time_t stop)
{
    double elapsed = ((double)stop - start) / CLOCKS_PER_SEC;
    DBG_INFO("%s %lf", m, elapsed);
}
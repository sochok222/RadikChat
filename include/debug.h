#ifndef DEBUG_H
#define DEBUG_H


#define MSG 0
#define ERR 1

#ifndef NO_DEBUG_BUILD 

#define SET_TEXT_COLOR(handle, color) \
	SetConsoleTextAttribute(handle, color);


#define DEBUG(mode, ...)\
if(mode == ERR) {\
	CONSOLE_SCREEN_BUFFER_INFO buffer; \
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); \
	GetConsoleScreenBufferInfo(hConsole, &buffer); \
	SET_TEXT_COLOR(hConsole, FOREGROUND_RED); \
	fprintf(stderr, "Error: %s(), line %d: ",__FUNCTION__, __LINE__);\
	fprintf(stderr, __VA_ARGS__);\
	SET_TEXT_COLOR(hConsole, buffer.wAttributes); \
}\
else if (mode == MSG) {\
	CONSOLE_SCREEN_BUFFER_INFO buffer; \
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); \
	GetConsoleScreenBufferInfo(hConsole, &buffer); \
	SET_TEXT_COLOR(hConsole, FOREGROUND_GREEN); \
	fprintf(stdout, "%s(): ",__FUNCTION__);\
	fprintf(stdout, __VA_ARGS__);\
	SET_TEXT_COLOR(hConsole, buffer.wAttributes); \
}

#define PRINT_WSA_ERROR(error) \
CONSOLE_SCREEN_BUFFER_INFO buffer; \
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); \
GetConsoleScreenBufferInfo(hConsole, &buffer); \
SET_TEXT_COLOR(hConsole, FOREGROUND_RED); \
int error_code = error; \
wchar_t *s = NULL; \
FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, \
	NULL, \
	error_code, \
	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), \
	(LPWSTR)&s, \
	0, \
	NULL \
	); \
if (s != NULL) { \
	fprintf(stderr, "Error message: %S", s); \
	if (LocalFree(s) != NULL) { \
		fprintf(stderr, "LocalFree() failed (%d)\n", GetLastError()); \
	} \
} \
else { \
	fprintf(stderr, "Can't find error message for error code (%d)\n", error_code); \
} \
SET_TEXT_COLOR(hConsole, buffer.wAttributes);

#else 

#define DEBUG(mode, ...) 
#define PRINT_WSA_ERROR(error)

#endif // NO_DEBUG_BUILD 

#endif // DEBUG_H

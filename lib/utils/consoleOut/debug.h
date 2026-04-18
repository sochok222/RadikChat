#ifndef DEBUG_H
#define DEBUG_H

#define DBG_MODE_FATAL 0
#define DBG_MODE_ERROR 1
#define DBG_MODE_WARNING 2
#define DBG_MODE_INFO 3
#define DBG_MODE_DEBUG 4

#define DEBUG_LEVEL 4

#ifndef NO_DEBUG_BUILD 

#if DEBUG_LEVEL >= 5
#define DBG_DEBUG(...)\
    log_message(DBG_MODE_DEBUG, TEXT("[TRACE]: %s(): "), __FUNCTION__, __LINE__);\
	log_message(DBG_MODE_DEBUG, __VA_ARGS__);
#elif DEBUG_LEVEL >= 4
#define DBG_DEBUG(...)\
    logMessage(DBG_MODE_DEBUG, "[DEBUG]: ");\
	logMessage(DBG_MODE_DEBUG, __VA_ARGS__);
#define DBG_FUNC() \
    logMessage(DBG_MODE_DEBUG, "[FUNC]: ");\
    logMessage(DBG_MODE_DEBUG, __FUNCTION__);\
    logMessage(DBG_MODE_DEBUG, "\n");
#else
#define DBG_DEBUG(...)
#define DBG_FUNC()
#endif

#if DEBUG_LEVEL >= 3
#define DBG_INFO(...)\
    logMessage(DBG_MODE_INFO, "[INFO]: ");\
	logMessage(DBG_MODE_INFO, __VA_ARGS__);
#else
#define DBG_INFO(...)
#endif

#if DEBUG_LEVEL >= 2
#define DBG_WARNING(...)\
    logMessage(DBG_MODE_WARNING, "[WARNING]: %s(), line %d: ", __FUNCTION__, __LINE__);\
	logMessage(DBG_MODE_WARNING, __VA_ARGS__);
#else
#define DBG_WARNING(...)
#endif

#if DEBUG_LEVEL >= 1
#define DBG_ERROR(...)\
    logMessage(DBG_MODE_ERROR, "[ERROR] %s(), line %d: ", __FUNCTION__, __LINE__);\
	logMessage(DBG_MODE_ERROR, __VA_ARGS__);
#else
#define DBG_ERROR(...)
#endif

#if DEBUG_LEVEL >= 1
#define DBG_FATAL(...)\
    logMessage(DBG_MODE_FATAL, "[FATAL]: %s(), line %d: ", __FUNCTION__, __LINE__);\
	logMessage(DBG_MODE_FATAL, __VA_ARGS__);
#else
#define DBG_FATAL(...)
#endif

typedef enum eConsoleFormatting
{
    fgRed = 0x01,
    fgGreen = 0x02,
    fgDefault = 0x04,
    bgRed = 0x08,
    bgGreen = 0x10,
    bgDefault = 0x20,
    formatDefault = fgDefault | bgDefault,
    formatError = fgRed | bgDefault,
    formatNotification = fgGreen | bgDefault,
    formatSuccess = fgGreen | bgDefault,
} TextFormat;

void logMessage(int mode, const char *format, ...);
void logWsaError(unsigned long error_code);
void logWinError(unsigned long error_code);
bool initDebug(const char *logFile);
void setTextColor(TextFormat color);

#define PRINT_WSA_ERROR() \
	logWsaError(WSAGetLastError())

#define PRINT_WIN_ERROR() \
	logWinError(GetLastError())

#else /* NO_DEBUG_BUILD */

#define PRINT_WSA_ERROR(error)
#define PRINT_WIN_ERROR(error)
#define INIT_DEBUG() true /* If debug build is not defined debug initialization is always successful */

#endif /* NO_DEBUG_BUILD */

#endif /* DEBUG_H */

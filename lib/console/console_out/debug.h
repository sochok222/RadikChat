#ifndef DEBUG_H
#define DEBUG_H

#define DBG_MODE_FATAL 0
#define DBG_MODE_ERROR 1
#define DBG_MODE_WARNING 2
#define DBG_MODE_INFO 3
#define DBG_MODE_DEBUG 4

#define DEBUG_LEVEL 4

#ifndef NO_DEBUG_BUILD
#include <time.h>

// TODO add logging mutex

#if DEBUG_LEVEL >= 5
#define DBG_DEBUG(fmt, ...)\
    log_message(DBG_MODE_DEBUG, "[TRACE]: %s(), line %d: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#elif DEBUG_LEVEL >= 4
#define DBG_DEBUG(fmt, ...)\
    log_message(DBG_MODE_DEBUG, "[DEBUG]: " fmt "\n", ##__VA_ARGS__)
#define DBG_FUNC() \
    log_message(DBG_MODE_DEBUG, "[FUNC]: %s()\n", __FUNCTION__)
#else
#define DBG_DEBUG(...)
#define DBG_FUNC()
#endif

#if DEBUG_LEVEL >= 3
#define DBG_INFO(fmt, ...)\
    log_message(DBG_MODE_INFO, "[INFO]: " fmt "\n", ##__VA_ARGS__)
#else
#define DBG_INFO(...)
#endif

#if DEBUG_LEVEL >= 2
#define DBG_WARNING(fmt, ...) \
    log_message(DBG_MODE_WARNING, "[WARNING]: %s(), line %d: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DBG_WARNING(...)
#endif

#if DEBUG_LEVEL >= 1
#define DBG_ERROR(fmt, ...) \
    log_message(DBG_MODE_ERROR, "[ERROR] %s(), line %d: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DBG_ERROR(...)
#endif

#if DEBUG_LEVEL >= 1
#define DBG_FATAL(fmt, ...)\
    log_message(DBG_MODE_FATAL, "[FATAL]: %s(), line %d: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
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

void log_message(int mode, const char *format, ...);
void log_wsa_error(unsigned long error_code);
void log_win_error(unsigned long error_code);
bool init_debug(const char *log_file);
void set_text_color(TextFormat color);
void print_time_elapsed(const char *m, time_t start, time_t stop);

#define PRINT_WSA_ERROR() \
	log_wsa_error(WSAGetLastError())

#define PRINT_WIN_ERROR() \
	log_win_error(GetLastError())

#else /* NO_DEBUG_BUILD */

#define PRINT_WSA_ERROR(error)
#define PRINT_WIN_ERROR(error)
#define INIT_DEBUG() true /* If debug build is not defined debug initialization is always successful */

#endif /* NO_DEBUG_BUILD */

#endif /* DEBUG_H */

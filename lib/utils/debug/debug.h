#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>
#include <TCHAR.h>

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
	log_message(DBG_MODE_DEBUG, __VA_ARGS__);
#else
#define DBG_DEBUG(...)
#endif

#if DEBUG_LEVEL >= 3
#define DBG_INFO(...)\
    log_message(DBG_MODE_INFO, TEXT("[INFO]: "));\
	log_message(DBG_MODE_INFO, __VA_ARGS__);
#else
#define DBG_INFO(...)
#endif

#if DEBUG_LEVEL >= 2
#define DBG_WARNING(...)\
    log_message(DBG_MODE_WARNING, TEXT("[WARNING]: %s(), line %d: "), __FUNCTION__, __LINE__);\
	log_message(DBG_MODE_WARNING, __VA_ARGS__);
#else
#define DBG_WARNING(...)
#endif

#if DEBUG_LEVEL >= 1
#define DBG_ERROR(...)\
    log_message(DBG_MODE_ERROR, TEXT("[ERROR] %s(), line %d: "), __FUNCTION__, __LINE__);\
	log_message(DBG_MODE_ERROR, __VA_ARGS__);
#else
#define DBG_ERROR(...)
#endif

#if DEBUG_LEVEL >= 1
#define DBG_FATAL(...)\
    log_message(DBG_MODE_FATAL, TEXT("[FATAL]: %s(), line %d: "), __FUNCTION__, __LINE__);\
	log_message(DBG_MODE_FATAL, __VA_ARGS__);
#else
#define DBG_FATAL(...)
#endif

void log_message(int mode, const TCHAR *format, ...);
void log_wsa_error(int error_code);
void log_win_error(int error_code);
bool init_debug(void);

#define PRINT_WSA_ERROR(error) \
	log_wsa_error(error)

#define PRINT_WIN_ERROR(error) \
	log_win_error(error)

#define INIT_DEBUG() \
	init_debug()

#else /* NO_DEBUG_BUILD */

#define PRINT_WSA_ERROR(error)
#define PRINT_WIN_ERROR(error)
#define INIT_DEBUG() true /* If debug build is not defined debug initialization is always successful */

#endif /* NO_DEBUG_BUILD */

#endif /* DEBUG_H */

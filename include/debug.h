#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#define DBG_MODE_FATAL 0
#define DBG_MODE_ERROR 1
#define DBG_MODE_WARNING 2
#define DBG_MODE_INFO 3
#define DBG_MODE_DEBUG 4

#define DEBUG_LEVEL 4

#ifndef NO_DEBUG_BUILD 

#if DEBUG_LEVEL >= 4
#define DBG_DEBUG(...)\
	log_message(DBG_MODE_DEBUG, "[DEBUG]: %s(): ", __FUNCTION__, __LINE__);\
	log_message(DBG_MODE_DEBUG, __VA_ARGS__);
#else
#define DBG_DEBUG(...)
#endif

#if DEBUG_LEVEL >= 3
#define DBG_INFO(...)\
	log_message(DBG_MODE_INFO, "[INFO]: ");\
	log_message(DBG_MODE_INFO, __VA_ARGS__);
#else
#define DBG_INFO(...)
#endif

#if DEBUG_LEVEL >= 2
#define DBG_WARNING(...)\
	log_message(DBG_MODE_WARNING, "[WARNING]: %s(), line %d: ", __FUNCTION__, __LINE__);\
	log_message(DBG_MODE_WARNING, __VA_ARGS__);
#else
#define DBG_WARNING(...)
#endif

#if DEBUG_LEVEL >= 1
#define DBG_ERROR(...)\
	log_message(DBG_MODE_ERROR, "[ERROR] %s(), line %d: ", __FUNCTION__, __LINE__);\
	log_message(DBG_MODE_ERROR, __VA_ARGS__);
#else
#define DBG_ERROR(...)
#endif

#if DEBUG_LEVEL >= 1
#define DBG_FATAL(...)\
	log_message(DBG_MODE_FATAL, "[FATAL]: %s(), line %d: ", __FUNCTION__, __LINE__);\
	log_message(DBG_MODE_FATAL, __VA_ARGS__);
#else
#define DBG_FATAL(...)
#endif

void log_message(int mode, const char *format, ...);
void log_wsa_error(int error_code);

#define PRINT_WSA_ERROR(error) \
log_wsa_error(error)

#else 

#define DBG(mode, ...) 
#define PRINT_WSA_ERROR(error)

#endif // NO_DEBUG_BUILD 

#endif // DEBUG_H

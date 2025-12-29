#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG_ON 1

#define MSG 0
#define ERR 1

#if DEBUG_ON == 1

#define DEBUG(mode, ...)\
if(mode == ERR) {\
printf("Error : %s, %d: ",__FUNCTION__, __LINE__);\
printf(__VA_ARGS__);\
}\
else if (mode == MSG) {\
printf("Debug: %s: ",__FUNCTION__);\
printf(__VA_ARGS__);\
}

#else

#define DEBUG(...) 

#endif // DEBUG_ON

#endif // DEBUG_H

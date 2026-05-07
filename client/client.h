#ifndef RADIKCHAT_CLIENT_H
#define RADIKCHAT_CLIENT_H

#include <windows.h>

typedef struct sAppData
{
    size_t contactCount;
    HANDLE messageEvent;
} AppData;

extern AppData appData;

#endif //RADIKCHAT_CLIENT_H
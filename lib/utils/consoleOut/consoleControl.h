#ifndef RADIKCHAT_CONSOLECONTROL_H
#define RADIKCHAT_CONSOLECONTROL_H

#include <windows.h>

void drawNotificationBar(void);
void drawTextInputBar(void);
void drawMainWindowBar(void);
void inputThread(void*);

extern HANDLE consoleOutMutex;

#endif //RADIKCHAT_CONSOLECONTROL_H

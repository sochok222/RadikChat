#ifndef RADIKCHAT_CONSOLECONTROL_H
#define RADIKCHAT_CONSOLECONTROL_H

#include <windows.h>

#define showCursor() setCursorVisibility(true)
#define hideCursor() setCursorVisibility(false)

bool initConsoleSize(void);
void inputThread(void*);
void setAlternateConsoleBuffer(bool alternate);
void setPos(int row, int col);
bool getConsoleSize(int *width, int *height);
void lockConsoleSize(bool lock);
inline int getConsoleWidth(void);
inline int getConsoleHeight(void);
void clearCurrentLine(int width);
bool enableVirtualProcessing(bool enable);
bool disableSelection(bool disable);

void setCursorVisibility(bool visible);

extern HANDLE consoleCursorMutex;

#endif //RADIKCHAT_CONSOLECONTROL_H

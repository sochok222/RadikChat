#ifndef RADIKCHAT_CONSOLECONTROL_H
#define RADIKCHAT_CONSOLECONTROL_H

#include <windows.h>

void inputThread(void*);
void setAlternateConsoleBuffer(bool alternate);
void setPos(int row, int col);
void saveCursorPos();
void restoreCursorPos();
bool getConsoleSize(DWORD64 *size);
void lockConsoleSize(bool lock);
inline int getConsoleWidth(DWORD64 size);
inline int getConsoleHeight(DWORD64 size);
void clearCurrentRow(int width);

#endif //RADIKCHAT_CONSOLECONTROL_H

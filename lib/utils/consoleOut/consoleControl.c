#include "consoleControl.h"

#include "consoleOutput.h"

#include <inttypes.h>
#include <stdio.h>

#define ESC "\x1b"
#define CSI "\x1b["
#define PUBLIC
#define PRIVATE static


bool getConsoleSize(DWORD64 *size)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return false;
    }

    *size = 0;
    *size |= csbi.srWindow.Bottom;
    *size <<= 32;
    *size |= csbi.srWindow.Right;
    return true;
}

void setAlternateConsoleBuffer(bool alternate)
{
    if (alternate == true)
        printf(CSI "?1049h");
    else
        printf(CSI "?1049l");
}

void setPos(int row, int col)
{
    char buffer[35] = CSI;

    sprintf(buffer + strlen(buffer), "%d", row);
    strcat(buffer, ";");
    sprintf(buffer + strlen(buffer), "%d", col);
    strcat(buffer, "H");
    printf(buffer);
}

void saveCursorPos()
{
    printf(CSI"s");
}

void restoreCursorPos()
{
    printf(CSI"u");
}

void lockConsoleSize(bool lock)
{
    HWND consoleWindow = GetConsoleWindow();
    if (lock == true)
        SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
    else
        SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & WS_MAXIMIZEBOX & WS_SIZEBOX);
}

int getConsoleWidth(DWORD64 size)
{
    return *(int*)&size;
}

int getConsoleHeight(DWORD64 size)
{
    return *((int*)&size + 1);
}

void clearCurrentRow(int width)
{
    char buffer[15] = CSI;
    sprintf(buffer + strlen(buffer), "%d", width);
    strcat(buffer, "@");
    mprintf(buffer);
}
#include "consoleControl.h"

#include "consoleInput.h"
#include "consoleOutput.h"

#include <inttypes.h>
#include <stdio.h>

#define ESC "\x1b"
#define CSI "\x1b["
#define PUBLIC
#define PRIVATE static

HANDLE consoleCursorMutex;
static int consoleWidth, consoleHeight;

bool initConsoleSize(void)
{
    if (!getConsoleSize(&consoleWidth, &consoleHeight))
        return false;

    initInput(consoleWidth, consoleHeight);
    initOutput(consoleWidth, consoleHeight);
    return true;
}

bool getConsoleSize(int *width, int *height)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return false;
    }

    *height = csbi.srWindow.Bottom + 1;
    *width = csbi.srWindow.Right + 1;
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

    sprintf(buffer + strlen(buffer), "%d", row + 1);
    strcat(buffer, ";");
    sprintf(buffer + strlen(buffer), "%d", col + 1);
    strcat(buffer, "H");
    printf("%s", buffer);
}

void lockConsoleSize(bool lock)
{
    HWND consoleWindow = GetConsoleWindow();
    if (lock == true)
        SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
    else
        SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & WS_MAXIMIZEBOX & WS_SIZEBOX);
}

int getConsoleWidth(void)
{
    return consoleWidth;
}

int getConsoleHeight(void)
{
    return consoleHeight;
}

void clearCurrentLine(int width)
{
    char buffer[15] = CSI;
    sprintf(buffer + strlen(buffer), "%d", width + 1);
    strcat(buffer, "@");
    printf(buffer);
}

void setCursorVisibility(bool visible)
{
   if (visible == true) {
       printf(CSI "?25h");
   } else {
       printf(CSI "?25l");
   }
}

bool enableVirtualProcessing(bool enable)
{
    DWORD consoleMode = 0;
    HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!GetConsoleMode(stdHandle, &consoleMode)) {
        DBG_ERROR("Can't get console mode\n");
        return false;
    }

    if (enable == true)
        consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    else
        consoleMode &= ~ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (!SetConsoleMode(stdHandle, consoleMode)) {
        DBG_ERROR("Can't set console mode\n");
        return false;
    }

    return true;
}

bool disableSelection(bool disable)
{
    DWORD consoleMode = 0;
    HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!GetConsoleMode(stdHandle, &consoleMode)) {
        DBG_ERROR("Can't get console mode\n");
        return false;
    }

    if (disable == true) {
        consoleMode ^= ENABLE_QUICK_EDIT_MODE;
    }
    else
        consoleMode &= ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS;

    if (!SetConsoleMode(stdHandle, consoleMode)) {
        DBG_ERROR("Can't set console mode (%d)\n", GetLastError());
        return false;
    }

    return true;
}
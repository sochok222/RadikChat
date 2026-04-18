#include "consoleOutput.h"

#include "consoleControl.h"
#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdio.h>

#define REQUEST_POS consoleHeight - 1, 0

static int consoleHeight, consoleWidth;
static HANDLE consoleOutMutex;

bool initConsoleOutput()
{
    DWORD64 consoleSize;

    if (!getConsoleSize(&consoleSize))
        return false;

    consoleHeight = getConsoleHeight(consoleSize);
    consoleWidth = getConsoleWidth(consoleSize);
    return true;
}

void colorfulPrintf(TextFormat color, const char *format, ...)
{
    va_list args;

    WaitForSingleObject(consoleOutMutex, INFINITE);
    setTextColor(color);
    va_start(args, format);
    mvprintf(format, args);
    va_end(args);
    setTextColor(formatDefault);
    ReleaseMutex(consoleOutMutex);
}

void colorfulVPrintf(TextFormat color, const char *format, va_list args)
{
    WaitForSingleObject(consoleOutMutex, INFINITE);
    setTextColor(color);
    mvprintf(format, args);
    setTextColor(formatDefault);
    ReleaseMutex(consoleOutMutex);
}

void printRequest(const char *format, ...)
{
    va_list args;

    saveCursorPos();

    setPos(REQUEST_POS);
    va_start(args, format);
        mvprintf(format, args);
    va_end(args);

    restoreCursorPos();
}

void mprintf(const char *format, ...)
{
    static va_list args;
    WaitForSingleObject(consoleOutMutex, INFINITE);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    ReleaseMutex(consoleOutMutex);
}

void mvprintf(const char *format, va_list args)
{
    WaitForSingleObject(consoleOutMutex, INFINITE);
    vprintf(format, args);
    ReleaseMutex(consoleOutMutex);
}

void mfprintf(FILE *fout, const char *format, ...)
{
    static va_list args;
    WaitForSingleObject(consoleOutMutex, INFINITE);
    va_start(args, format);
    vfprintf(fout, format, args);
    va_end(args);
    ReleaseMutex(consoleOutMutex);
}

void mvfprintf(FILE *fout, const char *format, va_list args)
{
    WaitForSingleObject(consoleOutMutex, INFINITE);
    vfprintf(fout, format, args);
    ReleaseMutex(consoleOutMutex);
}

void drawNotificationBar(void)
{
    saveCursorPos();
    setPos(0, 0);
    printf("Last Notification: ");
    restoreCursorPos();
}

void printNotification(TextFormat textFormat, const char *format, ...)
{
    static va_list args;
    saveCursorPos();
    setPos(0, strlen("Last Notification: ") + 1);
    va_start(args, format);
    colorfulVPrintf(textFormat, format, args);
    va_end(args);
    restoreCursorPos();
}

void clearNotificationBar(void)
{
    saveCursorPos();
    setPos(0, strlen("Last Notification: ") + 1);
    clearCurrentRow(consoleWidth);
    restoreCursorPos();
}

void drawTextInputBar(void)
{
    // Go to lower-left corner
    setPos(consoleHeight, 0);
    printf(">");
}

void clearTextInputBar(void)
{
    saveCursorPos();
    setPos(consoleHeight, 2);
    clearCurrentRow(consoleWidth);
    restoreCursorPos();
}

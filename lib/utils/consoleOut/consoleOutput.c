#include "consoleOutput.h"

#include "consoleControl.h"
#include "debug.h"

#include <process.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

#define REQUEST_POS consoleHeight - 1, 0
#define ESC "\x1b"
#define CSI "\x1b["
#define calcPos(row, col) ((row) * (consoleWidth) + (col))

static int consoleWidth, consoleHeight;
static int mainAreaStart, mainAreaEnd;
static char *consoleBuffer;
static HANDLE hStdOut;
static HANDLE consoleOutMutex;
static HANDLE consoleOutSemaphore;

static void writeToConsoleBuffer(const char *data, size_t size, int row, int col, bool updateScreen);
static void vprintToConsoleBuffer(int row, int col, const char *format, va_list args);
static void printToConsoleBuffer(int row, int col, const char *format, ...);
static void clearBufferLine(int row, int col);
static void clearLine(int row, int col);
static void drawSeparatorLine(int row, int col, bool updateScreen);
static void redrawConsole(void);

void initOutput(int width, int height)
{
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    consoleOutMutex = CreateMutex(NULL, FALSE, NULL);
    consoleOutSemaphore = CreateSemaphore(NULL, 0, 100, NULL);
    consoleWidth = width;
    consoleHeight = height;

    mainAreaStart = 1;
    mainAreaEnd = consoleHeight - 1 - 2;

    consoleBuffer = calloc(consoleWidth * consoleHeight, sizeof(char));
}

int getMainAreaHeight(void)
{
    // Main are height - border height
    return mainAreaEnd - mainAreaStart - 2;
}

void colorfulPrintf(TextFormat color, const char *format, ...)
{
    va_list args;

    WaitForSingleObject(consoleOutMutex, INFINITE);
    setTextColor(color);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    setTextColor(formatDefault);
    ReleaseMutex(consoleOutMutex);
}

void colorfulVPrintf(TextFormat color, const char *format, va_list args)
{
    WaitForSingleObject(consoleOutMutex, INFINITE);
    setTextColor(color);
    vprintf(format, args);
    setTextColor(formatDefault);
    ReleaseMutex(consoleOutMutex);
}

void printRequest(const char *format, ...)
{
    va_list args;

    clearRequest();
    va_start(args, format);
    vprintToConsoleBuffer(consoleHeight - 2, 0, format, args);
    va_end(args);
}

void clearRequest()
{
    clearLine(consoleHeight - 2, 0);
}

void printNotification(TextFormat textFormat, const char *format, ...)
{
    va_list args;

    writeToConsoleBuffer("Last Notification: ", strlen("Last Notification: "), 0, 0, true);
    va_start(args, format);
    vprintToConsoleBuffer(0, strlen("Last Notification: "), format, args);
    va_end(args);
}

void clearNotificationBar(void)
{
    clearLine(0, 0);
}

void drawTextInputBar(void)
{
    writeToConsoleBuffer(">", 1, consoleHeight - 1, 0, true);
}

void clearTextInputBar(void)
{
    clearLine(consoleHeight - 1, 0);
}

void clearScreen(void)
{
    WaitForSingleObject(consoleOutMutex, INFINITE);
    memset(consoleBuffer, 0, consoleWidth * consoleHeight);
    ReleaseMutex(consoleOutMutex);
    ReleaseSemaphore(consoleOutSemaphore, 1, NULL);
}

void printChatHistory(ChatHistory history, int startFrom)
{
    int i = 0;
    if (startFrom <= 0 && startFrom > history.messages)
        return;

    if (getMainAreaHeight() < history.messages) {
        for (int i = 0; i < history.messages - getMainAreaHeight() - startFrom; i++) {
            history.head = history.head->next;
        }
    }

    drawSeparatorLine(mainAreaStart + 1, 0, false);
    drawSeparatorLine(mainAreaEnd, 0, false);

    // Clear chat frame
    for (i = mainAreaStart + 2; history.head != NULL && i < mainAreaEnd; i++)
        clearBufferLine(i, 0);

    for (i = mainAreaStart + 2; history.head != NULL && i < mainAreaEnd; i++) {
        writeToConsoleBuffer(history.head->message, strlen(history.head->message), i, 0, false);
        history.head = history.head->next;
    }
    redrawConsole();
}

void printContactName(const char *format, ...)
{
    clearLine(mainAreaStart, 0);
    va_list args;
    va_start(args, format);
    vprintToConsoleBuffer(mainAreaStart, 0, format, args);
    va_end(args);
}

void printContacts(Contact *contact, int startFrom)
{
    int i = 0;
    int startPos = mainAreaStart + 1;

    if (contact == NULL)
        return;

    for (; i < startFrom; i++) {
        contact = contact->next;
    }

    // TODO optimize this like in printChatHistory
    drawSeparatorLine(mainAreaStart, 0, true);
    drawSeparatorLine(mainAreaEnd, 0, true);
    for (i = startPos; contact != NULL && i < mainAreaEnd; i++) {
        printToConsoleBuffer(i, 0, "%d - %s", startFrom++, contact->nickname);
        contact = contact->next;
    }
}

void writeToInputLine(const char *buffer)
{
    writeToConsoleBuffer(buffer, strlen(buffer), consoleHeight - 1, 2, true);
}

static CHAR_INFO *toCHAR_INFOArray(const char *buffer)
{
    static CHAR_INFO *charArray = NULL;
    if (charArray == NULL) {
        charArray = malloc(sizeof(CHAR_INFO) * consoleWidth * consoleHeight);
    }
    memset(charArray, 0, sizeof(CHAR_INFO) * consoleWidth * consoleHeight);

    for (register int i = 0; i < consoleWidth * consoleHeight; i++) {
        charArray[i].Char.AsciiChar = buffer[i];
        charArray[i].Attributes = FOREGROUND_GREEN | BACKGROUND_BLUE;
    }

    return charArray;
}

void consoleDrawThread(void *)
{
    DWORD written;
    COORD pos = {0, 0};

    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    while (true) {
        WaitForSingleObject(consoleOutSemaphore, INFINITE);
        WaitForSingleObject(consoleOutMutex, INFINITE);
        setPos(0, 0);
        WriteConsoleOutputCharacterA(
            hStdOut,
            consoleBuffer,
            consoleWidth * consoleHeight,
            pos,
            &written
        );
        ReleaseMutex(consoleOutMutex);
    }
    _endthread();
}

static void writeToConsoleBuffer(const char *data, size_t size, int row, int col, bool updateScreen)
{
    WaitForSingleObject(consoleOutMutex, INFINITE);
    if (col < consoleWidth && row < consoleHeight) {
        memset(consoleBuffer + calcPos(row, col), 0, consoleWidth);
        memcpy(consoleBuffer + calcPos(row, col), data, min(size, consoleWidth - col));
    }
    ReleaseMutex(consoleOutMutex);
    if (updateScreen)
        ReleaseSemaphore(consoleOutSemaphore, 1, NULL);
}

static void vprintToConsoleBuffer(int row, int col, const char *format, va_list args)
{
    WaitForSingleObject(consoleOutMutex, INFINITE);
    if (col < consoleWidth && row < consoleHeight) {
        vsnprintf(consoleBuffer + calcPos(row, col), consoleWidth - col, format, args);
    }
    ReleaseMutex(consoleOutMutex);
    ReleaseSemaphore(consoleOutSemaphore, 1, NULL);
}

static void printToConsoleBuffer(int row, int col, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintToConsoleBuffer(row, col, format, args);
    va_end(args);
}

static void clearBufferLine(int row, int col)
{
    if (col < consoleWidth && row < consoleHeight) {
        memset(consoleBuffer + calcPos(row, col), 0, consoleWidth - col);
    }
}

static void clearLine(int row, int col)
{
    WaitForSingleObject(consoleOutMutex, INFINITE);
    if (col < consoleWidth && row < consoleHeight) {
        memset(consoleBuffer + calcPos(row, col), 0, consoleWidth - col);
    }
    ReleaseMutex(consoleOutMutex);
    ReleaseSemaphore(consoleOutSemaphore, 1, NULL);
}

static void drawSeparatorLine(int row, int col, bool updateScreen)
{
    WaitForSingleObject(consoleOutMutex, INFINITE);
    if (col < consoleWidth && row < consoleHeight) {
        memset(consoleBuffer + calcPos(row, col), '-', consoleWidth - col);
    }
    ReleaseMutex(consoleOutMutex);
    if (updateScreen)
        ReleaseSemaphore(consoleOutSemaphore, 1, NULL);
}

static void redrawConsole(void)
{
    ReleaseSemaphore(consoleOutSemaphore, 1, NULL);
}
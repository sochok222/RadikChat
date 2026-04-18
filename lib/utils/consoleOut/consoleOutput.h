#ifndef RADIKCHAT_CONSOLEOUTPUT_H
#define RADIKCHAT_CONSOLEOUTPUT_H

#include "debug.h"
#include <stdio.h>
#include <stdarg.h>

#define printSuccess(...) printNotification(formatSuccess, __VA_ARGS__)
#define printError(...) printNotification(formatError, __VA_ARGS__)

bool initConsoleOutput();
void colorfulPrintf(TextFormat color, const char *format, ...);
void colorfulVPrintf(TextFormat color, const char *format, va_list args);
void printRequest(const char *format, ...);
void mprintf(const char *format, ...);
void mvprintf(const char *format, va_list args);
void mfprintf(FILE *fout, const char *format, ...);
void mvfprintf(FILE *fout, const char *format, va_list args);
void drawNotificationBar(void);
void printNotification(TextFormat textFormat, const char *format, ...);
void clearNotificationBar(void);
void drawTextInputBar(void);
void clearTextInputBar(void);

#endif //RADIKCHAT_CONSOLEOUTPUT_H

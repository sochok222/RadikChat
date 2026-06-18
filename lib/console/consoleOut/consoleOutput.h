#ifndef RADIKCHAT_CONSOLEOUTPUT_H
#define RADIKCHAT_CONSOLEOUTPUT_H

#include "contact.h"
#include "debug.h"
#include <stdarg.h>
#include <stdio.h>
#include <synchapi.h>

#define printSuccess(...) printNotification(formatSuccess, __VA_ARGS__)
#define printError(...) printNotification(formatError, __VA_ARGS__)

void initOutput(int width, int height);
int  getMainAreaHeight(void);
void colorfulPrintf(TextFormat color, const char *format, ...);
void colorfulVPrintf(TextFormat color, const char *format, va_list args);
void printRequest(const char *format, ...);
void clearRequest(void);
void printNotification(TextFormat textFormat, const char *format, ...);
void clearNotificationBar(void);
void drawTextInputBar(void);
void clearTextInputBar(void);
void printChatHistory(ChatHistory history, int startFrom);
void printContactName(const char *format, ...);
void printContacts(Contact *contact, int startFrom);
void writeToInputLine(const char *buffer);
void clearScreen(void);
void consoleDrawThread(void *);

#endif //RADIKCHAT_CONSOLEOUTPUT_H

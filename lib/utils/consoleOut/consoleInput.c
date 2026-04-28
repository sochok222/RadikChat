#include "consoleInput.h"

#include "consoleControl.h"
#include "consoleOutput.h"
#include <conio.h>
#include <string.h>

static int consoleWidth, consoleHeight;
static int maxChars;
static inline void setInputCursorPos(void);

void initInput(int width, int height)
{
    maxChars = width - 2;
    consoleWidth = width;
    consoleHeight = height;
}

char *readInput()
{
    static char inputBuffer[512];
    int inputPos = 0;
    int ch;

    clearTextInputBar();
    drawTextInputBar();
    setInputCursorPos();
    while (true) {
        if (_kbhit()) {
            switch (ch = getch()) {
            case 8: /* backspace */
                if (inputPos > 0) {
                    inputPos--;
                    inputBuffer[inputPos] = '\0';
                    writeToInputLine(inputBuffer);
                }
                break;
            case 13: case 10: /* enter key */
                inputBuffer[inputPos] = 0x0;
                clearTextInputBar();
                return inputBuffer;
            default:
                if (inputPos < getConsoleWidth()) {
                    inputBuffer[inputPos++] = ch;
                    inputBuffer[inputPos] = '\0';
                    writeToInputLine(inputBuffer);
                } else {
                    printError("Limit of characters");
                }
            }
        }
    }
}

void readInBuffer(char *buffer, int bufferSize)
{
    int inputPos = 0;
    int ch;

    if (bufferSize == 1) {
        fseek(stdin, 0, SEEK_END);
        *buffer = getch();
        return;
    }

    clearTextInputBar();
    drawTextInputBar();
    setInputCursorPos();
    while (true) {
        if (_kbhit()) {
            switch (ch = getch()) {
            case 8: /* backspace */
                if (inputPos > 0) {
                    inputPos--;
                    buffer[inputPos] = '\0';
                    writeToInputLine(buffer);
                }
                break;
            case 13: case 10: /* enter key */
                buffer[inputPos] = 0x0;
                clearTextInputBar();
                return;
            default:
                if (inputPos < bufferSize - 1) {
                    buffer[inputPos++] = ch;
                    buffer[inputPos] = '\0';
                    writeToInputLine(buffer);
                } else {
                    printError("Limit of characters");
                }
            }
        }
    }
}

int readChar(bool echo)
{
    int ch;

    clearTextInputBar();
    while (true) {
        if (_kbhit()) {
            ch = getch();
            if (echo)
                putch(ch);
            return ch;
        }
    }
}

static void setInputCursorPos(void)
{
    setPos(consoleHeight - 1, 2);
}
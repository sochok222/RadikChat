#include "consoleControl.h"

#define PUBLIC
#define PRIVATE static

PRIVATE int consoleRows, consoleCols;
PRIVATE bool saveConsoleSize(void);

bool saveConsoleSize(void)
{
    PCONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };

    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), csbi)) {
        return false;
    }

    consoleRows = csbi.srWindow.Bottom;
    consoleCols = csbi.srWindow.Right;
    return true;
}

void drawTextInputBar(void)
{
    // Go to lower-left corner

}
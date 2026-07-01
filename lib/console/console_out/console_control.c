#include "console_control.h"

#include "console_input.h"
#include "console_output.h"

#include <inttypes.h>
#include <stdio.h>

#define ESC "\x1b"
#define CSI "\x1b["
#define PUBLIC
#define PRIVATE static

HANDLE console_cursor_mutex;
static int console_width, console_height;

bool init_console_size(void)
{
    if (!get_console_size(&console_width, &console_height))
        return false;

    init_input(console_width, console_height);
    init_output(console_width, console_height);
    return true;
}

bool get_console_size(int *width, int *height)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return false;
    }

    *height = csbi.srWindow.Bottom + 1;
    *width = csbi.srWindow.Right + 1;
    return true;
}

void set_alternate_console_buffer(bool alternate)
{
    if (alternate == true)
        printf(CSI "?1049h");
    else
        printf(CSI "?1049l");
}

void set_pos(int row, int col)
{
    char buffer[35] = CSI;

    sprintf(buffer + strlen(buffer), "%d", row + 1);
    strcat(buffer, ";");
    sprintf(buffer + strlen(buffer), "%d", col + 1);
    strcat(buffer, "H");
    printf("%s", buffer);
}

void lock_console_size(bool lock)
{
    HWND console_window = GetConsoleWindow();
    if (lock == true)
        SetWindowLong(console_window, GWL_STYLE, GetWindowLong(console_window, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
    else
        SetWindowLong(console_window, GWL_STYLE, GetWindowLong(console_window, GWL_STYLE) & WS_MAXIMIZEBOX & WS_SIZEBOX);
}

int get_console_width(void)
{
    return console_width;
}

int get_console_height(void)
{
    return console_height;
}

void clear_current_line(int width)
{
    char buffer[15] = CSI;
    sprintf(buffer + strlen(buffer), "%d", width + 1);
    strcat(buffer, "@");
    printf(buffer);
}

void set_cursor_visibility(bool visible)
{
   if (visible == true) {
       printf(CSI "?25h");
   } else {
       printf(CSI "?25l");
   }
}

bool enable_virtual_processing(bool enable)
{
    DWORD console_mode = 0;
    HANDLE std_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!GetConsoleMode(std_handle, &console_mode)) {
        DBG_ERROR("Can't get console mode");
        return false;
    }

    if (enable == true)
        console_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    else
        console_mode &= ~ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (!SetConsoleMode(std_handle, console_mode)) {
        DBG_ERROR("Can't set console mode");
        return false;
    }

    return true;
}

bool disable_selection(bool disable)
{
    DWORD console_mode = 0;
    HANDLE std_handle = GetStdHandle(STD_INPUT_HANDLE);

    if (!GetConsoleMode(std_handle, &console_mode)) {
        DBG_ERROR("Can't get console mode");
        return false;
    }

    if (disable == true) {
        console_mode |= ENABLE_EXTENDED_FLAGS;
        console_mode &= ~ENABLE_QUICK_EDIT_MODE;
    }
    else
        console_mode &= ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS;

    if (!SetConsoleMode(std_handle, console_mode)) {
        DBG_ERROR("Can't set console mode");
        log_win_error(GetLastError());
        return false;
    }

    return true;
}
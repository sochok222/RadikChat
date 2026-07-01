#include "console_input.h"

#include "console_control.h"
#include "console_output.h"
#include <conio.h>
#include <string.h>

static int console_width, console_height;
static int max_chars;
static inline void set_input_cursor_pos(void);

void init_input(int width, int height)
{
    max_chars = width - 2;
    console_width = width;
    console_height = height;
}

char *read_input()
{
    static char input_buffer[512];
    int input_pos = 0;
    int ch;

    clear_text_input_bar();
    draw_text_input_bar();
    set_input_cursor_pos();
    while (true) {
        if (_kbhit()) {
            switch (ch = getch()) {
            case 8: /* backspace */
                if (input_pos > 0) {
                    input_pos--;
                    input_buffer[input_pos] = '\0';
                    write_to_input_line(input_buffer);
                }
                break;
            case 13: case 10: /* enter key */
                input_buffer[input_pos] = 0x0;
                clear_text_input_bar();
                return input_buffer;
            default:
                if (input_pos < get_console_width()) {
                    input_buffer[input_pos++] = ch;
                    input_buffer[input_pos] = '\0';
                    write_to_input_line(input_buffer);
                } else {
                    print_error("Limit of characters");
                }
            }
        }
    }
}

void read_in_buffer(char *buffer, int buffer_size)
{
    int input_pos = 0;
    int ch;

    if (buffer_size == 1) {
        fseek(stdin, 0, SEEK_END);
        *buffer = getch();
        return;
    }

    clear_text_input_bar();
    draw_text_input_bar();
    set_input_cursor_pos();
    while (true) {
        if (_kbhit()) {
            switch (ch = getch()) {
            case 8: /* backspace */
                if (input_pos > 0) {
                    input_pos--;
                    buffer[input_pos] = '\0';
                    write_to_input_line(buffer);
                }
                break;
            case 13: case 10: /* enter key */
                buffer[input_pos] = 0x0;
                clear_text_input_bar();
                return;
            default:
                if (input_pos < buffer_size - 1) {
                    buffer[input_pos++] = ch;
                    buffer[input_pos] = '\0';
                    write_to_input_line(buffer);
                } else {
                    print_error("Limit of characters");
                }
            }
        }
    }
}

int read_char(bool echo)
{
    int ch;

    clear_text_input_bar();
    while (true) {
        if (_kbhit()) {
            ch = getch();
            if (echo)
                putch(ch);
            return ch;
        }
    }
}

static void set_input_cursor_pos(void)
{
    set_pos(console_height - 1, 2);
}
#include "console_output.h"

#include "console_control.h"
#include "debug.h"

#include <process.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

#define REQUEST_POS console_height - 1, 0
#define ESC "\x1b"
#define CSI "\x1b["
#define calcPos(row, col) ((row) * (console_width) + (col))

static int console_width, console_height;
static int main_area_start, main_area_end;
static char *console_buffer;
static HANDLE h_std_out;
static HANDLE console_out_mutex;
static HANDLE console_out_semaphore;

static void write_to_console_buffer(const char *data, size_t size, int row, int col, bool update_screen);
static void vprint_to_console_buffer(int row, int col, const char *format, va_list args);
static void print_to_console_buffer(int row, int col, const char *format, ...);
static void clear_buffer_line(int row, int col);
static void clear_line(int row, int col);
static void draw_separator_line(int row, int col, bool update_screen);
static void redraw_console(void);

void init_output(int width, int height)
{
    h_std_out = GetStdHandle(STD_OUTPUT_HANDLE);
    console_out_mutex = CreateMutex(NULL, FALSE, NULL);
    console_out_semaphore = CreateSemaphore(NULL, 0, 100, NULL);
    console_width = width;
    console_height = height;

    main_area_start = 1;
    main_area_end = console_height - 1 - 2;

    console_buffer = calloc(console_width * console_height, sizeof(char));
}

int get_main_area_height(void)
{
    // Main are height - border height
    return main_area_end - main_area_start - 2;
}

void colorful_printf(TextFormat color, const char *format, ...)
{
    va_list args;

    WaitForSingleObject(console_out_mutex, INFINITE);
    set_text_color(color);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    set_text_color(formatDefault);
    ReleaseMutex(console_out_mutex);
}

void colorful_v_printf(TextFormat color, const char *format, va_list args)
{
    WaitForSingleObject(console_out_mutex, INFINITE);
    set_text_color(color);
    vprintf(format, args);
    set_text_color(formatDefault);
    ReleaseMutex(console_out_mutex);
}

void print_request(const char *format, ...)
{
    va_list args;

    clear_request();
    va_start(args, format);
    vprint_to_console_buffer(console_height - 2, 0, format, args);
    va_end(args);
}

void clear_request()
{
    clear_line(console_height - 2, 0);
}

void print_notification(TextFormat text_format, const char *format, ...)
{
    va_list args;

    write_to_console_buffer("Last Notification: ", strlen("Last Notification: "), 0, 0, true);
    va_start(args, format);
    vprint_to_console_buffer(0, strlen("Last Notification: "), format, args);
    va_end(args);
}

void clearNotificationBar(void)
{
    clear_line(0, 0);
}

void draw_text_input_bar(void)
{
    write_to_console_buffer(">", 1, console_height - 1, 0, true);
}

void clear_text_input_bar(void)
{
    clear_line(console_height - 1, 0);
}

void clear_screen(void)
{
    WaitForSingleObject(console_out_mutex, INFINITE);
    memset(console_buffer, 0, console_width * console_height);
    ReleaseMutex(console_out_mutex);
    ReleaseSemaphore(console_out_semaphore, 1, NULL);
}

void print_chat_history(ChatHistory history, int start_from)
{
    int i = 0;
    WaitForSingleObject(console_out_mutex, INFINITE);
    if (start_from <= 0 && start_from > history.messages)
        return;

    if (get_main_area_height() < history.messages) {
        for (int i = 0; i < history.messages - get_main_area_height() - start_from; i++) {
            history.head = history.head->next;
        }
    }

    draw_separator_line(main_area_start + 1, 0, false);
    draw_separator_line(main_area_end, 0, false);

    // Clear chat frame
    for (i = main_area_start + 2; history.head != NULL && i < main_area_end; i++)
        clear_buffer_line(i, 0);

    // Print messages
    for (i = main_area_start + 2; history.head != NULL && i < main_area_end; i++) {
        int spacing = 0;
        write_to_console_buffer(history.head->sender == true ? "You:" : "Contact:",
            history.head->sender ? strlen("You:") : strlen("Contact:"), i, spacing, false);
        spacing += history.head->sender ? strlen("You:") : strlen("Contact:") + 2;
        write_to_console_buffer(history.head->text, strlen(history.head->text), i,
            spacing, false);
        spacing += strlen(history.head->text) + 1;

        if (history.head->state == MESSAGE_SEND_PENDING)
            write_to_console_buffer("(+)", strlen("(+)"), i, spacing, false);
        else if (history.head->state == MESSAGE_SEND_SUCCESS)
            write_to_console_buffer("(++)", strlen("(++)"), i, spacing, false);
        else
            write_to_console_buffer("(not sent)", strlen("(not sent)"), i, spacing, false);

        history.head = history.head->next;
    }
    ReleaseMutex(console_out_mutex);
    redraw_console();
}

void print_contact_name(const char *format, ...)
{
    clear_line(main_area_start, 0);
    va_list args;
    va_start(args, format);
    vprint_to_console_buffer(main_area_start, 0, format, args);
    va_end(args);
}

void print_contacts(Contact *contact, int start_from)
{
    int i = 0;
    int start_pos = main_area_start + 1;

    if (contact == NULL)
        return;

    for (; i < start_from; i++) {
        contact = contact->next;
    }

    // TODO optimize this like in print_chat_history
    draw_separator_line(main_area_start, 0, true);
    draw_separator_line(main_area_end, 0, true);
    for (i = start_pos; contact != NULL && i < main_area_end; i++) {
        print_to_console_buffer(i, 0, "%d - %s", start_from++, contact->nickname);
        contact = contact->next;
    }
}

void write_to_input_line(const char *buffer)
{
    write_to_console_buffer(buffer, strlen(buffer), console_height - 1, 2, true);
}

void console_draw_thread(void *)
{
    DWORD written;
    COORD pos = {0, 0};

    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    while (true) {
        WaitForSingleObject(console_out_semaphore, INFINITE);
        WaitForSingleObject(console_out_mutex, INFINITE);
        set_pos(0, 0);
        WriteConsoleOutputCharacterA(
            h_std_out,
            console_buffer,
            console_width * console_height,
            pos,
            &written
        );
        ReleaseMutex(console_out_mutex);
    }
    _endthread();
}

static void write_to_console_buffer(const char *data, size_t size, int row, int col, bool update_screen)
{
    WaitForSingleObject(console_out_mutex, INFINITE);
    if (col < console_width && row < console_height) {
        memset(console_buffer + calcPos(row, col), 0, console_width);
        memcpy(console_buffer + calcPos(row, col), data, min(size, console_width - col));
    }
    ReleaseMutex(console_out_mutex);
    if (update_screen)
        ReleaseSemaphore(console_out_semaphore, 1, NULL);
}

static void vprint_to_console_buffer(int row, int col, const char *format, va_list args)
{
    WaitForSingleObject(console_out_mutex, INFINITE);
    if (col < console_width && row < console_height) {
        vsnprintf(console_buffer + calcPos(row, col), console_width - col, format, args);
    }
    ReleaseMutex(console_out_mutex);
    ReleaseSemaphore(console_out_semaphore, 1, NULL);
}

static void print_to_console_buffer(int row, int col, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprint_to_console_buffer(row, col, format, args);
    va_end(args);
}

static void clear_buffer_line(int row, int col)
{
    if (col < console_width && row < console_height) {
        memset(console_buffer + calcPos(row, col), 0, console_width - col);
    }
}

static void clear_line(int row, int col)
{
    WaitForSingleObject(console_out_mutex, INFINITE);
    if (col < console_width && row < console_height) {
        memset(console_buffer + calcPos(row, col), 0, console_width - col);
    }
    ReleaseMutex(console_out_mutex);
    ReleaseSemaphore(console_out_semaphore, 1, NULL);
}

static void draw_separator_line(int row, int col, bool update_screen)
{
    WaitForSingleObject(console_out_mutex, INFINITE);
    if (col < console_width && row < console_height) {
        memset(console_buffer + calcPos(row, col), '-', console_width - col);
    }
    ReleaseMutex(console_out_mutex);
    if (update_screen)
        ReleaseSemaphore(console_out_semaphore, 1, NULL);
}

static void redraw_console(void)
{
    ReleaseSemaphore(console_out_semaphore, 1, NULL);
}
#ifndef RADIKCHAT_CONSOLEOUTPUT_H
#define RADIKCHAT_CONSOLEOUTPUT_H

#include "contact.h"
#include "debug.h"
#include <stdarg.h>
#include <stdio.h>
#include <synchapi.h>

#define printSuccess(...) print_notification(formatSuccess, __VA_ARGS__)
#define print_error(...) print_notification(formatError, __VA_ARGS__)

void init_output(int width, int height);
void deinit_output(void);
int  get_main_area_height(void);
void colorful_printf(TextFormat color, const char *format, ...);
void colorful_v_printf(TextFormat color, const char *format, va_list args);
void print_request(const char *format, ...);
void clear_request(void);
void print_notification(TextFormat text_format, const char *format, ...);
void clearNotificationBar(void);
void draw_text_input_bar(void);
void clear_text_input_bar(void);
void print_chat_history(ChatHistory history, int start_from);
void print_contact_name(const char *format, ...);
void print_contacts(Contact *contact, int start_from);
void write_to_input_line(const char *buffer);
void clear_screen(void);
void console_draw_thread(void *);

#endif //RADIKCHAT_CONSOLEOUTPUT_H

#ifndef RADIKCHAT_CONSOLECONTROL_H
#define RADIKCHAT_CONSOLECONTROL_H

#include <windows.h>

#define showCursor() set_cursor_visibility(true)
#define hide_cursor() set_cursor_visibility(false)

bool init_console_size(void);
void input_thread(void*);
void set_alternate_console_buffer(bool alternate);
void set_pos(int row, int col);
bool get_console_size(int *width, int *height);
void lock_console_size(bool lock);
inline int get_console_width(void);
inline int get_console_height(void);
void clear_current_line(int width);
bool enable_virtual_processing(bool enable);
bool disable_selection(bool disable);

void set_cursor_visibility(bool visible);

extern HANDLE console_cursor_mutex;

#endif //RADIKCHAT_CONSOLECONTROL_H

#ifndef RADIKCHAT_CONSOLEINPUT_H
#define RADIKCHAT_CONSOLEINPUT_H

void init_input(int width, int height);
char *read_input();
void read_in_buffer(char *buffer, int buffer_size);
int  read_char(bool echo);

#endif //RADIKCHAT_CONSOLEINPUT_H

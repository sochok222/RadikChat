#ifndef RADIKCHAT_CONSOLEINPUT_H
#define RADIKCHAT_CONSOLEINPUT_H

void initInput(int width, int height);
char *readInput();
void readInBuffer(char *buffer, int bufferSize);
int  readChar(bool echo);

#endif //RADIKCHAT_CONSOLEINPUT_H

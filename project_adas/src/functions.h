#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#define READ 0
#define WRITE  1

void writeMessageToPipe(int pipeFd, const char * format, ...);

int createPipe(char *pipeName);

int openPipeOnRead(char *pipeName);

int readline(int fd, char *str);

void formattedTime(char *timeBuffer);

int readFromPipe(int fileDescriptor);

int readFromSocket (int fd, char *str);  

void writeMessage(FILE *fp, const char * format, ...);


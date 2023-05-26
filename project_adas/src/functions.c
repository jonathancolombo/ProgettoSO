
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>


void writeMessageToPipe(int pipeFd, const char * format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);
    write(pipeFd, buffer, strlen(buffer)+1);
}

int createPipe(char *pipeName)
{
    int fileDescriptor;
    unlink(pipeName);
    if (mknod(pipeName, __S_IFIFO, 0) < 0)
    { // Creating named pipe
        exit(EXIT_FAILURE);
    }
    chmod(pipeName, 0660);
    do
    {
        fileDescriptor = open(pipeName, O_WRONLY); // Opening named pipe for write
        if (fileDescriptor == -1)
        {
            printf("%s non trovata. Riprova ancora...\n", pipeName);
            sleep(1);
        }
    } while (fileDescriptor == -1);
    return fileDescriptor;
}

int openPipeOnRead(char *pipeName)
{
    int fileDescriptor;
    do
    {
        fileDescriptor = open(pipeName, O_RDONLY); // Opening named pipe for write
        if (fileDescriptor == -1)
        {
            printf("Pipename:%s non trovata. Riprova ancora...\n", pipeName);
            sleep(1);
        }
    } while (fileDescriptor == -1);
    return fileDescriptor;
}

int readline (int fd, char *str) {
    int n;
    do {
        if(read (fd, str, 1) < 0) {
            return -1;
        }
    } while(*str++ != '\0');
    return 0;
}

void formattedTime(char *timeBuffer) 
{
    time_t rawTime;
    struct tm *info;
    time(&rawTime);
    info = localtime(&rawTime);
    strftime(timeBuffer,256,"%x - %X", info);
}

int readFromPipe(int fileDescriptor) 
{
    char str[256];
    if(readline(fileDescriptor, str) < 0) {
        return -1;
    }
    printf("%s\n", str);
    return 0;
}

int readFromSocket (int fd, char *str) {
    int n;
    do {
        n = read (fd, str, 1);
    } while (n > 0 && *str++ != '\0');
    return (n > 0);
}

void writeMessage(FILE *fp, const char * format, ...)
{
    char buffer[256];
    char date[256];
    formattedTime(date);
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);
    fprintf(fp, "[%s] - [%s]\n", date, buffer);
    fflush(fp);
}
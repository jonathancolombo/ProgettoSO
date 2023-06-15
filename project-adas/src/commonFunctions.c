#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int createLog(char *name, FILE **log) {
    char *fileName = (char*) malloc((strlen(name)+5)*sizeof(char));
    sprintf(fileName, "%s.log", name);
    remove(fileName);
    *log = fopen(fileName, "w");
    free(fileName);
    
    if (*log == NULL) {
        return -1; // Indica il fallimento nell'apertura del file
    }
    
    return 0; // Indica il successo
}


void formattedTime(char *timeBuffer) {
    time_t rawTime;
    struct tm *info;
    time(&rawTime);
    info = localtime(&rawTime);
    strftime(timeBuffer,256,"%x - %X", info);
}

void writeMessage(FILE *fp, const char * format, ...){
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

int createPipe(char *pipeName) {
    int fd;
    unlink(pipeName);
    if(mknod(pipeName, __S_IFIFO, 0) < 0 ) {    //Creating named pipe
        exit(EXIT_FAILURE);
    }
    chmod(pipeName, 0660);
    do {
        fd = open(pipeName, O_WRONLY);    //Opening named pipe for write
        if(fd == -1){
            printf("%s not found. Trying again...\n", pipeName);
            sleep(1);
        }
    } while(fd == -1);
    return fd;
}

int openPipeOnRead(char *pipeName) {
    int fd;
    do {
        fd = open(pipeName, O_RDONLY);    //Opening named pipe for write
        if(fd == -1){
            printf("%s not found. Trying again...\n", pipeName);
            sleep(1);
        }
    } while(fd == -1);
    return fd;
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

void writeMessageToPipe(int pipeFd, const char * format, ...){
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);
    write(pipeFd, buffer, strlen(buffer)+1);
}
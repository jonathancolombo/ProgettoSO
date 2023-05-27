#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "functions.h"

FILE *fileLog;

void handleFailure()
{
    fclose(fileLog);
    exit(EXIT_FAILURE);
}

void handleStop()
{
    writeMessage(fileLog, "ARRESTO AUTO");
}

int main(void)
{
    printf("PROCESSO BRAKE BY WIRE\n");
    
    signal(SIGTSTP, handleStop);
    signal(SIGUSR1, handleFailure);

    printf("Tento di aprire il file brake.log in scrittura\n");
    fileLog = fopen("brake.log", "w");

    if (fileLog == NULL)
    {
        printf("Errore nell'apertura del file brake.log\n");
        exit(EXIT_FAILURE);
    }

    int ecuFileDescriptor;
    char command[16];
    printf("File di log aperto correttamente\n");

    ecuFileDescriptor = openPipeOnRead("./brakePipe");

    for (;;)
    {
        readline(ecuFileDescriptor, command);
        if (strncmp(command, "FRENO", 5) == 0)
        {
            int speed = atoi(command + 6);
            writeMessage(fileLog, "FRENO %d", speed);
        }
    }

    fclose(fileLog);
    exit(EXIT_SUCCESS);
}

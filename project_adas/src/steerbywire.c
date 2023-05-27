#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "functions.h"

#define PIPE_NAME "./steerPipe"
#define LOG_FILE_NAME "steer.log"

FILE* fileLog;

void handleFailure()
{
    fclose(fileLog);
    exit(EXIT_FAILURE);
}

void handleStop()
{
    writeMessage(fileLog, "ARRESTO AUTO");
    fclose(fileLog);
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
    printf("PROCESSO STEER BY WIRE\n");

    signal(SIGUSR1, handleFailure);
    signal(SIGTSTP, handleStop);

    printf("Tento di aprire il file %s in scrittura\n", LOG_FILE_NAME);
    fileLog = fopen(LOG_FILE_NAME, "w");

    if (fileLog == NULL)
    {
        printf("Errore nell'apertura del file %s\n", LOG_FILE_NAME);
        exit(EXIT_FAILURE);
    }

    printf("File di log aperto correttamente\n");

    int pipeDescriptor;
    int bytesRead;
    char command[16];

    do
    {
        pipeDescriptor = open(PIPE_NAME, O_RDONLY | O_NONBLOCK);
        if (pipeDescriptor == -1)
        {
            printf("Pipe non trovata. Riprova ancora...\n");
            sleep(1);
        }
    } while (pipeDescriptor == -1);

    printf("Pipe %s trovata\n", PIPE_NAME);

    for (;;)
    {
        if ((bytesRead = readline(pipeDescriptor, command)) == -1)
        {
            writeMessage(fileLog, "NO ACTION");
            sleep(1);
        }
        else if (strcmp(command, "SINISTRA") == 0 || strcmp(command, "DESTRA") == 0)
        {
            writeMessage(fileLog, "STO GIRANDO A %s", command);
            sleep(1);
        }
    }

    fclose(fileLog);
    exit(EXIT_SUCCESS);
}

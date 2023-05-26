
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "functions.h"
// new functions
void handleStop();


int pipeArray[2];
FILE *fileLog;
char *command;

int main(void)
{
    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("PROCESSO BRAKE BY WIRE\n");
    signal(SIGTSTP, handleStop);

    printf("Tento di aprire il file brake.log in scrittura\n");
    fileLog = fopen("brake.log", "w");

    if (fileLog ==  NULL)
    {
        printf("Errore nell'apertura del file brake.log\n");
        exit(EXIT_FAILURE);
    }

    printf("File di log aperto correttamente\n");
    
    printf("Faccio una read non bloccante dalla pipe\n");
    int ecuFileDescriptor = openPipeOnRead("./brakePipe");
    char command[16];
    for(;;)
    {
        //printf("Leggo una linea\n");
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

void handleStop() 
{
    writeMessage(fileLog, "ARRESTO AUTO");
}


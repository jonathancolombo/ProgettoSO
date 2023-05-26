//
// Created by jonathan on 09/05/23.
//


#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "functions.h"

FILE* fileLog;
int pipeArray[2];

void handleFailure() {
    fclose(fileLog);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("PROCESSO STEER BY WIRE\n");
    signal(SIGUSR1, handleFailure);

    printf("Tento di aprire il file brake.log in scrittura\n");
    fileLog = fopen("steer.log", "w");

    if (fileLog ==  NULL)
    {
        printf("Errore nell'apertura del file brake.log\n");
        exit(EXIT_FAILURE);
    }

    printf("File di log aperto correttamente\n");
    
    printf("Faccio una read non bloccante su steer by wire\n");
    char command[16];
    int fileDescriptor = openPipeOnRead("./steerPipe");
    int readValue = 0;
    for (;;)
    {
        //printf("Leggo una linea\n");
        if (readValue = readline(fileDescriptor, command) == -1)
        {
            writeMessage(fileLog, "NO ACTION");
            sleep(1);
        }
        else if (strcmp(command, "DESTRA") * strcmp(command, "SINISTRA") == 0)
        {
            writeMessage(fileLog, "STO GIRANDO A %s", command);
            sleep(1);
        } 
    }

    fclose(fileLog);
    wait(NULL);
    exit(EXIT_SUCCESS);
}



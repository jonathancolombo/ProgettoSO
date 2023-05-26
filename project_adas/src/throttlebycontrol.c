//
// Created by jonathan on 09/05/23.
//
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "functions.h"


int deltaSpeed;
FILE* fileLog;
int status;	//pipe status
int pipeArray[2]; //pipe array
pid_t pidChildWriter;


int main(int argc, char* argv[])
{
    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("PROCESSO THROTTLE BY CONTROL\n");

    printf("Tento di aprire il file di log throttle.log\n");
    fileLog = fopen("throttle.log", "w");
        
    if (fileLog == NULL) 
    {
        printf("Errore nell'apertura del file throttle.log\n");
        exit(EXIT_FAILURE);
    }

    printf("File throttle.log aperto correttamente\n");

    int ecuFileDescriptor = openPipeOnRead("./throttlePipe");
    char command[16];

    for (;;)
    {
        readline(ecuFileDescriptor, command);
        if (strncmp(command, "INCREMENTO", 10) == 0)
        {
            int increment = atoi(command + 11);
            writeMessage(fileLog, "AUMENTO %d", increment);
            sleep(1);
        } 
        else
        {
            writeMessage(fileLog, "NO ACTION");
            sleep(1);
        }
    }

    fclose(fileLog);
    exit(EXIT_SUCCESS);
}

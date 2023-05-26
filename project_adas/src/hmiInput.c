
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "functions.h"

int fileDescriptor;

void signalHandler();

int main(int argc, char *argv[]) {
    char command[32];
    int pid = getpid();

    signal(SIGINT, signalHandler);

    printf("HMI Input system initialized\n\n");
    fileDescriptor = createPipe("./hmiInputToEcuPipe");
    write(fileDescriptor, &pid, sizeof(int));
    while(1) 
    {
        scanf("%s", &command);
        if(strcmp(command, "INIZIO") == 0) 
        {
            printf("Veicolo avviato\n");
            write(fileDescriptor, command, strlen(command)+1);
        } else if(strcmp(command, "PARCHEGGIO") == 0) {
            printf("Parcheggio avviato\n");
            write(fileDescriptor, command, strlen(command)+1);
        } else if(strcmp(command, "ARRESTO") == 0) {
            write(fileDescriptor, command, strlen(command)+1);
        } else {
            printf("Command not found. Please try again\n");
        }
    }
}

void signalHandler() 
{
    close(fileDescriptor);
    unlink("./hmiInputToEcuPipe");
    exit(EXIT_SUCCESS);
}
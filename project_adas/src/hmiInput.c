
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

int fileDescriptor;

void signalHandler();
int createPipe(char *pipeName);

int main(int argc, char *argv[]) {
    char command[32];
    int pid = getpid();

    signal(SIGINT, signalHandler);

    printf("HMI Input system initialized\n\n");
    fileDescriptor = createPipe("../../ipc/hmiInputToEcuPipe");
    write(fileDescriptor, &pid, sizeof(int));
    while(1) 
    {
        printf("Inserisci un comando per avviare il sistema: INIZIO, PARCHEGGIO o ARRESTO\n:\n");
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
    unlink("../../ipc/hmiInputToEcuPipe");
    exit(EXIT_SUCCESS);
}

int createPipe(char *pipeName) 
{
    int fileDescriptor;
    unlink(pipeName);
    if (mknod(pipeName, __S_IFIFO, 0) < 0 )
    {    //Creating named pipe
        exit(EXIT_FAILURE);
    }
    chmod(pipeName, 0660);
    do {
        fileDescriptor = open(pipeName, O_WRONLY);    //Opening named pipe for write
        if(fileDescriptor == -1)
        {
            printf("%s not found. Trying again...\n", pipeName);
            sleep(1);
        }
    } while(fileDescriptor == -1);
    return fileDescriptor;
}
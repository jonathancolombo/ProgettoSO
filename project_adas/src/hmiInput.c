
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
    int pid = getpid();
    signal(SIGINT, signalHandler);
    
    printf("HMI Input inizializzato\n\n");
    fileDescriptor = createPipe("./hmiInputToEcuPipe");
    write(fileDescriptor, &pid, sizeof(int));
    
    while (1) {
        char command[32];
        printf("Input possibili: INIZIO, PARCHEGGIO, ARRESTO:\n");
        scanf("%s", command);
        getchar();  // Pulizia del buffer di input

        if (strcmp(command, "INIZIO") == 0) {
            printf("Veicolo avviato\n");
            if (write(fileDescriptor, command, strlen(command) + 1) == -1) {
                perror("Errore durante la scrittura sul pipe");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(command, "PARCHEGGIO") == 0) {
            printf("Parcheggio avviato\n");
            if (write(fileDescriptor, command, strlen(command) + 1) == -1) {
                perror("Errore durante la scrittura sul pipe");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(command, "ARRESTO") == 0) {
            printf("Arresto avviato\n");
            if (write(fileDescriptor, command, strlen(command) + 1) == -1) {
                perror("Errore durante la scrittura sul pipe");
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Command not found. Please try again\n");
        }
    }

    exit(EXIT_SUCCESS);
}

void signalHandler() {
    close(fileDescriptor);
    unlink("./hmiInputToEcuPipe");
    exit(EXIT_SUCCESS);
}

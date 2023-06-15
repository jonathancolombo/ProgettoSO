#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "commonFunctions.h"

int ecuProcessID;
int terminatedProcessesCount;
int ecuToHmiPipeFD;

// Funzione per conteggiare i processi terminati
void handleTermination() {
    terminatedProcessesCount++;
    if (terminatedProcessesCount >= 2) {
        close(ecuToHmiPipeFD);
        exit(EXIT_SUCCESS);
    }
}

// Funzione per leggere dalla pipe
int readFromPipe(int fd) {
    char str[256];
    if (readline(fd, str) < 0) {
        return -1;
    }
    printf("%s\n", str);
    return 0;
}

// Funzione per gestire l'errore di accelerazione
void handleAccelerationError() {
    printf("ERRORE ACCELERAZIONE\nTERMINAZIONE PROGRAMMA...");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, handleTermination);
    signal(SIGUSR1, handleAccelerationError);
    terminatedProcessesCount = 0;

    // Controllo degli argomenti passati al programma
    if (argc != 2 || (strcmp(argv[1], "NORMALE") != 0 && strcmp(argv[1], "ARTIFICIALE") != 0)) {
        printf("Argomenti non validi. Riprovare\n");
        exit(EXIT_FAILURE);
    }

    // Apertura del terminale per l'input dell'HMI
    system("gnome-terminal -- ./hmiInput");

    // Creazione del processo ECU
    if ((ecuProcessID = fork()) < 0) {
        exit(EXIT_FAILURE);
    } else if (ecuProcessID == 0) {
        execl("./ecu", "./ecu", argv[1], NULL); // Esecuzione del processo ECU
    }

    printf("HMI Output inizializzato\n\n");
    int n;
    ecuToHmiPipeFD = openPipeOnRead("./ecuToHmiPipe");
    printf("Nome pipe trovato\n\n");
    while (1) {
        while (readFromPipe(ecuToHmiPipeFD) < 0);
    }
}

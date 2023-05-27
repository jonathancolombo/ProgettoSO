#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "functions.h"

FILE *fileEcuLog;
int ecuProcessPID;
int terminated;
int fileDescriptor;

void countTerminated() {
    terminated++;
    if (terminated >= 2) {
        close(fileDescriptor);
        exit(EXIT_SUCCESS);
    }
}

void failureHandler() {
    fprintf(stderr, "ERRORE ACCELERAZIONE\nTERMINAZIONE PROGRAMMA...\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, countTerminated);
    signal(SIGUSR1, failureHandler);
    terminated = 0;

    /*
     * Controlla la tipologia di AVVIO impostata e restituisce un errore
     */
    if (argc != 2 || (strcmp(argv[1], "NORMALE") != 0 && strcmp(argv[1], "ARTIFICIALE") != 0)) {
        fprintf(stderr, "Inserisci il comando di avvio NORMALE o ARTIFICIALE\n");
        exit(EXIT_FAILURE);
    }

    printf("Eseguo il comando 'gnome-terminal' per avviare 'hmiInput'\n");
    system("gnome-terminal -- ./hmiInput");

    // Fork del processo ECU
    ecuProcessPID = fork();

    // Controllo se il processo ECU è stato creato correttamente
    if (ecuProcessPID < 0) {
        fprintf(stderr, "Errore durante la fork del processo figlio\n");
        perror("fork error\n");
        exit(EXIT_FAILURE);
    } else if (ecuProcessPID == 0) {
        // Sono il processo figlio
        printf("Sono il processo figlio\n");
        printf("Eseguo l'execv dal processo figlio\n");
        char *args[] = {"./ecu", NULL};
        execv(args[0], args);
        // Se l'esecuzione raggiunge questo punto, c'è stato un errore nell'execv
        fprintf(stderr, "Errore durante l'esecuzione di 'ecu'\n");
        perror("execv error\n");
        exit(EXIT_FAILURE);
    }

    printf("HMI Output system initialized\n\n");
    fileDescriptor = openPipeOnRead("./ecuToHmiPipe");
    printf("Pipe trovata.\n\n");
    printf("Inizio a leggere dalla pipe\n");

    for (;;) {
        while (readFromPipe(fileDescriptor) < 0);
    }

    exit(EXIT_SUCCESS);
}

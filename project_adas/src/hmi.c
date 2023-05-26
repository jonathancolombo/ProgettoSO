
// LIBRERIE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

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
    printf("ERRORE ACCELERAZIONE\nTERMINAZIONE PROGRAMMA...\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    signal(SIGCHLD, countTerminated);
    signal(SIGUSR1, failureHandler);
    terminated = 0;
    /*
     * Controlla la tipologia di AVVIO impostata e restituisce un errore
     */
    if ((argc < 2) || (strcmp(argv[1], "ARTIFICIALE") != 0 && strcmp(argv[1], "NORMALE") != 0))
    {
        printf("Inserisci il comando di avvio NORMALE o ARTIFICIALE\n");
        exit(EXIT_FAILURE);
    }

    printf("Eseguo la system per la gnome-terminal");

    system("gnome-terminal -- ./hmiInput");
    // fork del processo ECU
    ecuProcessPID = fork();
    // controllo se il processo ECU è stato creato correttamente
    if (ecuProcessPID < 0)
    {
        printf("Errore della fork sul processo figlio\n");
        perror("fork error\n");
        exit(EXIT_FAILURE);
    }
    else if (ecuProcessPID == 0)
    {
        // imposto il pgid ed eseguo una execv
        printf("Sono il processo figlio\n");
        printf("Eseguo la exec dal processo figlio\n");
        //printf("argv[0]: %s, argv: %s\n", argv[1], argv);
        execl("./ecu", "./ecu", NULL);
        //execv(argv[0], argv);
        //execl("../control/centralECU", "./centralECU", argv[1], 0);
    }

    printf("HMI Output system initialized\n\n");
    int n;
    fileDescriptor = openPipeOnRead("./ecuToHmiPipe");
    printf("Pipe trovata.\n\n");
    printf("Inizio a leggere dalla pipe\n");
    for (;;)
    {
        while (readFromPipe(fileDescriptor) < 0);
    }

    exit(EXIT_SUCCESS);
}

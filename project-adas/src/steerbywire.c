#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "commonFunctions.h"

FILE *steerLog;

void handleFailure() {
    fclose(steerLog);
    exit(EXIT_FAILURE);
}

int openSteerPipe() {
    int pipeFd;

    do {
        // Apertura della pipe in modalità di sola lettura e non bloccante
        pipeFd = open("./steerPipe", O_RDONLY | O_NONBLOCK);
        if (pipeFd == -1) {
            printf("Pipe non trovata. Riprovo...\n");
            sleep(1);
        }
    } while (pipeFd == -1);

    return pipeFd;
}

void processSteerDirection(int pipeFd) {
    char steerDirection[16];
    int bytesRead;

    while (1) {
        // Lettura dalla pipe
        bytesRead = readline(pipeFd, steerDirection);
        if (bytesRead == -1) {
            writeMessage(steerLog, "NO ACTION");
            sleep(1);
        } else if (strcmp(steerDirection, "SINISTRA") == 0 || strcmp(steerDirection, "DESTRA") == 0) {
            writeMessage(steerLog, "STO GIRANDO A %s", steerDirection);
            sleep(1);
        }
    }
}

int main(int argc, char *argv[]) {
    int pipeFd;

    // Impostazione del gestore del segnale di fallimento
    signal(SIGUSR1, handleFailure);

    // Creazione del file di log
    createLog("./steer", &steerLog);

    // Apertura della pipe
    pipeFd = openSteerPipe();

    // Elaborazione delle direzioni del volante
    processSteerDirection(pipeFd);

    // Chiusura del file di log e terminazione del programma
    fclose(steerLog);
    wait(NULL);
    exit(EXIT_SUCCESS);
}

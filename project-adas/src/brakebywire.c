#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "commonFunctions.h"

FILE *brakeLog;

void handleStop() {
    writeMessage(brakeLog, "ARRESTO AUTO");
}

void handleFailure() {
    fclose(brakeLog);
    exit(EXIT_FAILURE);
}

void setupSignalHandlers() {
    // Imposta i gestori dei segnali
    signal(SIGTSTP, handleStop);
    signal(SIGUSR1, handleFailure);
}

void closeResourcesAndExit(int exitCode) {
    fclose(brakeLog);
    exit(exitCode);
}

int openBrakePipe() {
    int ecuFd = openPipeOnRead("./brakePipe");
    if (ecuFd < 0) {
        printf("Impossibile aprire la pipe\n");
        closeResourcesAndExit(EXIT_FAILURE);
    }
    return ecuFd;
}

void processCommand(const char* command) {
    int decrement;
    if (strncmp(command, "FRENO", 5) == 0) {
        decrement = atoi(command + 6);
        writeMessage(brakeLog, "FRENO %d", decrement);
    }
}

int main(int argc, char *argv[]) {
    setupSignalHandlers();

    // Crea il file di log
    if (createLog("./brake", &brakeLog) != 0) {
        printf("Impossibile creare il file di log\n");
        closeResourcesAndExit(EXIT_FAILURE);
    }

    // Apri la pipe
    int ecuFd = openBrakePipe();

    // Loop principale
    char str[16];
    while (1) 
    {
        // Leggi dalla pipe
        if (readline(ecuFd, str) == -1) {
            printf("Errore nella lettura dalla pipe\n");
            break;
        }

        // Processa il comando
        processCommand(str);
    }

    closeResourcesAndExit(EXIT_SUCCESS);
}

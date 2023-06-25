#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h> 
#include <unistd.h>

#include "utility.h"

FILE *brakeLog; // Puntatore al file di log per il sistema frenante dell'auto

// Gestisce il segnale di arresto dell'auto
void handleStop() {
    writeMessage(brakeLog, "ARRESTO AUTO");
}

// Gestisce il segnale di errore
void handleFailure() {
    fclose(brakeLog);
    exit(EXIT_FAILURE);
}

// Imposta i gestori dei segnali
void setupSignalHandlers() {
    signal(SIGTSTP, handleStop);
    signal(SIGUSR1, handleFailure);
}

// Chiude il file di log e termina il programma con il codice di uscita specificato
void closeResourcesAndExit(int exitCode) {
    fclose(brakeLog);
    exit(exitCode);
}

// Apre la pipe in modalità di sola lettura
int openBrakePipe() {
    int ecuFd = openPipeOnRead("./brakePipe");
    if (ecuFd < 0) {
        printf("Impossibile aprire la pipe\n");
        closeResourcesAndExit(EXIT_FAILURE);
    }
    return ecuFd;
}

// Processa il comando ricevuto
void processCommand(const char* command) {
    if (strncmp(command, "FRENO", 5) == 0) {
        int decrement = atoi(command + 6); // Estrae il valore del decremento dal comando
        writeMessage(brakeLog, "FRENO %d", decrement);
    }
}

int main(int argc, char *argv[]) {
    setupSignalHandlers(); // Imposta i gestori dei segnali

    // Crea il file di log per il sistema frenante dell'auto
    if (createLog("./brake", &brakeLog) != 0) {
        printf("Impossibile creare il file di log\n");
        closeResourcesAndExit(EXIT_FAILURE);
    }

    // Apri la pipe per la comunicazione con l'ECU (Unità di Controllo)
    int ecuFd = openBrakePipe();

    // Loop principale
    char str[16];
    while (1) {
        // Leggi un comando dalla pipe
        if (readline(ecuFd, str) == -1) {
            printf("Errore nella lettura dalla pipe\n");
            break;
        }

        // Processa il comando ricevuto
        processCommand(str);
    }

    closeResourcesAndExit(EXIT_SUCCESS);
}

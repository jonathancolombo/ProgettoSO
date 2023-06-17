#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#include "utility.h"

FILE *throttleLog;

// Funzione per gestire l'incremento
void handleIncrement(FILE *randomFile, FILE *throttleLog, int ecuFileDescriptor) {
    int increment;
    int randomNumber;
    char inputString[16];

    // Leggi dalla pipe
    if (readline(ecuFileDescriptor, inputString) == -1) {
        printf("Errore nella lettura dalla pipe\n");
        return;
    }

    // Controlla se la stringa inizia con "INCREMENTO"
    if (strncmp(inputString, "INCREMENTO", 10) == 0) {
        increment = atoi(inputString + 11);
        writeMessage(throttleLog, "AUMENTO %d", increment);

        // Leggi un numero casuale dal file
        while (fread(&randomNumber, sizeof(int), 1, randomFile) < 0) {
            // Attendi fino a quando i numeri casuali non possono essere letti
        }

        // Calcola un numero casuale compreso tra 0 e 99999
        randomNumber = randomNumber % 100000;

        // Se il numero casuale è 0, invia un segnale al processo padre e termina con un errore
        if (randomNumber == 0) {
            kill(getppid(), SIGUSR1);
            exit(EXIT_FAILURE);
        }
    }
}

void closeResourcesAndExit(int exitCode, FILE *randomFile, FILE *throttleLog) {
    fclose(randomFile);
    fclose(throttleLog);
    exit(exitCode);
}

int openThrottlePipe() {
    int ecuFileDescriptor = openPipeOnRead("./throttlePipe");
    if (ecuFileDescriptor < 0) {
        printf("Impossibile aprire la pipe\n");
        closeResourcesAndExit(EXIT_FAILURE, NULL, throttleLog);
    }
    return ecuFileDescriptor;
}

FILE *openRandomFile(const char *randomName) {
    FILE *randomFile = fopen(randomName, "r");
    if (randomFile == NULL) {
        printf("Impossibile aprire il file di numeri casuali: %s\n", randomName);
        closeResourcesAndExit(EXIT_FAILURE, NULL, throttleLog);
    }
    return randomFile;
}

void handleFailure() {
    fclose(throttleLog);
    exit(EXIT_FAILURE);
}

void setupSignalHandlers() {
    // Imposta i gestori dei segnali
    signal(SIGUSR1, handleFailure);
}

int main(int argc, char *argv[]) {
    char randomName[128];
    FILE *randomFile;
    int ecuFileDescriptor;

    // Controllo degli argomenti passati al programma
    if (argc < 2) {
        printf("Specificare il tipo di nome come argomento: NORMALE o ARTIFICIALE\n");
        return EXIT_FAILURE;
    }

    // Creazione del nome del file casuale in base all'argomento
    if (strcmp(argv[1], "NORMALE") == 0) {
        snprintf(randomName, sizeof(randomName), "/dev/random");
    } else if (strcmp(argv[1], "ARTIFICIALE") == 0) {
        snprintf(randomName, sizeof(randomName), "./randomARTIFICIALE.binary");
    } else {
        printf("Errore! Utilizzare NORMALE o ARTIFICIALE\n");
        return EXIT_FAILURE;
    }

    // Apertura del file casuale
    randomFile = openRandomFile(randomName);

    // Creazione del file di log
    if (createLog("./throttle", &throttleLog) != 0) {
        printf("Impossibile creare il file di log\n");
        fclose(randomFile);
        return EXIT_FAILURE;
    }

    // Imposta il gestore dei segnali
    setupSignalHandlers();

    // Apertura della pipe
    ecuFileDescriptor = openThrottlePipe();

    // Loop principale
    while (1) {
        handleIncrement(randomFile, throttleLog, ecuFileDescriptor);
    }

    // Chiusura dei file e terminazione del programma
    closeResourcesAndExit(EXIT_SUCCESS, randomFile, throttleLog);
}

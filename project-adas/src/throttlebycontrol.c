#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h> // Aggiunto l'inclusione per sys/types.h
#include "commonFunctions.h"

FILE *throttleLog;

int main(int argc, char *argv[]) {
    int ecuFd;
    char str[16];
    char randName[128];
    FILE *rnd;
    int increment;
    int randomNum;
    
    if (argc < 2) {
        printf("Specificare il tipo di nome casuale come argomento: NORMALE o ARTIFICIALE\n");
        return 1;
    }

    if (strcmp(argv[1], "NORMALE") == 0) {
        snprintf(randName, sizeof(randName), "/dev/random");
    } else if (strcmp(argv[1], "ARTIFICIALE") == 0) {
        snprintf(randName, sizeof(randName), "./randomARTIFICIALE.binary");
    } else {
        printf("Tipo di nome casuale non valido. Utilizzare NORMALE o ARTIFICIALE\n");
        return 1;
    }

    rnd = fopen(randName, "r");
    if (rnd == NULL) {
        printf("Impossibile aprire il file di numeri casuali: %s\n", randName);
        return 1;
    }

    if (createLog("./throttle", &throttleLog) != 0) {
        printf("Impossibile creare il file di log\n");
        fclose(rnd);
        return 1;
    }

    ecuFd = openPipeOnRead("./throttlePipe");
    if (ecuFd < 0) {
        printf("Impossibile aprire la pipe\n");
        fclose(rnd);
        fclose(throttleLog);
        return 1;
    }

    while (1) {
        if (readline(ecuFd, str) == -1) {
            printf("Errore nella lettura dalla pipe\n");
            break;
        }
        if (strncmp(str, "INCREMENTO", 10) == 0) {
            increment = atoi(str + 11);
            writeMessage(throttleLog, "AUMENTO %d", increment);
            while (fread(&randomNum, sizeof(int), 1, rnd) < 0) {
                // Attendi fino a quando i numeri casuali non possono essere letti
            }
            randomNum = randomNum % 100000;
            if (randomNum == 0) {
                kill(getppid(), SIGUSR1);
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(rnd);
    fclose(throttleLog);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>

#include "commonFunctions.h"
#include "socketFunctions.h"

FILE *sensorLog;

// Funzione per ottenere il nome del file di urandom in base all'argomento passato
void getUrandomName(const char *arg, char *urandName) {
    if (strcmp(arg, "NORMALE") == 0) {
        snprintf(urandName, 128, "/dev/urandom");
    } else if (strcmp(arg, "ARTIFICIALE") == 0) {
        snprintf(urandName, 128, "./urandomARTIFICIALE.binary");
    }
}

// Funzione per inviare l'ID del sensore all'ECU
void sendSensorID(int socket, int sensorID) {
    if (send(socket, &sensorID, sizeof(sensorID), 0) < 0) {
        perror("Errore durante l'invio dell'ID del sensore");
        exit(EXIT_FAILURE);
    }
}

// Funzione per ricevere lo stato di ascolto dell'ECU
int receiveListeningStatus(int socket, int sensorID) {
    int isListening;
    if (recv(socket, &isListening, sizeof(sensorID), 0) < 0) {
        perror("Errore durante la ricezione dello stato di ascolto");
        exit(EXIT_FAILURE);
    }
    return isListening;
}

// Funzione per inviare i dati del sensore all'ECU
void sendSensorData(FILE *urand, int socket, FILE *log) {
    char data[8];
    int count = 0;

    while (count < 30) {
        for (int i = 0; i < 4; i++) {
            int c1 = fgetc(urand);
            int c2 = fgetc(urand);
            snprintf(data, 8, "0x%02x%02x", c1, c2);
            if (send(socket, data, strlen(data) + 1, 0) < 0) {
                perror("Errore durante l'invio dei dati del sensore");
                exit(EXIT_FAILURE);
            }
            writeMessage(log, "%s", data);
        }
        count++;
        sleep(1);
    }
}

int main(int argc, char *argv[]) {
    // Creazione del file di log del sensore
    if ((sensorLog = fopen("./assist.log", "a")) == NULL) {
        perror("Impossibile aprire il file di log");
        exit(EXIT_FAILURE);
    }

    // Creazione del nome del file di urandom
    char urandName[128];
    getUrandomName(argv[1], urandName);

    // Apertura del file di urandom
    FILE *urand = fopen(urandName, "rb");
    if (urand == NULL) {
        perror("Impossibile aprire il file di urandom");
        fclose(sensorLog);
        exit(EXIT_FAILURE);
    }

    const int sensorID = 1; // Valore per essere riconosciuto dal socket

    int ecuSocket, ecuAddrLen;
    struct sockaddr_un ecuUNIXAddress;
    struct sockaddr *ecuSockAddrPtr = (struct sockaddr *) &ecuUNIXAddress;

    // Autenticazione e connessione al socket dell'ECU
    socketAuth(&ecuSocket, &ecuUNIXAddress, &ecuAddrLen, "./ecuSocket");
    connectServer(ecuSocket, ecuSockAddrPtr, ecuAddrLen);

    // Invio dell'ID del sensore all'ECU
    sendSensorID(ecuSocket, sensorID);

    // Ricezione dello stato di ascolto dell'ECU
    int isListening = receiveListeningStatus(ecuSocket, sensorID);

    // Invio dei dati del sensore all'ECU
    sendSensorData(urand, ecuSocket, sensorLog);

    // Chiusura dei file e terminazione del programma
    fclose(urand);
    fclose(sensorLog);
    exit(EXIT_SUCCESS);
}

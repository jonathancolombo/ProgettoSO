#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

#define DEFAULT_PROTOCOL 0

#include "utility.h"
#include "utilitySocket.h"

FILE *cameraLog;
int cameraDataFileDescriptor;
int ecuSocketFileDescriptor;

// Gestore del segnale di fallimento
void handleFailure(int signal) {
    close(cameraDataFileDescriptor);
    fclose(cameraLog);
    exit(EXIT_FAILURE);
}

// Gestore del segnale di stop
void stopHandler(int signal) {
    close(ecuSocketFileDescriptor);
}

// Inizializzazione della fotocamera
void initializeCamera() {
    createLog("./camera", &cameraLog);

    // Impostazione dei gestori dei segnali
    signal(SIGUSR1, handleFailure);
    signal(SIGTSTP, stopHandler);

    cameraDataFileDescriptor = open("./frontCamera.data", O_RDONLY);
}

// Invia i dati all'ECU
void sendDataToEcu(const char *data) {
    const int cameraID = 0; // Identificatore per la camera
    int isEcuListening;

    int ecuAddressLength;
    struct sockaddr_un ecuUNIXAddress;
    struct sockaddr *ecuSocketAddressPtr = (struct sockaddr *)&ecuUNIXAddress;

    // Autenticazione e connessione all'ECU tramite socket
    socketAuth(&ecuSocketFileDescriptor, &ecuUNIXAddress, &ecuAddressLength, "./ecuSocket");
    connectServer(ecuSocketFileDescriptor, ecuSocketAddressPtr, ecuAddressLength);

    // Invio dell'ID della camera
    while (send(ecuSocketFileDescriptor, &cameraID, sizeof(cameraID), 0) < 0)
        ;
    // Ricezione dello stato dell'ECU
    while (recv(ecuSocketFileDescriptor, &isEcuListening, sizeof(cameraID), 0) < 0)
        ;

    // Se l'ECU è in ascolto, invia i dati e registra nel file di log della fotocamera
    if (isEcuListening == 1) {
        send(ecuSocketFileDescriptor, data, strlen(data) + 1, 0);
        writeMessage(cameraLog, "%s", data);
    }

    close(ecuSocketFileDescriptor);
}

int main(int argc, char *argv[]) {
    initializeCamera();

    char line[16];
    int i = 0;

    while (1) {
        // Lettura di una riga di dati dalla fotocamera
        memset(line, '\0', 16);
        i = 0;
        while (read(cameraDataFileDescriptor, &line[i], 1) < 0)
            ;
        while (line[i] != '\n' && line[i] != EOF) {
            i++;
            while (read(cameraDataFileDescriptor, &line[i], 1) < 0)
                ;
        }
        char temp = line[i];
        line[i] = '\0';

        // Invio dei dati all'ECU
        sendDataToEcu(line);

        // Controllo se è stata raggiunta la fine del file di dati della fotocamera
        if (temp == EOF) {
            fclose(cameraLog);
            exit(EXIT_SUCCESS);
        }
        sleep(1);
    }
}

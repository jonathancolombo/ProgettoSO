#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <signal.h>

#include "commonFunctions.h"
#include "socketFunctions.h"

FILE *sensorLog;

// Funzione per gestire il fallimento del sensore
void handleFailure() {
    fclose(sensorLog);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    // Imposta il gestore per il segnale di fallimento
    signal(SIGUSR1, handleFailure);

    // Creazione del file di log del sensore
    createLog("./radar", &sensorLog);

    // Scrittura del messaggio di avvio del sensore nel log
    writeMessage(sensorLog, "SENSOR LAUNCHED");

    // Chiusura del file di log
    fclose(sensorLog);

    // Attende la terminazione dei processi figli
    wait(NULL);

    // Terminazione del programma con successo
    exit(EXIT_SUCCESS);
}

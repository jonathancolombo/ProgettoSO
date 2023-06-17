#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */

#include "utility.h"
#include "utilitySocket.h"

// Funzione per gestire il lancio del sensore della fotocamera
void launchCameraSensor() {
    FILE *log;

    // Creazione del file di log
    createLog("./cameras", &log);

    // Scrittura del messaggio di avvio del sensore
    writeMessage(log, "Sensore avviato");

    // Chiusura del file di log
    fclose(log);
}

int main(int argc, char *argv[]) {
    // Lancio del sensore della fotocamera
    launchCameraSensor();

    // Attende la terminazione dei processi figli
    wait(NULL);

    // Terminazione del programma con successo
    exit(EXIT_SUCCESS);
}

//
// Created by jonathan on 09/05/23.
//
/*
Facoltativo. Surround view cameras: è un processo che agisce solo
quando Park assist è attivo. Legge i dati e li invia a Park Assist:
– Finchè Park Assist è Attivo, 1 volta al secondo, legge 8 byte da
/dev/urandom, li invia a park assist, e logga i dati inviati.
– Suggerimento: Surround view cameras può essere un figlio di Park Assist,
creato e terminato al bisogno

 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>


#define MAX_BYTE 8

// Variables
FILE *fileRead;
FILE *fileLog;
int socketFd;
int stop = 0;
unsigned char buffer[MAX_BYTE];

// Functions
void start(char **mode);
void readAndLog();
void writeOnLog();
void sigStartHandler();
void sigStopHandler();
void sigTermHandler();
void openFile(char filename[], char mode[], FILE **filePointer);
int createConnection(char *socketName);

int main (int argc, char *argv[]) {
    signal(SIGUSR1,sigStartHandler);
    signal(SIGTERM,sigTermHandler);
    start(&argv[1]);
}

void start(char **mode) {
    if (strcmp(*mode, "NORMALE") == 0) {
        openFile("/dev/urandom", "rb", &fileRead);
    } else {
        openFile("urandomARTIFICIALE.binary", "rb", &fileRead);
    }
    openFile("camera.log","w",&fileLog);
    while((socketFd = createConnection("svcSocket")) < 0)
        usleep(100000);
    pause();
}

void writeOnLog() {
    for (int c = 0; c < MAX_BYTE; c++) {
        fprintf(fileLog, "%.2X", buffer[c]);
    }
    fprintf(fileLog, "\n");
}

void readAndLog() {
    int qtaRead = 0;
    while (stop == 0){
        qtaRead = fread(buffer,1,MAX_BYTE,fileRead);
        if(qtaRead == 0)
            break;
        //sendToSocket(socketFd, buffer);
        write(socketFd, buffer, strlen(buffer)+1);
        writeOnLog();
        fflush(fileLog);
        sleep(1);
    }
    fprintf(fileLog, "\n");
    stop = 0;
}

void sigStartHandler() {
    signal(SIGUSR1,sigStopHandler); // inizializza gestione di stop
    readAndLog(); // avvia il log
}

void sigStopHandler() {
    signal(SIGUSR1,sigStartHandler); // inizializza la gestione di start
    stop = 1;
}

void sigTermHandler() {
    signal(SIGTERM,SIG_DFL);
    fclose(fileRead);
    fclose(fileLog);
    kill(getpid(),SIGTERM);
}

void openFile(char filename[], char mode[], FILE **filePointer) {
    *filePointer = fopen(filename, mode);
    if (*filePointer == NULL) {
        printf("Errore nell'apertura del file");
        exit(1);
    }
}

int createConnection(char *socketName){
    int socketFd, serverLen;
    struct sockaddr_un serverUNIXAddress;
    struct sockaddr* serverSockAddrPtr;
    serverSockAddrPtr = (struct sockaddr*) &serverUNIXAddress;
    serverLen = sizeof (serverUNIXAddress);
    socketFd = socket (AF_UNIX, SOCK_STREAM, 0);
    serverUNIXAddress.sun_family = AF_UNIX; /* Server domain */
    strcpy (serverUNIXAddress.sun_path, socketName);/*Server name*/
    int result = connect(socketFd, serverSockAddrPtr, serverLen);
    if(result < 0){
        return result;
    }
    return socketFd;
}


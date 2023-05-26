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
FILE *fileCamerasLog;
int socketFd;
int stop = 0;
unsigned char buffer[MAX_BYTE];

// Functions
void startProcess(char **mode);
void readAndLog();
void writeOnLog();
void sigStartHandler();
void sigStopHandler();
void sigTermHandler();
int createConnection(char *socketName);

int main (int argc, char *argv[]) 
{
    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("PROCESSO SURROUND VIEW CAMERA\n");
    signal(SIGUSR1,sigStartHandler);
    signal(SIGTERM,sigTermHandler);
    startProcess(&argv[1]);
}

void startProcess(char **mode) {
    if (strcmp(*mode, "NORMALE") == 0) 
    {
        printf("Tento di aprire il file dev/urandom\n");
        fileRead = fopen("dev/urandom", "rb");
        if (fileRead == NULL)
        {
            printf("Impossibile aprire il file dev/urandom\n");
            return EXIT_FAILURE;
        }
        printf("Lettura da dev/urandom avviata\n");
    }
    else 
    {
        printf("Tento di aprire il file urandomARTIFICIALE\n");
        fileRead = fopen("urandomARTIFICIALE.binary", "rb");
        if (fileRead == NULL)
        {
            printf("Impossibile aprire il file urandomARTIFICIALE.binary\n");
            return EXIT_FAILURE;
        }
        printf("Lettura da urandomARTIFICIALE.binary avviata\n");
    }

    printf("Tento di aprire il file cameras.log\n");
    fileCamerasLog = fopen("cameras.log", "w");

    if (fileCamerasLog == NULL)
    {
        printf("Impossibile aprire il file cameras.log\n");
        return EXIT_FAILURE;
    }

    printf("Cameras.log aperto in modalità lettura\n");

    while((socketFd = createConnection("./ecuSocket")) < 0)
    {
        sleep(1);
    }
    pause();
}

void writeOnLog() {
    printf("Scrivo sul file di log cameras.log \n");
    for (int c = 0; c < MAX_BYTE; c++) {
        fprintf(fileCamerasLog, "%.2X", buffer[c]);
    }
    fprintf(fileCamerasLog, "\n");
}

void readAndLog() {
    int qtaRead = 0;
    while (stop == 0)
    {
        qtaRead = fread(buffer,1,MAX_BYTE, fileRead);
        if(qtaRead == 0)
        {
            printf("Dati non sufficienti letti\n");
            break;
        }
        //sendToSocket(socketFd, buffer);
        write(socketFd, buffer, strlen(buffer)+1);
        writeOnLog();
        fflush(fileCamerasLog);
        sleep(1);
    }
    fprintf(fileCamerasLog, "\n");
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
    fclose(fileCamerasLog);
    kill(getpid(),SIGTERM);
}


int createConnection(char *socketName)
{
    printf("Inizializzo la socket\n");
    int socketFd, serverLen;
    struct sockaddr_un serverUNIXAddress;
    struct sockaddr* serverSockAddrPtr;
    serverSockAddrPtr = (struct sockaddr*) &serverUNIXAddress;
    serverLen = sizeof (serverUNIXAddress);
    socketFd = socket (AF_UNIX, SOCK_STREAM, 0);
    serverUNIXAddress.sun_family = AF_UNIX; /* Server domain */
    strcpy (serverUNIXAddress.sun_path, socketName);/*Server name*/
    int result = connect(socketFd, serverSockAddrPtr, serverLen);
    if(result < 0)
    {
        return result;
    }
    printf("Restituisco il file descriptor\n");
    return socketFd;
}


//
// Created by jonathan on 09/05/23.
//

/*
 * Componente forward facing radar (facoltativo). Il componente iterativamente legge i dati da
una sorgente e li invia alla Central ECU. In dettaglio, ogni 1 secondo prova a leggere 8 byte da
/dev/urandom. Se riesce a leggere 8 byte, questi sono trasmessi alla Central ECU. Altrimenti (se legge meno
di 8 byte), non invia dati alla Central ECU. I dati inviati sono registrati nel file di log radar.log.


 IMPLEMENTAZIONE DA MODIFICARE
    sensore ​Forward Racing Radar ​quando viene creato dalla ECU si connette alla ​socket​, e ogni 2
secondi prova a leggere 24 byte da ​/dev/random​. Se riesce a leggere correttamente 24 byte, li
trasmette alla ECU e li scrive nel file di log, altrimenti non invia niente.

 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>

void sigTermHandler();
void openFile(char filename[], char mode[], FILE **filePointer);
int createConnection(char *socketName);

FILE *filePointer;
FILE *logFilePointer;
int socketFileDescriptor = 0;

int main(int argc, char **argv)
{
    signal(SIGTERM, sigTermHandler);
    if (strcmp(argv[1], "NORMALE") == 0)
    {
        openFile("/dev/random", "rb", &filePointer);
    }
    else
    {
        // avvia lettura da ARTIFICIALE
        openFile("randomARTIFICIALE.binary", "rb", &filePointer);
    }
    openFile("radar.log", "w", &logFilePointer);

    while((socketFileDescriptor = createConnection("ffrSocket")) < 0)
        usleep(100000); //0.1 sec
    int elementsRead;
    unsigned char buffer[24];
    while(1)
    {
        elementsRead = fread(buffer, 1, 8, filePointer);
        if (elementsRead == 8) {
            write(socketFileDescriptor, buffer, strlen(buffer)+1);
            for (int c = 0; c < 8; c++)
            {
                fprintf(logFilePointer, "%.2X", buffer[c]);
            }
            fprintf(logFilePointer, "\n");
            fflush(logFilePointer);
        }
        sleep(2);
    }

}


void sigTermHandler() {
    fclose(filePointer);
    fclose(logFilePointer);
    exit(0);
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


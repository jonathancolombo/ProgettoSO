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

FILE *fileRead;
FILE *fileRadarLog;
int socketFileDescriptor = 0;
unsigned char buffer[24];

int main(int argc, char **argv)
{
    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("PROCESSO FORWARD FACING RADAR\n");
    signal(SIGTERM, sigTermHandler);
    if (strcmp(argv[1], "NORMALE") == 0)
    {
        printf("Tento di aprire il file dev/urandom in lettura\n");
        fileRead = fopen("/dev/random","rb");
        if (fileRead == NULL)
        {
            printf("Errore nell'apertura del file /dev/urandom\n");
            exit(EXIT_FAILURE);
        }
        printf("File /dev/random aperto correttamente\n");
    }
    else
    {
        printf("Tento di aprire il file urandomARTIFICIALE.binary in lettura\n");
        fileRead = fopen("randomARTIFICIALE.binary","rb");
        if (fileRead == NULL)
        {
            printf("Errore nell'apertura del file urandomARTIFICIALE.binary\n");
            exit(EXIT_FAILURE);
        }
        printf("File randomARTIFICIALE.binary aperto correttamente\n");
    }
    
    printf("Tento di aprire il file radar.log in scrittura\n");
    fileRadarLog = fopen("radar.log", "w+");
    if (fileRadarLog == NULL)
    {
        printf("Errore nell'apertura del file radar.log\n");
        exit(EXIT_FAILURE);
    }
    printf("File radar.log aperto correttamente\n");
    
    while((socketFileDescriptor = createConnection("ecuSocket")) < 0)
    {
        sleep(1);
    }

    int elementsRead;
    while(1)
    {
        elementsRead = fread(buffer, 1, 8, fileRead);
        if (elementsRead == 8) {
            printf("Invio dati alla socket\n");
            write(socketFileDescriptor, buffer, strlen(buffer)+1);
            printf("Scrivo sul file radar.log\n");
            for (int c = 0; c < 8; c++)
            {
                fprintf(fileRadarLog, "%.2X", buffer[c]);
            }
            fprintf(fileRadarLog, "\n");
            fflush(fileRadarLog);
        }
        sleep(2);
    }
    fclose(fileRadarLog);
    exit(EXIT_SUCCESS);
}


void sigTermHandler() {
    fclose(fileRead);
    fclose(fileRadarLog);
    exit(EXIT_SUCCESS);
}

void openFile(char filename[], char mode[], FILE **filePointer) {
    *filePointer = fopen(filename, mode);
    if (*filePointer == NULL) {
        printf("Errore nell'apertura del file\n");
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


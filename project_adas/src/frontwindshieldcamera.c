//
// Created by jonathan on 09/05/23.
//
/*
 * Il sensore Front Windshield Camera quando viene creato dalla ECU si connette alla socket per
scambiare i messaggi con ECU server e iterativamente, ogni secondo, legge dati da una
sorgente e li invia alla ECU.
I dati inviati sono inoltre registrati nel file di log camera.log.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#include "functions.h"


FILE *cameraLog;
int socketFileDescriptor;
long readPosition;
int fileToRead;

void handleFailure(int signal) {
    close(fileToRead);
    fclose(cameraLog);
    exit(EXIT_FAILURE);
}

void stopHandler(int signal) {
    close(socketFileDescriptor);
}

int main(int argc, char *argv[])
{
    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("PROCESSO FRONT WIND SHIELD CAMERA\n");

    printf("Cerco di aprire camera.log\n");

    cameraLog = fopen("camera.log", "w");

    if (cameraLog == NULL)
    {
        printf("Errore sull'apertura del file camera.log\n");
        perror("open file error!\n");
        exit(EXIT_FAILURE);
    }

    printf("File camera.log aperto correttamente\n");

    signal(SIGUSR1, handleFailure);
    signal(SIGTSTP, stopHandler);

    printf("Tento di aprire il file frontCamera.data \n");
    fileToRead = open("frontCamera.data", O_RDONLY);
    
    printf("Inizializzo la socket\n");
    int socketFd, serverLen;
    struct sockaddr_un serverUNIXAddress;
    struct sockaddr* serverSockAddrPtr;
    int sensorID = 0;
    int isListening; 
    char command[16];
    int index = 0;
    for(;;)
    {
        serverSockAddrPtr = (struct sockaddr*) &serverUNIXAddress;
        serverLen = sizeof (serverUNIXAddress);
        socketFd = socket (AF_UNIX, SOCK_STREAM, 0);
        serverUNIXAddress.sun_family = AF_UNIX;
        strcpy (serverUNIXAddress.sun_path, "./ecuSocket");

        while(connect(socketFd, serverSockAddrPtr, serverLen) < 0) 
        { 
            sleep(2);
        }

        memset(command, '\0', 16);
        
        while(read(fileToRead, &command[index], 1) < 0)
        {
            while ((command[index] != '\n') && (command != EOF))
            {
                index++;
                while(read(fileToRead, &command[index], 1) < 0);
            }
        }

        while(send(socketFd, &sensorID, sizeof(int), 0) < 0);
        while (read(socketFd, &isListening, sizeof(int)) < 0);

        if (isListening == 1)
        {
            send(socketFd, command, strlen(command) + 1, 0);
            writeMessage(cameraLog, "%s", command);
        }
        close(socketFd);

        if (command == EOF)
        {
            fclose(cameraLog);
            exit(EXIT_SUCCESS);
        }
        sleep(1);
    }
    fclose(cameraLog);
    exit(EXIT_SUCCESS);
}



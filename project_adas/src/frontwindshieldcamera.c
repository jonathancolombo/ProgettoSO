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
int fileToRead;

void handleFailure(int signal) {
    close(fileToRead);
    fclose(cameraLog);
    exit(EXIT_FAILURE);
}

void stopHandler(int signal) {
    close(socketFileDescriptor);
}

int main(int argc, char *argv[]) {
    printf("PROCESSO FRONT WIND SHIELD CAMERA\n");

    printf("Cerco di aprire camera.log\n");
    cameraLog = fopen("camera.log", "w");

    if (cameraLog == NULL) {
        printf("Errore sull'apertura del file camera.log\n");
        perror("open file error!\n");
        exit(EXIT_FAILURE);
    }

    printf("File camera.log aperto correttamente\n");

    signal(SIGUSR1, handleFailure);
    signal(SIGTSTP, stopHandler);

    printf("Tento di aprire il file frontCamera.data\n");
    fileToRead = open("frontCamera.data", O_RDONLY);

    printf("Inizializzo la socket\n");
    int socketFd, serverLen;
    struct sockaddr_un serverUNIXAddress;
    struct sockaddr *serverSockAddrPtr;
    int sensorID = 0;
    int isListening;
    char command[16];
    int index = 0;

    for (;;) {
        serverSockAddrPtr = (struct sockaddr *) &serverUNIXAddress;
        serverLen = sizeof(serverUNIXAddress);
        socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
        serverUNIXAddress.sun_family = AF_UNIX;
        strcpy(serverUNIXAddress.sun_path, "./ecuSocket");

        while (connect(socketFd, serverSockAddrPtr, serverLen) < 0) {
            sleep(2);
        }

        memset(command, '\0', sizeof(command));
        index = 0;

        while (read(fileToRead, &command[index], 1) > 0) {
            if (command[index] == '\n') {
                while (send(socketFd, &sensorID, sizeof(int), 0) < 0);
                while (read(socketFd, &isListening, sizeof(int)) < 0);

                if (isListening == 1) {
                    send(socketFd, command, strlen(command) + 1, 0);
                    writeMessage(cameraLog, "%s", command);
                }

                break;
            }

            index++;
        }

        close(socketFd);

        if (command[0] == '\0') {
            // Hai raggiunto la fine del file
            fclose(cameraLog);
            close(fileToRead);
            exit(EXIT_SUCCESS);
        }

        sleep(1);
    }

    return 0;
}

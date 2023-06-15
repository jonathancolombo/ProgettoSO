#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#define DEFAULT_PROTOCOL 0

#include "commonFunctions.h"
#include "socketFunctions.h"

FILE *cameraLog;
int cameraDataFileDescriptor;
int ecuSocketFileDescriptor;

void handleFailure(int signal) {
    close(cameraDataFileDescriptor);
    fclose(cameraLog);
    exit(EXIT_FAILURE);
}

void stopHandler(int signal) {
    close(ecuSocketFileDescriptor);
}

void initializeCamera() {
    createLog("./camera", &cameraLog);

    signal(SIGUSR1, handleFailure);
    signal(SIGTSTP, stopHandler);

    cameraDataFileDescriptor = open("./frontCamera.data", O_RDONLY);
}

void sendDataToEcu(const char *data) {
    const int cameraID = 0; // Identificatore per la camera
    int isEcuListening;

    int ecuAddressLength;
    struct sockaddr_un ecuUNIXAddress;
    struct sockaddr *ecuSocketAddressPtr = (struct sockaddr *)&ecuUNIXAddress;

    socketAuth(&ecuSocketFileDescriptor, &ecuUNIXAddress, &ecuAddressLength, "./ecuSocket");
    connectServer(ecuSocketFileDescriptor, ecuSocketAddressPtr, ecuAddressLength);

    while (send(ecuSocketFileDescriptor, &cameraID, sizeof(cameraID), 0) < 0)
        ;
    while (recv(ecuSocketFileDescriptor, &isEcuListening, sizeof(cameraID), 0) < 0)
        ;

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

        sendDataToEcu(line);

        if (temp == EOF) {
            fclose(cameraLog);
            exit(EXIT_SUCCESS);
        }
        sleep(1);
    }
}

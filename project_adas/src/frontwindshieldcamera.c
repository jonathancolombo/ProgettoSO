#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#define DEFAULT_PROTOCOL 0

#include "commonFunctions.h"
#include "socketFunctions.h"

FILE *sensorLog;
int fd;
int ecuFd;

void handleFailure(int signal) {
    close(fd);
    fclose(sensorLog);
    exit(EXIT_FAILURE);
}

void stopHandler(int signal) {
    close(ecuFd);
}

int main(int argc, char *argv[]) {
    createLog("./camera", &sensorLog);

    signal(SIGUSR1, handleFailure);
    signal(SIGTSTP, stopHandler);
    
    const int sensorID = 0; // Value in order to be recognized by the socket
    int isListening;    // Indicates whether ECU is listening or not

    int ecuLen;
    struct sockaddr_un ecuUNIXAddress;
    struct sockaddr *ecuSockAddrPtr = (struct sockaddr*) &ecuUNIXAddress;

    fd = open("./frontCamera.data", O_RDONLY);
    char str[16];
    int i = 0;

    while(1) {
        socketAuth(&ecuFd, &ecuUNIXAddress, &ecuLen, "./ecuSocket");
        connectServer(ecuFd, ecuSockAddrPtr, ecuLen);

        memset(str, '\0', 16);
        i = 0;
        while(read(fd, &str[i], 1) < 0);
        while(str[i] != '\n' && str[i] != EOF) {
            i++;
            while(read(fd, &str[i], 1) < 0);
        }
        char temp = str[i];
        str[i] = '\0';
        while(send(ecuFd, &sensorID, sizeof(sensorID), 0) < 0);
        while(recv(ecuFd, &isListening, sizeof(sensorID), 0) < 0);
        if(isListening == 1) {
            send(ecuFd, str, strlen(str)+1, 0);
            writeMessage(sensorLog, "%s", str);
        }
        close(ecuFd);
        if (temp == EOF)
        {
            fclose(sensorLog);
            exit(EXIT_SUCCESS);
        }
        sleep(1);
    }
}
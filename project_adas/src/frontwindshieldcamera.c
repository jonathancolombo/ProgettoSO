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
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

FILE* fileLog;
FILE *fileToRead;
int socketFileDescriptor = 0;
long readPosition = 0;

void openFile(char filename[], char mode[], FILE **filePointer);
void sigTermHandler();
char* readLine(FILE *filePointer);
int createConnection(char *socketName);

int main(int argc, char *argv[])
{
    signal(SIGTERM, sigTermHandler);
    openFile("frontCamera.data","r", &fileToRead);
    openFile("camera.log", "w", &fileLog);
    // aggiornare il puntatore di lettura
    FILE *fileUtility;
    openFile("utility.data", "r", &fileUtility);
    char *buffer;
    buffer = readLine(fileUtility);
    readPosition = atol(buffer);
    fclose(fileUtility);
    fseek(fileToRead, readPosition, SEEK_SET);

    while(socketFileDescriptor = createConnection("forwardingWindShieldCamera") < 0)
    {
        sleep(1);
    }

    char lineaLetta[100];
    for(;;)
    {
        char *linea = readLine(fileToRead);
        //sendToSocket(socketFd, linea); la send = write
        write(socketFileDescriptor, linea, strlen(linea)+1);
        fprintf(fileLog, "%s\n",linea);
        fflush(fileLog);
        sleep(1);
    }
    return EXIT_SUCCESS;
}


// non dovrebbe essere necessaria...Da testare
void sigTermHandler() {
    fclose(fileLog);
    FILE *fileUtility;
    openFile("utility.data", "w", &fileUtility);
    readPosition = ftell(fileToRead);
    fprintf(fileUtility, "%ld\n",readPosition);
    fclose(fileUtility);
    fclose(fileToRead);
    exit(0);
}


void openFile(char filename[], char mode[], FILE **filePointer) {
    *filePointer = fopen(filename, mode);
    if (*filePointer == NULL) {
        printf("Errore nell'apertura del file");
        exit(1);
    }
}

char* readLine(FILE *filePointer){
    int maximumLineLength = 8;
    char *lineBuffer = (char *)malloc(sizeof(char) * maximumLineLength);
    if (lineBuffer == NULL) {
        printf("Errore nell'allocare la memoria del buffer.\n");
        exit(1);
    }
    char ch = getc(filePointer);
    int count = 0;
    while ((ch != '\n') && (ch != EOF)) {
        lineBuffer[count] = ch;
        count++;
        ch = getc(filePointer);
    }
    return lineBuffer;
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

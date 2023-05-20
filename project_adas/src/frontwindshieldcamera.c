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

FILE *fileLog;
FILE *fileToRead;
int socketFileDescriptor;
long readPosition;

void openFile(char filename[], char mode[], FILE **filePointer);
void sigTermHandler();
char* readLine(FILE*);
int createConnection(char *socketName);

int main(int argc, char *argv[])
{
    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("PROCESSO FRONT WIND SHIELD CAMERA\n");
    signal(SIGTERM, sigTermHandler);

    printf("Tento di aprire il file frontCamera.data \n");
    fileToRead = fopen("frontCamera.data", "r");
    if (fileToRead ==  NULL)
    {
        printf("Errore nell'apertura del file frontCamera.data\n");
        exit(EXIT_FAILURE);
    }

    printf("Tento di aprire il file utility.data \n");
    fileLog = fopen("utility.data", "w");
    if (fileLog == NULL)
    {
        printf("Errore nell'apertura del file utility.data\n");
        exit(EXIT_FAILURE);
    }

    // aggiornare il puntatore di lettura
    FILE *fileUtility;
    printf("Tento di aprire il file utility.data in lettura per l'aggiornamento di posizione del puntatore\n");
    fileUtility = fopen("utility.data", "r");
    if (fileUtility == NULL)
    {
        printf("Errore nell'apertura del file utility.data\n");
        exit(EXIT_FAILURE);
    }    

    printf("Aggiorno la posizione del puntatore\n");
    char *buffer;
    buffer = readLine(fileUtility);
    readPosition = atol(buffer);
    fclose(fileUtility);
    fseek(fileToRead, readPosition, SEEK_SET);

    printf("Apro la connessione con la socket\n");
    while(socketFileDescriptor = createConnection("forwardingWindShieldCamera") < 0)
    {
        sleep(1);
    }

    char lineaLetta[100];
    for(;;)
    {
        printf("Leggo la linea \n");
        char *linea = readLine(fileToRead);
        printf("La mando sulla socket\n");
        write(socketFileDescriptor, linea, strlen(linea)+1);
        fprintf(fileLog, "%s\n",linea);
        fflush(fileLog);
        printf("Fatto\n");
        sleep(1);
    }
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
    exit(EXIT_SUCCESS);
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
    char *lineBuffer = (char *) malloc(sizeof(char) * maximumLineLength);
    if (lineBuffer == NULL) {
        printf("Errore nell'allocare la memoria del buffer.\n");
        exit(1);
    }
    char ch = getc(filePointer);
    int count = 0;
    while ((ch != '\n') && (ch != EOF)) 
    {
        lineBuffer[count] = ch;
        count++;
        ch = getc(filePointer);
    }
    printf("Linea letta\n");
    return lineBuffer;
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
    printf("Socket connessa\n");
    return socketFd;
}

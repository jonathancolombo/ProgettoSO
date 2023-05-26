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

FILE *fileLog;
FILE *fileToRead;
int socketFileDescriptor;
long readPosition;

void writeMessage(FILE *fp, const char * format, ...);
void formattedTime(char *timeBuffer);

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
    //signal(SIGTERM, sigTermHandler);

    printf("Cerco di aprire camera.log\n");

    fileLog = fopen("camera.log", "w+");

    if (fileLog == NULL)
    {
        printf("Errore sull'apertura del file camera.log\n");
        perror("open file error!\n");
        exit(EXIT_FAILURE);
    }

    printf("File camera.log aperto correttamente\n");

    printf("Tento di aprire il file frontCamera.data \n");
    int fileToRead = open("frontCamera.data", O_RDONLY);
    
    printf("Inizializzo la socket\n");
    int socketFd, serverLen;
    struct sockaddr_un serverUNIXAddress;
    struct sockaddr* serverSockAddrPtr;
    serverSockAddrPtr = (struct sockaddr*) &serverUNIXAddress;
    serverLen = sizeof (serverUNIXAddress);
    socketFd = socket (AF_UNIX, SOCK_STREAM, 0);
    serverUNIXAddress.sun_family = AF_UNIX; /* Server domain */
    strcpy (serverUNIXAddress.sun_path, "ecuSocket");
    int sensorID = 0;
    int isListening; 
    char command[16];
    int index = 0;
    for(;;)
    {
        while(connect(socketFd, serverSockAddrPtr, serverLen) < 0) 
        { 
            sleep(2);
        }

        memset(command, '\0', 16);
        
        while(read(fileToRead, command[index], 1) < 0)
        {
            while ((command[index] != '\n') && (command != EOF))
            {
                index++;
                while(read(fileToRead, command[index], 1) < 0);
            }
        }

        while(send(socketFd, &sensorID, sizeof(int), 0) < 0);
        while (read(socketFd, &isListening, sizeof(int)) < 0);

        if (isListening == 1)
        {
            send(socketFd, command, strlen(command) + 1, 0);
            writeMessage(fileLog, "%s", command);
        }
        close(socketFd);

        if (command == EOF)
        {
            fclose(fileLog);
            exit(EXIT_SUCCESS);
        }
        sleep(1);
    }
    fclose(fileLog);
    exit(EXIT_SUCCESS);
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

void writeMessage(FILE *fp, const char * format, ...)
{
    char buffer[256];
    char date[256];
    formattedTime(date);
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);
    fprintf(fp, "[%s] - [%s]\n", date, buffer);
    fflush(fp);
}

void formattedTime(char *timeBuffer) 
{
    time_t rawTime;
    struct tm *info;
    time(&rawTime);
    info = localtime(&rawTime);
    strftime(timeBuffer,256,"%x - %X", info);
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

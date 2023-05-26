//
// Created by jonathan on 09/05/23.
//

/*
 *  * Park assist: è un processo che è agisce solo su richiesta della
Central ECU. Quando riceve un comando di attivazione dalla Central
ECU:
– Per 30 secondi, 1 volta al secondo, legge 8 byte da /dev/urandom, li invia
alla Central ECU, scrive su file di log.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/socket.h>

pid_t ecuPid;
pid_t pidSurroundViewCamera;
int restart = 0;
int socketFileDescriptor;
FILE *fileLog;
FILE *fileRead;

unsigned char buffer[8]; // 8 = max byte

void sigStartHandler();
void sigStopHandler();
void sigTermHandler();
int createConnection(char *socketName);
void openFile(char filename[], char mode[], FILE **filePointer);
void init(char* modalita);

int main(int argc, char *argv[])
{
    ecuPid = getppid();
    pidSurroundViewCamera = fork();

    if (pidSurroundViewCamera < 0) 
    {
        perror("fork error!");
        return EXIT_FAILURE;
    }

    if (pidSurroundViewCamera == 0) 
    {
        char *newArgv[] = {"./surroundviewcamera", NULL};
        execv(newArgv[0], newArgv);
        perror("execv error!");
        return EXIT_FAILURE;
    } 
    else 
    {
        signal(SIGUSR1, sigStartHandler);
        signal(SIGTERM, sigTermHandler);
        
        init(argv[1]);
        pause();
    }

    return EXIT_SUCCESS;

}

void init(char* modalita)
{
    if (strcmp(modalita, "NORMALE") == 0) 
    {
        openFile("/dev/urandom", "rb", &fileRead);
    } 
    else 
    {
        openFile("urandomARTIFICIALE.binary", "rb", &fileRead);
    }
        
        openFile("assist.log","w",&fileLog);
        while((socketFileDescriptor = createConnection("./ecuSocket")) < 0)
            sleep(1);
}

void sigStartHandler(){

    signal(SIGUSR1, sigStopHandler);
    kill(pidSurroundViewCamera, SIGUSR1);	// AVVIA SorroundViewWithCameras
    // parking readAndLog();
    printf("Inizio procedura di parcheggio\n");

    for (int i = 0; i<30; i++){
        printf("\rParcheggio in corso: %d%%", (i + 1) * 100 / 30);
        fflush(stdout);
        if (fread(buffer,1,4,fileRead) == 0) {
            perror("parkAssist: errore di lettura");
            exit(1);
        }
        if (restart == 1) {
            i = 0;		// se riceve un segnale di restart riparte per altri 30 secondi
            restart = 0;	// resetta restart
        }

        write(socketFileDescriptor, buffer, strlen(buffer) + 1);

        for (int c = 0; c < 4; c++) {
            fprintf(fileLog, "%.2X", buffer[c]);
        }
        fprintf(fileLog, "\n");

        fflush(fileLog);
        sleep(1);
    }

    printf("\n");
    kill(pidSurroundViewCamera,SIGUSR1); 	// DISATTIVA SorroundViewWithCameras
    kill(ecuPid, SIGUSR1); 	// COMUNICA alla ECU che ha finito
    sigTermHandler();		// MUORE
}

void sigStopHandler() {
    signal(SIGUSR1,sigStopHandler);
    int restart = 1;
}

void sigTermHandler() {
    signal(SIGTERM, SIG_DFL);
    fclose(fileRead);
    fclose(fileLog);
    kill(pidSurroundViewCamera,SIGTERM); 	// kill SVC process
    kill(getpid(), SIGTERM);
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

void openFile(char filename[], char mode[], FILE **filePointer) {
    *filePointer = fopen(filename, mode);
    if (*filePointer == NULL) {
        printf("Errore nell'apertura del file");
        exit(1);
    }
}

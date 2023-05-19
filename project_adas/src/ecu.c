//
// Created by jonathan on 09/05/23.
// DA FINIRE
//

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <unistd.h>
#include <fcntl.h>

// ecu client e server arrays
int ecuServerSocketArray[5];
int ecuClientSocketArray[3]; // throttle control = 0, brake by wire = 1, steer by wire = 2

// dichiarazioni PID hmi
pid_t hmiPID;
pid_t fatherPID;

// PID attuatori
pid_t attuatoriPid[3]; // 3 attuatori (steer by wire, throttle control, brake by wire

// PID sensori
pid_t sensoriPid[3]; // 3 sensori (front windshield camera, front facing radar, park assist)
pid_t frontWindshieldCameraClientPid;
pid_t forwardFacingRadarClientPid;
pid_t listenersPid[5]; // 5 socket number


// socket server 
struct sockaddr_un serverAddress;
struct sockaddr* ptrServerAddress;

// socket client
struct sockaddr_un clientAddress; 
struct sockaddr* ptrClientAddress;

// lunghezza client e server
int serverLength = 0;
int clientLength = 0; 

int READ = 0;
int WRITE = 1;
int currentSpeed = 0;

//
char sockets_name[][50]={
"frontfacingradarSocket",
"parkassistSocket",
"frontwindshieldSocket",
"surroundviewcameraSocket"
};

// inizializzazione pipe
// ognuno di lunghezza due per un file in lettura e l'altro in scrittura
int PIPE_server_to_frontwindshieldcamera_manager[2];
int PIPE_server_to_forwardfacingradar_manager[2];
int PIPE_server_to_hmi_manager[2];
//int PIPE_server_to_bs_manager[2]; // non esiste questo sensore in quanto ci sono 
int PIPE_server_to_parkassist_manager[2];

// file ECU.log
FILE *ecuLog;

int createSocketConnection(char *socketName);
int checkFfrWarning(unsigned char *data);
void serverStart();
int readFromSocket (int fd, char *str);
void openFile(char filename[], char mode[], FILE **filePointer);
int extractString(char* data);
int readFromPipe (int pipeFd, char *data);
int isInByteArray(unsigned char *len2toFind, unsigned char *toSearch, int toSearchLen);

void sigstartHandler();
void sigparkHandler();
void sigParkCallHandler();
void sigParkEndHandler();
void managePericoloToBrakeByWire();

int main(int argc, char *argv[])
{
    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("PROCESSO ECU\n");
    // primo segnale
    signal(SIGUSR1, sigstartHandler);
    hmiPID = getppid();
    fatherPID = getpid();

    // inizializzazione velocità
    int speed = 0;

    // inizializzazione socket server
    struct sockaddr_un serverAddress;
    struct sockaddr* ptrServerAddress;
    ptrServerAddress = (struct sockaddr*) &serverAddress;
    int serverLength = sizeof (serverAddress);

    // inizializzazione socket client
    struct sockaddr_un clientAddress;
    struct sockaddr* ptrClientAddress;
    ptrClientAddress = (struct sockaddr*) &clientAddress;
    int clientLength = sizeof (clientAddress);

    serverAddress.sun_family = AF_UNIX;

    // creazione delle pipe
    printf("Creo le varie PIPE per comunicare\n");
    pipe(PIPE_server_to_frontwindshieldcamera_manager);
    pipe(PIPE_server_to_forwardfacingradar_manager);
    pipe(PIPE_server_to_hmi_manager);
    //pipe(PIPE_server_to_bs_manager); // non esiste questo sensore
    pipe(PIPE_server_to_parkassist_manager);

    printf("Sistema inizializzato.\n\n");

    printf("Processo ecu bloccato da pause\n");
    pause();

    int indexAttuatori = 0;
    int indexSensori = 0;
    attuatoriPid[indexAttuatori] = fork();

    if (attuatoriPid[indexAttuatori] == 0) // 0, child steer by wire
    {
        // chiudi tutte le pipe
        //close(PIPE_server_to_bs_manager[READ]);
        //close(PIPE_server_to_bs_manager[WRITE]);
        printf("Chiudo tutte le pipe dal processo figlio steer by wire\n");
        close(PIPE_server_to_forwardfacingradar_manager[READ]);
        close(PIPE_server_to_forwardfacingradar_manager[WRITE]);
        close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
        close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
        close(PIPE_server_to_parkassist_manager[READ]);
        close(PIPE_server_to_parkassist_manager[WRITE]);
        close(PIPE_server_to_hmi_manager[READ]);
        close(PIPE_server_to_hmi_manager[WRITE]);

        // viene passato il comando tramite exec ed eseguito lo steer by wire
        printf("Eseguo steer by wire con una execv\n");
        argv[0] = "./steerbywire";
        execv(argv[0],argv);
        return;
    }
    else
    {
        indexAttuatori++; // 1
        attuatoriPid[indexAttuatori] = fork();
        if (attuatoriPid[indexAttuatori] == 0) // throttle by control
        {
            // chiudi tutte le pipe
            //close(PIPE_server_to_bs_manager[READ]);
            //close(PIPE_server_to_bs_manager[WRITE]);
            printf("Chiudo tutte le pipe dal processo figlio throttle by control\n");
            close(PIPE_server_to_forwardfacingradar_manager[READ]);
            close(PIPE_server_to_forwardfacingradar_manager[WRITE]);
            close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
            close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
            close(PIPE_server_to_parkassist_manager[READ]);
            close(PIPE_server_to_parkassist_manager[WRITE]);
            close(PIPE_server_to_hmi_manager[READ]);
            close(PIPE_server_to_hmi_manager[WRITE]);
            printf("Eseguo throttle by control con una execv\n");
            argv[0] = "./throttlebycontrol";
            execv(argv[0],argv);
            return;
        }
        else
        {
            indexAttuatori++; // 2
            attuatoriPid[indexAttuatori] = fork();
            if (attuatoriPid[indexAttuatori] == 0) // child brake by wire
            {
                printf("Chiudo tutte le pipe dal processo figlio brake by wire\n");
                close(PIPE_server_to_forwardfacingradar_manager[READ]);
                close(PIPE_server_to_forwardfacingradar_manager[WRITE]);
                close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
                close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
                close(PIPE_server_to_parkassist_manager[READ]);
                close(PIPE_server_to_parkassist_manager[WRITE]);
                close(PIPE_server_to_hmi_manager[READ]);
                close(PIPE_server_to_hmi_manager[WRITE]);
                argv[0] = "./brakebywire";
                execv(argv[0],argv);
            }
            else
            {
                frontWindshieldCameraClientPid = fork();
                if (frontWindshieldCameraClientPid == 0)
                {
                    printf("Chiudo tutte le pipe dal processo figlio front windshield camera\n");
                    close(PIPE_server_to_forwardfacingradar_manager[WRITE]);
                    close(PIPE_server_to_forwardfacingradar_manager[READ]);
                    close(PIPE_server_to_parkassist_manager[WRITE]);
                    close(PIPE_server_to_parkassist_manager[READ]);
                    close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
                    //close(PIPE_server_to_bs_manager[WRITE]);
                    //close(PIPE_server_to_bs_manager[READ]);
                    close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
                    // ecuFwcClient(); qua sotto
                    while((ecuClientSocketArray[0] = createSocketConnection("throttlebycontrolSocket")) < 0) //attende di connettersi alle socket degli attuatori
                        usleep(100000);

                    while ((ecuClientSocketArray[1] = createSocketConnection("brakebywireSocket")) < 0)
                        usleep(100000);

                    while ((ecuClientSocketArray[2] = createSocketConnection("steerbywireSocket")) < 0)
                        usleep(100000);

                    char dataReceived[100];
                        
                    for(;;)
                    {
                        read(PIPE_server_to_frontwindshieldcamera_manager[READ], dataReceived, 100);
                        char command[30];
                        sprintf(command, "%s", "NO COMMAND");
                        if (strcmp(dataReceived, "SINISTRA")==0 || strcmp(dataReceived, "DESTRA") ==0)
                        {
                            sprintf(command, "%s", dataReceived);
                            // invio dati a steer by wire
                            printf("Invio i dati a steer by wire\n");
                            write(ecuClientSocketArray[2], dataReceived, strlen(dataReceived) + 1);
                        }
                        else if (strcmp(dataReceived, "PERICOLO") == 0)
                        {
                            // fai qualcos'altro (da gestire il pericolo)
                            printf("Segnalo il brake by wire per gestire il pericolo\n");
                            kill(attuatoriPid[2], SIGUSR1);
	                        waitpid(attuatoriPid[2], NULL, 0);
	                        kill(hmiPID, SIGUSR2); //comunico alla hmi di terminare l'intero albero di processi, lei esclusa, e di riavviare il sistema
                        }
                        else
                        {
                            int newSpeed = atoi(dataReceived); // converte da stringa a intero
                            if (newSpeed < speed)
                            {
                                sprintf(command, "%s%d", "FRENA", (speed - newSpeed));
                                speed = newSpeed;
                                write(ecuClientSocketArray[1], command, strlen(command) + 1);
                            }
                            else if (newSpeed > speed)
                            {
                                sprintf(command, "%s%d", "INCREMENTO", (speed + newSpeed));
                                write(ecuClientSocketArray[0], command, strlen(command) + 1);
                            }
                            char updatedSpeed[10];
                            sprintf(updatedSpeed, "%s %d", "#", newSpeed);
                            write(PIPE_server_to_hmi_manager[WRITE], updatedSpeed, strlen(updatedSpeed)+1);
                        }
                        if (strcmp(command, "NO COMMAND") != 0)
                        {
                            write(PIPE_server_to_hmi_manager[WRITE], command, strlen(command) + 1);
                        }
                    }
                }
                else
                {
                    sensoriPid[indexSensori] = fork(); // 0
                    if (sensoriPid[indexSensori] == 0) // front windshield camera process
                    {
                        printf("Chiudo tutte le pipe dal processo figlio front wind shield camera\n");
                        close(PIPE_server_to_hmi_manager[WRITE]);
                        close(PIPE_server_to_hmi_manager[READ]);
                        close(PIPE_server_to_forwardfacingradar_manager[WRITE]);
                        close(PIPE_server_to_forwardfacingradar_manager[READ]);
                        close(PIPE_server_to_parkassist_manager[WRITE]);
                        close(PIPE_server_to_parkassist_manager[READ]);
                        close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
                        close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
                        argv[0] = "./frontwindshieldcamera";
                        execv(argv[0], argv);
                        //return;
                    }
                    else
                    {
                        indexSensori++; // 1
                        sensoriPid[indexSensori] = fork();
                        if (sensoriPid[indexSensori] == 0) // forwarding racing radar
                        {
                            printf("Dal processo figlio forwarding facing radar chiudo tutte le pipe\n");
                            close(PIPE_server_to_hmi_manager[WRITE]);
                            close(PIPE_server_to_hmi_manager[READ]);
                            close(PIPE_server_to_forwardfacingradar_manager[WRITE]);
                            close(PIPE_server_to_forwardfacingradar_manager[READ]);
                            close(PIPE_server_to_parkassist_manager[WRITE]);
                            close(PIPE_server_to_parkassist_manager[READ]);
                            close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
                            close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
                            argv[0] = "./forwardingfacingradar";
                            execv(argv[0],argv);
                        }
                        else
                        {
                            indexSensori++; // 2
                            sensoriPid[indexSensori] = fork();
                            if (sensoriPid[indexSensori] == 0) // park assist
                            {
                                printf("Dal processo figlio park assist chiudo tutte le pipe\n");
                                close(PIPE_server_to_hmi_manager[WRITE]);
                                close(PIPE_server_to_hmi_manager[READ]);
                                close(PIPE_server_to_forwardfacingradar_manager[WRITE]);
                                close(PIPE_server_to_forwardfacingradar_manager[READ]);
                                close(PIPE_server_to_parkassist_manager[WRITE]);
                                close(PIPE_server_to_parkassist_manager[READ]);
                                close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
                                close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
                                argv[0] = "./parkassist";
                                execv(argv[0], argv);
                            }
                            else
                            {
                                forwardFacingRadarClientPid = fork(); // forward facing radar client
                                if (forwardFacingRadarClientPid == 0)
                                {
                                    printf("Chiudo tutte le pipe da processo figlio client forwarding facing radar");
                                    close(PIPE_server_to_hmi_manager[WRITE]);
                                    close(PIPE_server_to_hmi_manager[READ]);
                                    close(PIPE_server_to_parkassist_manager[WRITE]);
                                    close(PIPE_server_to_parkassist_manager[READ]);
                                    close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
                                    close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
                                    close(PIPE_server_to_forwardfacingradar_manager[WRITE]);

                                    char data[100];
                                    for (;;)
                                    {
                                        // leggi dalla pipe
                                        int n = 0;
                                        do {
                                            n = read (PIPE_server_to_forwardfacingradar_manager[READ], data, 1);
                                        } while (n > 0);
                                        //return (n > 0);

                                        if(checkFfrWarning(data) == 1)
                                        {
                                            managePericoloToBrakeByWire();
                                        }
		                                    
                                    }

                                }
                                else
                                {
                                    serverStart();
                                }
                            }
                        }

                        }

                    }




            }
        }

    }


    pause(); //blocca il processo
    //la ecu esegue le sue funzioni normali con &argv[1]


}


void managePericoloToBrakeByWire()
{
	kill(attuatoriPid[2], SIGUSR1);
	waitpid(attuatoriPid[2], NULL, 0);
	kill(hmiPID, SIGUSR2); //comunico alla hmi di terminare l'intero albero di processi, lei esclusa, e di riavviare il sistema
}

// ARRIVATO FINO A QUI. (CONTINUARE CON LA ECU - BESTIAAAAAAA)
void serverStart()
{
    int socketFileDescriptor, clientFileDescriptor;

    for(int i = 0; i < 5; i++) // 5 = server socket number
    {
        listenersPid[i] = fork();
        if(listenersPid[i] == 0){
            ecuServerSocketArray[i] = socket (AF_UNIX, SOCK_STREAM, 0); // 0 = default protocol 
            strcpy (serverAddress.sun_path, sockets_name[i]);
            unlink (sockets_name[i]);
            bind (ecuServerSocketArray[i], ptrServerAddress, serverLength);
            listen (ecuServerSocketArray[i], 1);

            while (1) {/* Loop forever */ /* Accept a client connection */
                socketFileDescriptor = accept (ecuServerSocketArray[i], ptrClientAddress, &clientLength);
                char str[200];
                while(readFromSocket(socketFileDescriptor, str)){
                    if(strcmp(sockets_name[i], "frontwindshieldSocket")== 0){//controlla che figlio è
                    printf("Invio i dati a frontwindshield tramite socket dalla ecu\n");
                        write(PIPE_server_to_frontwindshieldcamera_manager[WRITE], str, strlen(str)+1);
                    } //Invia alla pipe che gestisce gli fwc.
                    else if(strcmp(sockets_name[i], "forwardfacingradarSocket") == 0){
                        printf("Invio i dati a forwardfacingradar tramite socket dalla ecu\n");
                        write(PIPE_server_to_forwardfacingradar_manager[WRITE], str, strlen(str)+1);
                    }else if(strcmp(sockets_name[i], "parkassistSocket") == 0){
                        printf("Invi i dati a parkassist tramite socket dalla ecu\n");
                        write(PIPE_server_to_parkassist_manager[WRITE], str, strlen(str)+1);
                    }
                }
                printf("Chiudo la socket ed esco\n");
                close (socketFileDescriptor); /* Close the socket */
                exit (EXIT_SUCCESS); /* Terminate */
            }
        }
    }
    close(PIPE_server_to_forwardfacingradar_manager[WRITE]);
    close(PIPE_server_to_forwardfacingradar_manager[READ]);
    close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
    close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
    close(PIPE_server_to_parkassist_manager[WRITE]);
    close(PIPE_server_to_parkassist_manager[READ]);
    //hmiCommunication(); 
    
    char dataReceived[200];
    int updatedSpeed;
    
    ecuLog = fopen("ECU.log", "a");
	
    if (ecuLog == NULL) {
		printf("Errore nell'apertura del file ECU.log");
        perror("open file error!\n");
		exit(EXIT_FAILURE);
	}

    
    while(readFromPipe(PIPE_server_to_hmi_manager[READ], dataReceived))
    {
        char receivedSpeed[20];
        sprintf(receivedSpeed, "%s", dataReceived);

        if((updatedSpeed = extractString(receivedSpeed)) > -1)
        {
            currentSpeed = updatedSpeed;
        }
        else
        {
            printf("Scrivo su file di log ECU.log\n");
            fprintf(ecuLog, "%s\n", dataReceived);
            fflush(ecuLog);
        }
    }

}

// metodi aggiunti
void sigstartHandler() {
	signal(SIGUSR1, sigparkHandler); //reset signal
    //return;
}

void sigparkHandler() {
    printf("Avvio la procedura di parcheggio segnalando park assist\n");
	signal(SIGUSR1, sigParkCallHandler);
	char parkCommand[20];
	sprintf(parkCommand,"%s %d", "PARCHEGGIO", currentSpeed);
    // mi creo e mi connetto alla socket con brake by wire
    printf("Creo la socket dalla ecu per connettermi con brake by wire\n");
	int bbwConne = createSocketConnection("brakebywireSocket");
	printf("Mando un comando a brake by wire\n");
    write(bbwConne, parkCommand, strlen(parkCommand) + 1);

    printf("Mando il comando di scrittura con la pipe alla hmi\n");
	write(PIPE_server_to_hmi_manager[WRITE], parkCommand, strlen(parkCommand)+1);
	printf("Faccio la kill su throttle by control e steer by wire\n");
    kill(attuatoriPid[0], SIGTERM);
	kill(attuatoriPid[1], SIGTERM);
	kill(sensoriPid[0], SIGTERM);
	kill(sensoriPid[1], SIGTERM);
}

void sigParkCallHandler(){
	signal(SIGUSR1,sigParkEndHandler);
	kill(sensoriPid[2], SIGUSR1); // avvia procedura di PARCHEGGIO
}

void sigParkEndHandler(){
    printf("Comunico alla hmi di terminare l'intero albero dei processi\n");
	kill(hmiPID, SIGUSR1); //comunico alla hmi di terminare l'intero albero di processi, lei inclusa
}

// metodi di utility

int createSocketConnection(char *socketName) {
    int socketFd = 0, serverLength = 0;
    struct sockaddr_un serverAddressUnix;
    struct sockaddr* serverSockAddrPtr;
    serverSockAddrPtr = (struct sockaddr*) &serverAddressUnix;
    serverLength = sizeof (serverAddressUnix);
    // creazione della socket
    socketFd = socket (AF_UNIX, SOCK_STREAM, 0);
    serverAddressUnix.sun_family = AF_UNIX; /* Server domain */
    strcpy (serverAddressUnix.sun_path, socketName);/*Server name*/
    int result = connect(socketFd, serverSockAddrPtr, serverLength);
    if (result < 0)
    {
        return result;
    }
    return socketFd;
}

int checkFfrWarning(unsigned char *data) {
	const unsigned char warnings[][2] = {
		{0xA0, 0x0F},
		{0xB0, 0x72},
		{0x2F, 0xA8},
		{0x83, 0x59},
		{0xCE, 0x23}
	};
	for(int i = 0; i < 5; i++) {
		if(isInByteArray(warnings[i], data, 24) == 1) {
			return 1;
		}
	}
	return 0;
}

int readFromSocket (int fd, char *str) {
	int n;
	do {
		n = read (fd, str, 1);
	} while (n > 0 && *str++ != '\0');
return (n > 0);
}

void openFile(char filename[], char mode[], FILE **filePointer) {
	*filePointer = fopen(filename, mode);
	if (*filePointer == NULL) {
		printf("Errore nell'apertura del file");
		exit(1);
	}
}

int extractString(char* data) {//estrae l'incremento di velocita' dall'input inviato dall'ECU
	char* updatedSpeed;
	updatedSpeed = strtok (data," ");
	if(strcmp(updatedSpeed, "#") == 0){
		updatedSpeed = strtok (NULL, " ");
		return atoi(updatedSpeed);
	}else
		return -1;
}

int readFromPipe (int pipeFd, char *data) {
	int n;
	do {
		n = read (pipeFd, data, 1);
	} while (n > 0 && *data++ != '\0');
return (n > 0);
}

int isInByteArray(unsigned char *len2toFind, unsigned char *toSearch, int 		toSearchLen) {
	int toFindLen = 2;
	//printf("Lunghezza: sizeof(toSearch) = %ld\n", sizeof(toSearch));
	//printf("Lunghezza: sizeof(toSearch[0]) = %ld\n", sizeof(toSearch[0]));
	//printf("Lunghezza: %d\n", toSearchLen);
	for (int i = 0; i + toFindLen <= toSearchLen; i++) {
		if (len2toFind[0] == toSearch[i] && len2toFind[1] == toSearch[i+1]) {
			return 1;
		}
	}
	return 0;
}

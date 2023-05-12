//
// Created by jonathan on 09/05/23.
// DA FINIRE
//

#include <signal.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>

// ecu client e server arrays
int ecuServerSocketArray[5];
int ecuClientSocketArray[3]; // throttle control = 0, brake by wire = 1, steer by wire = 2

// dichiarazioni PID hmi
pid_t hmiPID;
pid_t fatherPID;

// PID attuatori
pid_t attuatoriPid[3]; // 3 attuatori (steer by wire, throttle control, brake by wire

// PID sensori
pid_t sensoriPid[3]; // 3 sensori (
pid_t frontWindshieldCameraClientPid;
pid_t forwardFacingRadarClientPid;

int createSocketConnection(char *socketName);

void serverStart();

int main(int argc, char *argv[])
{
    // primo segnale
    //signal(SIGUSR1, sigstartHandler);
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
    // ognuno di lunghezza due per un file in lettura e l'altro in scrittura
    int PIPE_server_to_frontwindshieldcamera_manager[2];
    int PIPE_server_to_forwardfacingradar_manager[2];
    int PIPE_server_to_hmi_manager[2];
    //int PIPE_server_to_bs_manager[2]; // non esiste questo sensore
    int PIPE_server_to_parkassist_manager[2];

    pipe(PIPE_server_to_frontwindshieldcamera_manager);
    pipe(PIPE_server_to_forwardfacingradar_manager);
    pipe(PIPE_server_to_hmi_manager);
    //pipe(PIPE_server_to_bs_manager);
    pipe(PIPE_server_to_parkassist_manager);

    printf("Sistema inizializzato.\n\n");
    int indexAttuatori = 0;
    int indexSensori = 0;
    attuatoriPid[indexAttuatori] = fork();

    int READ = 0;
    int WRITE = 1;

    if (attuatoriPid[indexAttuatori] == 0) // 0, child steer by wire
    {
        // chiudi tutte le pipe
        //close(PIPE_server_to_bs_manager[READ]);
        //close(PIPE_server_to_bs_manager[WRITE]);
        close(PIPE_server_to_forwardfacingradar_manager[READ]);
        close(PIPE_server_to_forwardfacingradar_manager[WRITE]);
        close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
        close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
        close(PIPE_server_to_parkassist_manager[READ]);
        close(PIPE_server_to_parkassist_manager[WRITE]);
        close(PIPE_server_to_hmi_manager[READ]);
        close(PIPE_server_to_hmi_manager[WRITE]);

        // viene passato il comando tramite exec ed eseguito lo steer by wire
        argv[0] = "./steerbywire";
        execv(argv[0],argv);
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
            close(PIPE_server_to_forwardfacingradar_manager[READ]);
            close(PIPE_server_to_forwardfacingradar_manager[WRITE]);
            close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
            close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
            close(PIPE_server_to_parkassist_manager[READ]);
            close(PIPE_server_to_parkassist_manager[WRITE]);
            close(PIPE_server_to_hmi_manager[READ]);
            close(PIPE_server_to_hmi_manager[WRITE]);
            argv[0] = "./throttlebycontrol";
            execv(argv[0],argv);
        }
        else
        {
            indexAttuatori++; // 2
            attuatoriPid[indexAttuatori] = fork();
            if (attuatoriPid[indexAttuatori] == 0) // child brake by wire
            {
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
                    close(PIPE_server_to_forwardfacingradar_manager[WRITE]);
                    close(PIPE_server_to_forwardfacingradar_manager[READ]);
                    close(PIPE_server_to_parkassist_manager[WRITE]);
                    close(PIPE_server_to_parkassist_manager[READ]);
                    //close(PIPE_server_to_bs_manager[WRITE]);
                    //close(PIPE_server_to_bs_manager[READ]);
                    close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
                    // ecuFwcClient(); qua sotto
                    while((ecuClientSocketArray[0] = createSocketConnection("throttoleControlSocket")) < 0) //attende di connettersi alle socket degli attuatori
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
                        if (strcmp(dataReceived, "SINISTRA")==0 || strcmp(dataReceived, "DESTRA") ==0)
                        {
                            printf("\ncommand", "%s", dataReceived);
                            // invio dati a steer by wire
                            write(ecuClientSocketArray[2], dataReceived, strlen(dataReceived) + 1);
                        }
                        else if (strcmp(dataReceived, "PERICOLO") == 0)
                        {
                            // fai qualcos'altro (da gestire il pericolo)
                        }
                        else
                        {
                            int newSpeed = atoi(dataReceived); // converte da stringa a intero
                            if (newSpeed < speed)
                            {
                                printf(command, "%s%d", "FRENA", (speed - newSpeed));
                                speed = newSpeed;
                                write(ecuClientSocketArray[1], command, strlen(command) + 1);
                            }
                            else if (newSpeed > speed)
                            {
                                printf(command, "%s%d", "INCREMENTO", (speed + newSpeed));
                                write(ecuClientSocketArray[0], command, strlen(command) + 1);
                            }
                            char updatedSpeed[10];
                            printf(updatedSpeed, "%s %d", "#", newSpeed);
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
                        close(PIPE_server_to_hmi_manager[WRITE]);
                        close(PIPE_server_to_hmi_manager[READ]);
                        close(PIPE_server_to_forwardfacingradar_manager[WRITE]);
                        close(PIPE_server_to_forwardfacingradar_manager[READ]);
                        close(PIPE_server_to_parkassist_manager[WRITE]);
                        close(PIPE_server_to_parkassist_manager[READ]);
                        close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
                        close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
                        argv[0] = "./frontwildshieldcamera";
                        execv(argv[0], argv);
                        //return;
                    }
                    else
                    {
                        indexSensori++; // 1
                        sensoriPid[indexSensori] = fork();
                        if (sensoriPid[indexSensori] == 0) // forwarding racing radar
                        {
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
                            indexSensori++; // 2 // park assist
                            sensoriPid[indexSensori] = fork();
                            if (sensoriPid[indexSensori] == 0) // forwarding facing radar
                            {
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
                                    close(PIPE_server_to_hmi_manager[WRITE]);
                                    close(PIPE_server_to_hmi_manager[READ]);
                                    close(PIPE_server_to_parkassist_manager[WRITE]);
                                    close(PIPE_server_to_parkassist_manager[READ]);
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
                                        return (n > 0);

                                        if(checkFfrWarning(data) == 1)
                                        {
                                            managePericoloToBbw();
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


void managePericoloToBbw()
{
	kill(attuatoriPid[2], SIGUSR1);
	waitpid(attuatoriPid[2], NULL, 0);
	kill(hmiPID, SIGUSR2); //comunico alla hmi di terminare l'intero albero di processi, lei esclusa, e di riavviare il sistema
}

// ARRIVATO FINO A QUI. ( CONTINUARE CON LA ECU)
void serverStart()
{
    int socketFileDescriptor, clientFileDescriptor;

    for(int i = 0; i < SERVER_SOCKET_NUMBER; i++){
        listenersPid[i] = fork();
        if(listenersPid[i] == 0){
            ecuServerSocketArray[i] = socket (AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
            strcpy (serverUNIXAddress.sun_path, sockets_name[i]);
            unlink (sockets_name[i]);
            bind (ecuServerSocketArray[i], serverSockAddrPtr, serverLen);
            listen (ecuServerSocketArray[i], 1);
            while (1) {/* Loop forever */ /* Accept a client connection */
                socketFd = accept (ecuServerSocketArray[i], clientSockAddrPtr, &clientLen);
                char str[200];
                while(readFromSocket(socketFd, str)){
                    if(strcmp(sockets_name[i], "fwcSocket")== 0){//controlla che figlio è
                        write(PIPE_server_to_fwc_manager[WRITE], str, strlen(str)+1);
                    } //Invia alla pipe che gestisce gli fwc.
                    else if(strcmp(sockets_name[i], "ffrSocket") == 0){
                        write(PIPE_server_to_ffr_manager[WRITE], str, strlen(str)+1);
                    }
                    else if(strcmp(sockets_name[i], "bsSocket") == 0){
                        write(PIPE_server_to_bs_manager[WRITE], str, strlen(str)+1);
                    }else if(strcmp(sockets_name[i], "paSocket") == 0){
                        write(PIPE_server_to_pa_manager[WRITE], str, strlen(str)+1);
                    }
                }
                close (socketFd); /* Close the socket */
                exit (0); /* Terminate */
            }
        }
    }
    close(PIPE_server_to_ffr_manager[WRITE]);
    close(PIPE_server_to_ffr_manager[READ]);
    close(PIPE_server_to_fwc_manager[WRITE]);
    close(PIPE_server_to_fwc_manager[READ]);
    close(PIPE_server_to_bs_manager[WRITE]);
    close(PIPE_server_to_bs_manager[READ]);
    close(PIPE_server_to_pa_manager[WRITE]);
    close(PIPE_server_to_pa_manager[READ]);
    //hmiCommunication(); da qui in poi copia e incolla dell'altro codice

    FILE *ecuLog;
    char dataReceived[200];
    int updatedSpeed;
    openFile("ECU.log", "a", &ecuLog);
    while(readFromPipe(PIPE_server_to_hmi_manager[READ], dataReceived)){
        char receivedSpeed[20];
        sprintf(receivedSpeed, "%s", dataReceived);
        if((updatedSpeed = extractString(receivedSpeed)) > -1){
            currentSpeed = updatedSpeed;
        }else{
            fprintf(ecuLog, "%s\n", dataReceived);
            fflush(ecuLog);
        }
    }

}


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

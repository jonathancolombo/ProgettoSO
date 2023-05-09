//
// Created by jonathan on 09/05/23.
//

#include <signal.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>

// ecu client e server arrays
int ecuServerSocketArray[5];
int ecuClientSocketArray[3]; // throttle control = 0, brake by wire = 1, steer by wire = 2

// dichiarazioni PID hmi
pid_t hmiPID;
pid_t fatherPID;

// PID attuatori
pid_t attuatoriPid[3]; // 3 attuatori (steer by wire, throttle control, brake by wire

// PID sensori
pid_t frontWindshieldCameraClientPid;

int createSocketConnection(char *socketName);

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
    int PIPE_server_to_ffr_manager[2];
    int PIPE_server_to_hmi_manager[2];
    //int PIPE_server_to_bs_manager[2];
    int PIPE_server_to_pa_manager[2];

    pipe(PIPE_server_to_frontwindshieldcamera_manager);
    pipe(PIPE_server_to_ffr_manager);
    pipe(PIPE_server_to_hmi_manager);
    //pipe(PIPE_server_to_bs_manager);
    pipe(PIPE_server_to_pa_manager);

    printf("Sistema inizializzato.\n\n");
    int index = 0;
    attuatoriPid[index] = fork();

    if (attuatoriPid[index] == 0) // 0, child steer by wire
    {
        // chiudi tutte le pipe
        int READ = 0;
        int WRITE = 1;
        //close(PIPE_server_to_bs_manager[READ]);
        //close(PIPE_server_to_bs_manager[WRITE]);
        close(PIPE_server_to_ffr_manager[READ]);
        close(PIPE_server_to_ffr_manager[WRITE]);
        close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
        close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
        close(PIPE_server_to_pa_manager[READ]);
        close(PIPE_server_to_pa_manager[WRITE]);
        close(PIPE_server_to_hmi_manager[READ]);
        close(PIPE_server_to_hmi_manager[WRITE]);

        // viene passato il comando tramite exec ed eseguito lo steer by wire
        argv[0] = "./steerbywire";
        execv(argv[0],argv);
    }
    else
    {
        index++; // 1
        attuatoriPid[index] = fork();
        if (attuatoriPid[index] == 0) // throttle by control
        {
            // chiudi tutte le pipe
            int READ = 0;
            int WRITE = 1;
            //close(PIPE_server_to_bs_manager[READ]);
            //close(PIPE_server_to_bs_manager[WRITE]);
            close(PIPE_server_to_ffr_manager[READ]);
            close(PIPE_server_to_ffr_manager[WRITE]);
            close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
            close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
            close(PIPE_server_to_pa_manager[READ]);
            close(PIPE_server_to_pa_manager[WRITE]);
            close(PIPE_server_to_hmi_manager[READ]);
            close(PIPE_server_to_hmi_manager[WRITE]);
            argv[0] = "./throttlebycontrol";
            execv(argv[0],argv);
        }
        else
        {
            index++; // 2
            attuatoriPid[index] = fork();
            if (attuatoriPid[index] == 0) // child brake by wire
            {
                int READ = 0;
                int WRITE = 1;
                //close(PIPE_server_to_bs_manager[READ]);
                //close(PIPE_server_to_bs_manager[WRITE]);
                close(PIPE_server_to_ffr_manager[READ]);
                close(PIPE_server_to_ffr_manager[WRITE]);
                close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
                close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
                close(PIPE_server_to_pa_manager[READ]);
                close(PIPE_server_to_pa_manager[WRITE]);
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
                    int READ = 0;
                    int WRITE = 1;
                    close(PIPE_server_to_ffr_manager[WRITE]);
                    close(PIPE_server_to_ffr_manager[READ]);
                    close(PIPE_server_to_pa_manager[WRITE]);
                    close(PIPE_server_to_pa_manager[READ]);
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

                    while(1){
                        read(PIPE_server_to_frontwindshieldcamera_manager[READ], dataReceived, 100);
                        elaborateFwcData(dataReceived);
                }



            }
        }

    }


    pause(); //blocca il processo
    //la ecu esegue le sue funzioni normali con &argv[1]


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

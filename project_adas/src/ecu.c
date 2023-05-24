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
#include <time.h>
#include <sys/stat.h>


#define DEFAULT_PROTOCOL 0
// ecu client e server arrays
int ecuServerSocketArray[4];
int ecuClientSocketArray[3]; // throttle control = 0, brake by wire = 1, steer by wire = 2


// PID attuatori
pid_t attuatoriPid[3]; // 3 attuatori (steer by wire, throttle control, brake by wire

// PID sensori
pid_t sensoriPid[3]; // 3 sensori (front windshield camera, front facing radar, park assist)

pid_t frontWindshieldCameraClientPid;
pid_t forwardFacingRadarClientPid;
pid_t listenersPid[5]; // 5 socket number

// socket server
struct sockaddr_un ecuServerAddress;
struct sockaddr *ptrecuServerAddress;

// socket client
struct sockaddr_un clientAddress;
struct sockaddr *ptrClientAddress;

// lunghezza client e server
int serverLength = 0;
int clientLength = 0;

int READ = 0;
int WRITE = 1;
int currentSpeed = 0;

//
char sockets_name[][50] = {"forwardfacingradarSocket", "parkassistSocket", "frontwindshieldcameraSocket", "surroundviewcameraSocket"};

// inizializzazione pipe
// ognuno di lunghezza due per un file in lettura e l'altro in scrittura
int PIPE_server_to_frontwindshieldcamera_manager[2];
int PIPE_server_to_forwardfacingradar_manager[2];
int PIPE_server_to_hmi_manager[2];
int PIPE_server_to_parkassist_manager[2];

int pipeArray[2];
// file ECU.log
FILE *ecuLog;

int createSocketConnection(char *socketName);
int checkFfrWarning(unsigned char *data);
void serverStart();
int readFromSocket(int fd, char *str);
void openFile(char filename[], char mode[], FILE **filePointer);
int extractString(char *data);
int readFromPipe(int pipeFd, char *data);
int isInByteArray(unsigned char *len2toFind, unsigned char *toSearch, int toSearchLen);
int createPipe(char *pipeName);
int openPipeOnRead(char *pipeName);
void sigstartHandler();
void sigparkHandler();
void sigParkCallHandler();
void sigParkEndHandler();
void managePericoloToBrakeByWire();

int inputPid;

int main(int argc, char *argv[])
{
    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("PROCESSO ECU\n");
    
    // imposto il file di Log della ECU
    printf("Cerco di aprire ECU.log\n");

    ecuLog = fopen("ECU.log", "w+");

    if (ecuLog == NULL)
    {
        printf("Errore sull'apertura del file ECU.log\n");
        perror("open file error!\n");
        exit(EXIT_FAILURE);
    }

    // inizializzo la velocità e la imposto sul file di LOG
    int speed = 0;
    printf("Inserisco 0 su ECU.log\n");
    fprintf(ecuLog, "%d\n", speed);
    // fflush(ecuLog);
    printf("Chiudo il file di Log della ECU e stampo lo stato della close: %d\n", fclose(ecuLog)); // chiusura file ECU.log


    // crea la pipe e la comunicazione con l'hmi
    int hmiFileDescriptor = createPipe("../ecuToHmiPipe");
    int hmiInputFileDescriptor = openPipeOnRead("../../ipc/hmiInputToEcuPipe");
    read(hmiInputFileDescriptor, &inputPid, sizeof(int));

    // prevedere dei controlli per la configurazione del rilancio del sistema

    // genero tutti i processi figli della ECU (attuatori e sensori)
    // inizializzo gli attuatori
    int indexAttuatori = 0; // 3 attuatori (steer by wire, throttle control, brake by wire

    attuatoriPid[indexAttuatori] = fork(); // genero steer by wire

    if (attuatoriPid[indexAttuatori] < 0) // 0, figlio steer by wire
    {
        perror("Fork error\n");
        printf("Processo steer by wire non generato correttamente\n");
        exit(EXIT_FAILURE);
    } 
    else
    {
        printf("Eseguo steer by wire con una execv\n");
        sleep(1);
        argv[0] = "./steerbywire";
        execv(argv[0], argv);
    }

    indexAttuatori++;
    attuatoriPid[indexAttuatori] = fork(); // genero throttlecontrol

    if (attuatoriPid[indexAttuatori] < 0) // 1, figlio throttle control
    {
        perror("Fork error\n");
        printf("Processo throttle by control non generato correttamente\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Eseguo throttle by control con una execv\n");
        argv[0] = "./throttlebycontrol";
        execv(argv[0], argv);
    }

    indexAttuatori++;
    attuatoriPid[indexAttuatori] = fork(); // genero il brake by wire

    if (attuatoriPid[indexAttuatori] < 0)
    {
        perror("Fork error\n");
        printf("Processo brake by wire non generato correttamente\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Eseguo steer by wire con una execv\n");
        argv[0] = "./steerbywire";
        execv(argv[0], argv);
    }

    // inizializzo anche i sensori
    int indexSensori = 0; //3 sensori (front windshield camera, front facing radar, park assist)

    sensoriPid[indexSensori] = fork(); // 0, genero frontWindshieldCamera
    if (sensoriPid[indexSensori] < 0)
    {
        perror("Fork error\n");
        printf("Processo front windshield camera non generato correttamente\n");
        exit(EXIT_FAILURE);
    }
    else 
    {
        printf("Eseguo front windshield camera con una execv\n");
        argv[0] = "./frontwindshieldcamera";
        execv(argv[0], argv);
    }

    indexSensori++;
    sensoriPid[indexSensori] = fork(); // 1, genero front facing radar
    if (sensoriPid[indexSensori] < 0) 
    {
        perror("Fork error\n");
        printf("Processo front facing radar non generato correttamente\n");
        exit(EXIT_FAILURE);
    }
    else 
    {
        printf("Eseguo front facing radar con una execv\n");
        argv[0] = "./frontfacingradar";
        execv(argv[0], argv);
    }

    indexSensori++;
    sensoriPid[indexSensori] = fork(); // 2, genero park assist
    if (sensoriPid[indexSensori] < 0) 
    {
        perror("Fork error\n");
        printf("Processo park assist non generato correttamente\n");
        exit(EXIT_FAILURE);
    }
    else 
    {
        printf("Eseguo park assist con una execv\n");
        argv[0] = "./parkassist";
        execv(argv[0], argv);
    }

    // inizializzazione socket server
    struct sockaddr_un ecuServerAddress;
    struct sockaddr *ptrEcuServerAddress;
    ptrEcuServerAddress = (struct sockaddr *)&ecuServerAddress;
    int serverLength = sizeof(ecuServerAddress);

    // inizializzazione socket client
    struct sockaddr_un clientAddress;
    struct sockaddr *ptrClientAddress;
    ptrClientAddress = (struct sockaddr *)&clientAddress;
    int clientLength = sizeof(clientAddress);

    ecuServerAddress.sun_family = AF_UNIX;

    int ecuFileDescriptor = socket (AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
    strcpy(ecuServerAddress.sun_path, "./ecuSocket");
    unlink("./ecuSocket");
    bind(ecuFileDescriptor, ptrEcuServerAddress, serverLength);
    listen(ecuFileDescriptor, 3);

    pipe(pipeArray[2]);
    
    // clientAddress.sun_family = AF_UNIX;

    // creazione delle pipe
    /*
    printf("Creo le varie PIPE per comunicare\n");
    pipe(PIPE_server_to_frontwindshieldcamera_manager);
    pipe(PIPE_server_to_forwardfacingradar_manager);
    pipe(PIPE_server_to_hmi_manager);
    pipe(PIPE_server_to_parkassist_manager);
    */
    printf("Sistema inizializzato.\n\n");

    printf("Processo ecu bloccato da pause\n");
    pause();

    int indexAttuatori = 0;
    int indexSensori = 0;
    attuatoriPid[indexAttuatori] = fork();

    if (attuatoriPid[indexAttuatori] == 0) // 0, child steer by wire
    {
        // chiudi tutte le pipe
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
        execv(argv[0], argv);
        return;
    }
    else
    {
        indexAttuatori++; // 1
        attuatoriPid[indexAttuatori] = fork();
        if (attuatoriPid[indexAttuatori] == 0) // throttle by control
        {
            // chiudi tutte le pipe
            // close(PIPE_server_to_bs_manager[READ]);
            // close(PIPE_server_to_bs_manager[WRITE]);
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
            execv(argv[0], argv);
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
                execv(argv[0], argv);
                return;
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
                    // close(PIPE_server_to_bs_manager[WRITE]);
                    // close(PIPE_server_to_bs_manager[READ]);
                    close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
                    // ecuFwcClient(); qua sotto
                    printf("Creo le socket connection\n");
                    while ((ecuClientSocketArray[0] = createSocketConnection("throttlebycontrolSocket")) < 0) // attende di connettersi alle socket degli attuatori
                    {
                        printf("Socket in ascolto throttle by control\n");
                        usleep(100000);
                    }
                    while ((ecuClientSocketArray[1] = createSocketConnection("brakebywireSocket")) < 0)
                    {
                        printf("Socket in ascolto brake by wire\n");
                        usleep(100000);
                    }
                    while ((ecuClientSocketArray[2] = createSocketConnection("steerbywireSocket")) < 0)
                    {
                        printf("Socket in ascolto steer by wire\n");
                        usleep(100000);
                    }

                    char dataReceived[100];

                    for (;;)
                    {
                        printf("Leggo il comando ricevuto da frontwindshield camera: %s\n", dataReceived);
                        read(PIPE_server_to_frontwindshieldcamera_manager[READ], dataReceived, 100);
                        char command[30];
                        printf("Comando ricevuto %s\n", dataReceived);
                        sprintf(command, "%s", "NO COMMAND");
                        if (strcmp(dataReceived, "SINISTRA") == 0 || strcmp(dataReceived, "DESTRA") == 0)
                        {
                            printf("Comando di sterzata \n");
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
                            kill(hmiPID, SIGUSR2); // comunico alla hmi di terminare l'intero albero di processi, lei esclusa, e di riavviare il sistema
                        }
                        else
                        {
                            int newSpeed = atoi(dataReceived); // converte da stringa a intero
                            if (newSpeed < speed)
                            {
                                printf("Rallenta velocità\n");
                                sprintf(command, "%s%d", "FRENA ", (speed - newSpeed));
                                speed = newSpeed;
                                write(ecuClientSocketArray[1], command, strlen(command) + 1);
                            }
                            else if (newSpeed > speed)
                            {
                                printf("Incrementa velocità\n");
                                sprintf(command, "%s%d", "INCREMENTO ", (speed + newSpeed));
                                write(ecuClientSocketArray[0], command, strlen(command) + 1);
                            }
                            char updatedSpeed[10];
                            sprintf(updatedSpeed, "%s %d", "#", newSpeed);
                            write(PIPE_server_to_hmi_manager[WRITE], updatedSpeed, strlen(updatedSpeed) + 1);
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
                        return;
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
                            argv[0] = "./forwardfacingradar";
                            execv(argv[0], argv);
                            return;
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
                                return;
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
                                        do
                                        {
                                            n = read(PIPE_server_to_forwardfacingradar_manager[READ], data, 1);
                                        } while (n > 0);
                                        // return (n > 0);

                                        if (checkFfrWarning(data) == 1)
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
    return EXIT_SUCCESS;
}

void managePericoloToBrakeByWire()
{
    kill(attuatoriPid[2], SIGUSR1);
    waitpid(attuatoriPid[2], NULL, 0);
    kill(hmiPID, SIGUSR2); // comunico alla hmi di terminare l'intero albero di processi, lei esclusa, e di riavviare il sistema
}

// ARRIVATO FINO A QUI. (CONTINUARE CON LA ECU - BESTIAAAAAAA)
void serverStart()
{
    int socketFileDescriptor, clientFileDescriptor;

    for (int i = 0; i < 5; i++) // 5 = server socket number
    {
        listenersPid[i] = fork();
        if (listenersPid[i] == 0)
        {
            ecuServerSocketArray[i] = socket(AF_UNIX, SOCK_STREAM, 0); // 0 = default protocol
            strcpy(ecuServerAddress.sun_path, sockets_name[i]);
            unlink(sockets_name[i]);
            bind(ecuServerSocketArray[i], ptrecuServerAddress, serverLength);
            listen(ecuServerSocketArray[i], 1);

            while (1)
            { /* Loop forever */ /* Accept a client connection */
                socketFileDescriptor = accept(ecuServerSocketArray[i], ptrClientAddress, &clientLength);
                char str[200];
                while (readFromSocket(socketFileDescriptor, str))
                {
                    if (strcmp(sockets_name[i], "frontwindshieldcameraSocket") == 0)
                    { // controlla che figlio è
                        printf("Invio i dati a frontwindshield tramite socket dalla ecu\n");
                        write(PIPE_server_to_frontwindshieldcamera_manager[WRITE], str, strlen(str) + 1);
                    } // Invia alla pipe che gestisce gli fwc.
                    else if (strcmp(sockets_name[i], "forwardfacingradarSocket") == 0)
                    {
                        printf("Invio i dati a forwardfacingradar tramite socket dalla ecu\n");
                        write(PIPE_server_to_forwardfacingradar_manager[WRITE], str, strlen(str) + 1);
                    }
                    else if (strcmp(sockets_name[i], "parkassistSocket") == 0)
                    {
                        printf("Invio i dati a parkassist tramite socket dalla ecu\n");
                        write(PIPE_server_to_parkassist_manager[WRITE], str, strlen(str) + 1);
                    }
                }
                printf("Chiudo la socket ed esco\n");
                close(socketFileDescriptor); /* Close the socket */
                exit(EXIT_SUCCESS);          /* Terminate */
            }
        }
    }
    close(PIPE_server_to_forwardfacingradar_manager[WRITE]);
    close(PIPE_server_to_forwardfacingradar_manager[READ]);
    close(PIPE_server_to_frontwindshieldcamera_manager[WRITE]);
    close(PIPE_server_to_frontwindshieldcamera_manager[READ]);
    close(PIPE_server_to_parkassist_manager[WRITE]);
    close(PIPE_server_to_parkassist_manager[READ]);
    // hmiCommunication();

    char dataReceived[200];
    int updatedSpeed;

    printf("Riapro il file ECU.log\n");
    ecuLog = fopen("ECU.log", "a");

    if (ecuLog == NULL)
    {
        printf("Errore nell'apertura del file ECU.log");
        perror("open file error!\n");
        exit(EXIT_FAILURE);
    }

    printf("Rileggo i dati dalla pipe dell'hmi manager\n");
    while (readFromPipe(PIPE_server_to_hmi_manager[READ], dataReceived))
    {
        char receivedSpeed[20];
        sprintf(receivedSpeed, "%s", dataReceived);

        if ((updatedSpeed = extractString(receivedSpeed)) > -1)
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
void sigstartHandler()
{
    signal(SIGUSR1, sigparkHandler); // reset signal
    // return;
}

void sigparkHandler()
{
    printf("Avvio la procedura di parcheggio segnalando park assist\n");
    signal(SIGUSR1, sigParkCallHandler);
    char parkCommand[20];
    sprintf(parkCommand, "%s %d", "PARCHEGGIO", currentSpeed);
    // mi creo e mi connetto alla socket con brake by wire
    printf("Creo la socket dalla ecu per connettermi con brake by wire\n");
    int bbwConne = createSocketConnection("brakebywireSocket");
    printf("Mando un comando a brake by wire\n");
    write(bbwConne, parkCommand, strlen(parkCommand) + 1);

    printf("Mando il comando di scrittura con la pipe alla hmi\n");
    write(PIPE_server_to_hmi_manager[WRITE], parkCommand, strlen(parkCommand) + 1);
    printf("Faccio la kill su throttle by control e steer by wire\n");
    kill(attuatoriPid[0], SIGTERM);
    kill(attuatoriPid[1], SIGTERM);
    kill(sensoriPid[0], SIGTERM);
    kill(sensoriPid[1], SIGTERM);
}

void sigParkCallHandler()
{
    signal(SIGUSR1, sigParkEndHandler);
    kill(sensoriPid[2], SIGUSR1); // avvia procedura di PARCHEGGIO
}

void sigParkEndHandler()
{
    printf("Comunico alla hmi di terminare l'intero albero dei processi\n");
    kill(hmiPID, SIGUSR1); // comunico alla hmi di terminare l'intero albero di processi, lei inclusa
}

// metodi di utility

int createSocketConnection(char *socketName)
{
    int socketFd = 0, serverLength = 0;
    struct sockaddr_un ecuServerAddressUnix;
    struct sockaddr *serverSockAddrPtr;
    serverSockAddrPtr = (struct sockaddr *)&ecuServerAddressUnix;
    serverLength = sizeof(ecuServerAddressUnix);
    // creazione della socket
    socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    ecuServerAddressUnix.sun_family = AF_UNIX;         /* Server domain */
    strcpy(ecuServerAddressUnix.sun_path, socketName); /*Server name*/
    int result = connect(socketFd, serverSockAddrPtr, serverLength);
    if (result < 0)
    {
        printf("Socket non connessa\n");
        return result;
    }
    return socketFd;
}

int checkFfrWarning(unsigned char *data)
{
    // indirizzi di memoria da controllare durante il parcheggio
    const unsigned char warnings[][2] = 
    {
        {0x17, 0x2A},
        {0xD6, 0x93},
        {0x00, 0x00},
        {0xBD, 0xD8},
        {0xFA, 0xEE},
        {0x43, 0x00}
    };

    for (int i = 0; i < 6; i++)
    {
        if (isInByteArray(warnings[i], data, 24) == 1)
        {
            return 1;
        }
    }
    return 0;
}

int readFromSocket(int fd, char *str)
{
    int n;
    do
    {
        n = read(fd, str, 1);
    } while (n > 0 && *str++ != '\0');
    return (n > 0);
}

void openFile(char filename[], char mode[], FILE **filePointer)
{
    *filePointer = fopen(filename, mode);
    if (*filePointer == NULL)
    {
        printf("Errore nell'apertura del file");
        exit(1);
    }
}

int extractString(char *data)
{ // estrae l'incremento di velocita' dall'input inviato dall'ECU
    char *updatedSpeed;
    updatedSpeed = strtok(data, " ");
    if (strcmp(updatedSpeed, "#") == 0)
    {
        updatedSpeed = strtok(NULL, " ");
        return atoi(updatedSpeed);
    }
    else
        return -1;
}

int readFromPipe(int pipeFd, char *data)
{
    int n;
    do
    {
        n = read(pipeFd, data, 4);
    } while (n > 0 && *data++ != '\0');
    return (n > 0);
}

int isInByteArray(unsigned char *len2toFind, unsigned char *toSearch, int toSearchLen)
{
    int toFindLen = 2;
    // printf("Lunghezza: sizeof(toSearch) = %ld\n", sizeof(toSearch));
    // printf("Lunghezza: sizeof(toSearch[0]) = %ld\n", sizeof(toSearch[0]));
    // printf("Lunghezza: %d\n", toSearchLen);
    for (int i = 0; i + toFindLen <= toSearchLen; i++)
    {
        if (len2toFind[0] == toSearch[i] && len2toFind[1] == toSearch[i + 1])
        {
            return 1;
        }
    }
    return 0;
}

int createPipe(char *pipeName) 
{
    int fileDescriptor;
    unlink(pipeName);
    if (mknod(pipeName, __S_IFIFO, 0) < 0 )
    {    //Creating named pipe
        exit(EXIT_FAILURE);
    }
    chmod(pipeName, 0660);
    do {
        fileDescriptor = open(pipeName, O_WRONLY);    //Opening named pipe for write
        if(fileDescriptor == -1)
        {
            printf("%s not found. Trying again...\n", pipeName);
            sleep(1);
        }
    } while(fileDescriptor == -1);
    return fileDescriptor;
}

int openPipeOnRead(char *pipeName)
{
    int fileDescriptor;
    do
    {
        fileDescriptor = open(pipeName, O_RDONLY); // Opening named pipe for write
        if (fileDescriptor == -1)
        {
            printf("Pipename:%s non trovata. Riprova ancora...\n", pipeName);
            sleep(1);
        }
    } while (fileDescriptor == -1);
    return fileDescriptor;
}
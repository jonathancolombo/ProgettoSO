//
// Created by jonathan on 09/05/23.
// DA FINIRE
//

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <time.h>
#include <stdarg.h>
 
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

// file ECU.log
FILE *ecuLog;

int createPipe(char *pipeName);
int openPipeOnRead(char *pipeName);
int readLine(int fileDescriptor, char *str);
void writeMessageToPipe(int pipeFd, const char * format, ...);
void writeMessage(FILE *fp, const char * format, ...);
void formattedTime(char *timeBuffer);
void receiveString(int fileDescriptor, char *string);
int park(int clientFd);

int inputPid;
pid_t inputReader;

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
    attuatoriPid[indexAttuatori] = fork(); //2, genero il brake by wire

    if (attuatoriPid[indexAttuatori] < 0)
    {
        perror("Fork error\n");
        printf("Processo brake by wire non generato correttamente\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Eseguo steer by wire con una execv\n");
        argv[0] = "./brakebywire";
        execv(argv[0], argv);
    }

    // inizializzo anche i sensori
    int indexSensori = 0; // 3 sensori (front windshield camera, front facing radar, park assist)

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

    int ecuFileDescriptor = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
    strcpy(ecuServerAddress.sun_path, "./ecuSocket");
    unlink("./ecuSocket");
    bind(ecuFileDescriptor, ptrEcuServerAddress, serverLength);
    listen(ecuFileDescriptor, 3);
    int pipeArray[2];

    pipe2(pipeArray[2], O_NONBLOCK);
    FILE *hmiLog = NULL;
    int input = 0;
    char socketString[16];
    int isListening[2] = {0, 0};

    if ((inputReader = fork()) < 0)
    {
        printf("Processo d'input hmi non generato correttamente\n");
        perror("fork error!\n");
        exit(EXIT_FAILURE);
    }
    else
    { // Reads from HMI Input terminal

        close(pipeArray[READ]);
        for (;;)
        {
            //input = getInput(hmiInputFileDescriptor, hmiFileDescriptor, hmiLog);
            char command[32];
            if (readLine(hmiInputFileDescriptor, command) == 0)
            {
                if (strcmp(command, "ARRESTO") == 0)
                    input = 1;
                else if (strcmp(command, "INIZIO") == 0)
                    input =  2;
                else if (strcmp(command, "PARCHEGGIO") == 0)
                    input = 3;
                    input = -1;
            }

            write(pipeArray[WRITE], &input, sizeof(int));
            if (input == 3)
            {
                write(hmiFileDescriptor, "PARCHEGGIO", strlen("PARCHEGGIO") + 1);
                close(pipeArray[WRITE]);
                exit(EXIT_SUCCESS);
            }
        }
    }
    // creazione delle comunicazioni pipe da svolgere

    printf("Sistema inizializzato.\n\n");

    printf("Processo ecu bloccato da pause\n");
    
    int fileDescriptorThrottle = createPipe("../throttlebycontrol");
    int fileDescriptorSteer = createPipe("../steerbywire");
    int fileDescriptorBrake = createPipe("../brakebywire");

    close(pipeArray[WRITE]);

    for (;;)
    {
        read(pipeArray[READ], &input, sizeof(int));

        if (input == 1 || strcmp(socketString, "PERICOLO") == 0)
        {
            writeMessageToPipe(hmiFileDescriptor, "ARRESTO AUTO");
            kill(attuatoriPid[2], SIGTSTP); // segnalo brake by wire
            speed = 0;
            isListening[0] = 0;
            isListening[1] = 0;
        }
        else if (input == 2 && strcmp(socketString, "PARCHEGGIO") != 0)
        {
            kill(attuatoriPid[2], SIGCONT);
            //stopFlag = 0;   // Waiting for the next stop to be called
            isListening[0] = 1;
            isListening[1] = 0;
        }
        else if (input == 3 || strcmp(socketString, "PARCHEGGIO") == 0)
        {
            while (speed > 0)
            {
                writeMessage(ecuLog, "FRENO 5");
                writeMessageToPipe(hmiFileDescriptor, "FRENO 5");
                writeMessageToPipe(fileDescriptorBrake, "FRENO 5");
                speed = speed - 5;
                sleep(1);
            }
            
            isListening[0] = 0;
            isListening[1] = 1;
            // stoppo tutti i processi
            kill(attuatoriPid[0], SIGSTOP); // stoppo lo sterzo
            kill(attuatoriPid[1], SIGSTOP); // stoppo l'acceleratore
            kill(attuatoriPid[2], SIGSTOP); // stoppo la frenata
            kill(sensoriPid[0], SIGSTOP); // stoppo il frontwindshield camera
            kill(sensoriPid[1], SIGSTOP); // stoppo il radar

            sensoriPid[2] = fork(); // 2, genero park assist
            if (sensoriPid[2] < 0)
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
        }

        int clientFileDescriptor = accept(ecuFileDescriptor, ptrClientAddress, &clientLength);
        int sensor;
        while(read(clientFileDescriptor, &sensor, sizeof(int)) < 0)
        {

        }
        while(send(clientFileDescriptor, &isListening[sensor], sizeof(int), 0) < 0)
        {

        }

        // CONTINUARE DA QUI
        memset(socketString, '\0', 16);

        if (sensor == 0 && isListening[0] == 1)
        {
            printf("Pronto a ricevere le stringhe di dati\n");
            receiveString(clientFileDescriptor, socketString);
            
            if (strcmp(socketString, "SINISTRA") == 0 || strcmp(socketString, "DESTRA") == 0)
            {
                int count = 0;
                for (int count = 0; count < 4; count++)
                {
                    printf("Comando di sterzata\n");
                    writeMessageToPipe(fileDescriptorSteer, "%s", socketString);
                    writeMessage(ecuLog, "STERZATA A %s", socketString);
                    writeMessageToPipe(hmiFileDescriptor, "STERZATA A %s", socketString);
                    read(pipeArray[READ], &input, sizeof(int));

                    if (input != 2)
                    {
                        break;
                    }

                }
            }
            else
            {
                int newSpeed = atoi(socketString);

                if (newSpeed < speed)
                {
                    printf("Frena\n");
                    while (newSpeed < speed)
                    {
                        read(pipeArray[READ], &input, sizeof(int));
                        if (input != 2)
                        {
                            break;
                        }
                        writeMessage(ecuLog, "FRENO 5");
                        writeMessageToPipe(hmiFileDescriptor, "FRENO 5");
                        writeMessageToPipe(fileDescriptorBrake, "FRENO 5");
                        speed = speed - 5;
                        sleep(1);
                    }
                }

                if (newSpeed > speed)
                { // Throttle
                printf("Accelera\n");
                    while (newSpeed > speed)
                    {
                        read(pipeArray[READ], &input, sizeof(int));
                        if (input != 2)
                        {
                            break;
                        }
                        writeMessage(ecuLog, "INCREMENTO 5");
                        writeMessageToPipe(hmiFileDescriptor, "INCREMENTO 5");
                        writeMessageToPipe(fileDescriptorThrottle, "INCREMENTO 5");
                        speed = speed + 5;
                        sleep(1);
                    }
                }
            }
        }

        if (sensor == 1 && isListening[1] == 1)
        {
            printf("Parcheggio \n");
            if (park(clientFileDescriptor) != 0)
            {
                close(clientFileDescriptor);
                break;
            }
            close(clientFileDescriptor);
        }
    }

    close(pipeArray[READ]);
    fclose(ecuLog);
    close(hmiInputFileDescriptor);
    close(hmiFileDescriptor);
    kill(attuatoriPid[0], SIGINT); // sigint su steer by wire
    kill(attuatoriPid[1], SIGINT); // sigint su throttle control
    kill(attuatoriPid[2], SIGTSTP); // stoppo brake by wire
    kill(sensoriPid[0], SIGINT); // sigint su front windshield camera
    kill(sensoriPid[1], SIGINT); // sigint su front facing radar
    kill(sensoriPid[2], SIGINT);
    kill(inputPid, SIGINT);
    unlink("../ipc/ecuSocket");
    unlink("../ipc/throttlePipe");
    unlink("../ipc/steerPipe");
    unlink("../ipc/brakePipe");
    unlink("../ipc/ecuToHmiPipe");
    exit(EXIT_SUCCESS);
}


int createPipe(char *pipeName)
{
    int fileDescriptor;
    unlink(pipeName);
    if (mknod(pipeName, __S_IFIFO, 0) < 0)
    { // Creating named pipe
        exit(EXIT_FAILURE);
    }
    chmod(pipeName, 0660);
    do
    {
        fileDescriptor = open(pipeName, O_WRONLY); // Opening named pipe for write
        if (fileDescriptor == -1)
        {
            printf("%s not found. Trying again...\n", pipeName);
            sleep(1);
        }
    } while (fileDescriptor == -1);
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

int readLine(int fileDescriptor, char *str)
{
	int n;
	do {
		n = read (fileDescriptor, str, 1);
	} while (n > 0 && *str++ != '\0');
    return (n > 0);
}

void writeMessageToPipe(int pipeFd, const char * format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);
    write(pipeFd, buffer, strlen(buffer)+1);
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

void receiveString(int fileDescriptor, char *string)
{
    do
    {
        while(read(fileDescriptor, string, 1) < 0)
            sleep(1);
    } while(*string++ != '\0');
}


int park(int clientFd)
{ // PARKING METHOD
    char str[8];
    int result = 1;
    for (int count = 0; count < 120; count++)
    {
        receiveString(clientFd, str);
        result *= strcmp(str, "0x172a");
        result *= strcmp(str, "0xd693");
        result *= strcmp(str, "0x0000");
        result *= strcmp(str, "0xbdd8");
        result *= strcmp(str, "0xfaee");
        result *= strcmp(str, "0x4300");
        if (result != 0)
            result = 1;
    }
    return result;
}
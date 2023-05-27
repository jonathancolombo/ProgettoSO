//
// Created by jonathan on 09/05/23.
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

#include "functions.h"

 
#define DEFAULT_PROTOCOL 0

// PID attuatori
int attuatoriPid[3]; // 3 attuatori (steer by wire, throttle control, brake by wire

// PID sensori
int sensoriPid[3]; // 3 sensori (front windshield camera, front facing radar, park assist)


void receiveString(int fileDescriptor, char *string);
int park(int clientFd);
void startProcess(char **mode);
void startParkAssist(char **mode);

int inputPid;
int speed;

/*
int pidWithArgs[2]; //THROTTLE CONTROL, PARK ASSIST
int pidWithoutArgs[3];  //STEER BY WIRE, BRAKE BY WIRE, FRONT WINDHSIELD CAMERA
*/
void endProgram(int __sig) {    //ENDING PROGRAM WITH DESIRED SIGNAL
    kill(attuatoriPid[2], SIGTSTP);

    kill(attuatoriPid[1],__sig);
    kill(sensoriPid[2],__sig);

    kill(attuatoriPid[0],__sig);
    kill(attuatoriPid[2],__sig);
    kill(sensoriPid[0],__sig);
    
    kill(inputPid, SIGINT);
    unlink("./ecuSocket");
    unlink("./throttlePipe");
    unlink(".steerPipe");
    unlink(".brakePipe");
    unlink("./ecuToHmiPipe");
}


void throttleFailure() {    //HANDLING THROTTLE FAILURE
    speed = 0;
    kill(attuatoriPid[2], SIGTSTP);
    kill(getppid(), SIGUSR1);
    endProgram(SIGUSR1);
    exit(EXIT_FAILURE);
}

int getInput(int hmiInputFd, int hmiFd, FILE *log)
{ // GET INPUT FROM HMI INPUT
    char str[32];
    if (readline(hmiInputFd, str) == 0)
    {
        if (strcmp(str, "ARRESTO") == 0)
            return 1;
        else if (strcmp(str, "INIZIO") == 0)
            return 2;
        else if (strcmp(str, "PARCHEGGIO") == 0)
            return 3;
        return -1;
    }
}

int isNumber(char *str)
{ // CHECKING IF STRING IS A NUMBER
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (!isdigit(str[i]))
            return 0;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    printf("PROCESSO ECU\n");

    // imposto il file di Log della ECU
    printf("Cerco di aprire ECU.log\n");
    FILE *ecuLog;
    ecuLog = fopen("ecu.log", "w");

    if (ecuLog == NULL)
    {
        printf("Errore sull'apertura del file ecu.log\n");
        perror("open file error!\n");
        exit(EXIT_FAILURE);
    }

    int stopFlag = 0;
    int input = 0;
    int inputReader;

    struct sockaddr_un ecuServerAddress;
    struct sockaddr *ptrEcuServerAddress;

    struct sockaddr_un clientAddress;
    struct sockaddr *ptrClientAddress;

    printf("Creo le pipe per la comunicazione hmi-ecu\n");
    int hmiFileDescriptor = createPipe("./ecuToHmiPipe");
    int hmiInputFileDescriptor = openPipeOnRead("./hmiInputToEcuPipe");
    
    read(hmiInputFileDescriptor, &inputPid, sizeof(int));

    // prevedere dei controlli per la configurazione del rilancio del sistema
    signal(SIGUSR1, throttleFailure);

    // genero tutti i processi figli della ECU (attuatori e sensori)
    // inizializzo gli attuatori
    printf("Inizializzo i processi\n");
    startProcess(&argv[1]);

    
    printf("Inizializzo la socket\n");

     // inizializzazione socket server
    ptrEcuServerAddress = (struct sockaddr *)&ecuServerAddress;
    int serverLength = sizeof(ecuServerAddress);

    // inizializzazione socket client
    
    ptrClientAddress = (struct sockaddr *)&clientAddress;
    int clientLength = sizeof(clientAddress);

    int ecuFileDescriptor = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
    ecuServerAddress.sun_family = AF_UNIX;
    strcpy(ecuServerAddress.sun_path, "./ecuSocket");
    unlink("./ecuSocket");
    bind(ecuFileDescriptor, ptrEcuServerAddress, serverLength);
    listen(ecuFileDescriptor, 3);
    int pipeArray[2];

    pipe2(pipeArray, O_NONBLOCK);

    FILE *hmiLog = NULL;
    char socketString[16];
    int isListening[2] = {0, 0};

    if ((inputReader = fork()) < 0)
    {
        printf("Processo d'input hmi non generato correttamente\n");
        perror("fork error!\n");
        exit(EXIT_FAILURE);
    }
    else if (inputReader == 0)
    { // Reads from HMI Input terminal

        close(pipeArray[READ]);
        for (;;)
        {
            input = getInput(hmiInputFileDescriptor, hmiFileDescriptor, ecuLog);
            //input = getInput(hmiInputFileDescriptor, hmiFileDescriptor, hmiLog);
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

    int fileDescriptorThrottle = createPipe("./throttlePipe");
    int fileDescriptorSteer = createPipe("./steerPipe");
    int fileDescriptorBrake = createPipe("./brakePipe");

    close(pipeArray[WRITE]);

    for (;;)
    {
        read(pipeArray[READ], &input, sizeof(int));

        
        if ((input == 1 || strcmp(socketString, "PERICOLO") == 0) && stopFlag == 0 && strcmp(socketString, "PARCHEGGIO") != 0)
        {
            writeMessageToPipe(hmiFileDescriptor, "ARRESTO AUTO");
            kill(attuatoriPid[2], SIGTSTP); // segnalo brake by wire
            stopFlag = 1;
            speed = 0;
            isListening[0] = 0;
            isListening[1] = 0;
        }
        else if (input == 2 && strcmp(socketString, "PARCHEGGIO") != 0)
        {
            kill(attuatoriPid[2], SIGCONT);
            stopFlag = 0;   // Waiting for the next stop to be called
            isListening[0] = 1;
            isListening[1] = 0;
        }
        else if (input == 3 || strcmp(socketString, "PARCHEGGIO") == 0)
        {
            input = 3;
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
                startParkAssist(&argv[1]);
            }
        }

        int clientFileDescriptor = accept(ecuFileDescriptor, ptrClientAddress, &clientLength);
        int sensor;
        while(recv(clientFileDescriptor, &sensor, sizeof(int), 0) < 0);
        while(send(clientFileDescriptor, &isListening[sensor], sizeof(int), 0) < 0);

        // CONTINUARE DA QUI
        memset(socketString, '\0', 16);

        if (sensor == 0 && isListening[0] == 1)
        {
            //printf("Pronto a ricevere le stringhe di dati\n");
            receiveString(clientFileDescriptor, socketString);
            
            if (strcmp(socketString, "SINISTRA") * strcmp(socketString, "DESTRA") == 0)
            {
                int count = 0;
                while(count < 4)
                {
                    //printf("Comando di sterzata\n");
                    writeMessageToPipe(fileDescriptorSteer, "%s", socketString);
                    writeMessage(ecuLog, "STERZATA A %s", socketString);
                    writeMessageToPipe(hmiFileDescriptor, "STERZATA A %s", socketString);
                    read(pipeArray[READ], &input, sizeof(int));

                    if (input != 2)
                    {
                        break;
                    }
                    sleep(1);
                    count++;
                }
            }
            else
            {
                int newSpeed = atoi(socketString);

                if (newSpeed < speed)
                {
                    //printf("Frena\n");
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
                    //printf("Accelera\n");
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
            //printf("Parcheggio \n");
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
    unlink("./ecuSocket");
    unlink("./throttlePipe");
    unlink("./steerPipe");
    unlink("./brakePipe");
    unlink("./ecuToHmiPipe");
    unlink("./hmiInputToEcuPipe");
    exit(EXIT_SUCCESS);
}

void startProcess(char **mode)
{
    //pid_t attuatoriPid[3];
    //pid_t sensoriPid[3];

    attuatoriPid[1] = fork(); // Genera throttle control

    if (attuatoriPid[1] < 0) {
        perror("Fork error\n");
        printf("Processo throttle by control non generato correttamente\n");
        exit(EXIT_FAILURE);
    } else if (attuatoriPid[1] == 0) {
        printf("Eseguo throttle by control con una execv\n");
        char *argv[] = {"./throttlebycontrol", *mode, NULL};
        execv(argv[0], argv);
        perror("Errore durante l'esecuzione di execv\n");
        exit(EXIT_FAILURE);
    }

    sensoriPid[1] = fork(); // Genera forward facing radar
    if (sensoriPid[1] < 0) {
        perror("Fork error\n");
        printf("Processo forward facing radar non generato correttamente\n");
        exit(EXIT_FAILURE);
    } else if (sensoriPid[1] == 0) {
        printf("Eseguo forward facing radar con una execv\n");
        char *argv[] = {"./forwardfacingradar", *mode, NULL};
        execv(argv[0], argv);
        perror("Errore durante l'esecuzione di execv\n");
        exit(EXIT_FAILURE);
    }

    attuatoriPid[0] = fork(); // Genera steer by wire

    if (attuatoriPid[0] < 0) {
        perror("Fork error\n");
        printf("Processo steer by wire non generato correttamente\n");
        exit(EXIT_FAILURE);
    } else if (attuatoriPid[0] == 0) {
        printf("Eseguo steer by wire con una execv\n");
        char *argv[] = {"./steerbywire", *mode, NULL};
        execv(argv[0], argv);
        perror("Errore durante l'esecuzione di execv\n");
        exit(EXIT_FAILURE);
    }

    attuatoriPid[2] = fork(); // Genera brake by wire

    if (attuatoriPid[2] < 0) {
        perror("Fork error\n");
        printf("Processo brake by wire non generato correttamente\n");
        exit(EXIT_FAILURE);
    } else if (attuatoriPid[2] == 0) {
        printf("Eseguo brake by wire con una execv\n");
        char *argv[] = {"./brakebywire", *mode, NULL};
        execv(argv[0], argv);
        perror("Errore durante l'esecuzione di execv\n");
        exit(EXIT_FAILURE);
    }

    sensoriPid[0] = fork(); // Genera front windshield camera

    if (sensoriPid[0] < 0) {
        perror("Fork error\n");
        printf("Processo front windshield camera non generato correttamente\n");
        exit(EXIT_FAILURE);
    } else if (sensoriPid[0] == 0) {
        printf("Eseguo front windshield camera con una execv\n");
        char *argv[] = {"./frontwindshieldcamera", *mode, NULL};
        execv(argv[0], argv);
        perror("Errore durante l'esecuzione di execv\n");
        exit(EXIT_FAILURE);
    }

    // Codice del processo padre
    // ...
}

void startParkAssist(char **mode)
{
    char *argv[3];
    printf("Eseguo park assist con una execv\n");
    argv[0] = "./parkassist";
    argv[1] = *mode;
    argv[2] = NULL;
    execv(argv[0], argv);
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
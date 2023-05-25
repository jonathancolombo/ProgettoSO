//
// Created by jonathan on 09/05/23.
//

/*
 * Questo componente riceve il comando di accelerazione dalla Central
    ECU, nel formato “INCREMENTO 5”, dove 5 indica l’aumento di velocità richiesto. Alla ricezione del
    messaggio dalla Central ECU, il componente stampa nel file di log throttle.log la data attuale, e “AUMENTO 5”.
    Quando l’attuatore Throttle Control viene creato, genera due processi interni: Il processo padre,
    si occupa della generazione della socket e di rimanervi in ascolto; il processo figlio, invece, si
    occupa di aprire e scrivere sul file di log “NO ACTION” se non riceve comandi di accelerazione,5
    oppure “AUMENTO 5” se riceve il comando di accelerazione. Il processo padre riceve
    continuamente messaggi dalla ECU client e ne scrive il contenuto nella pipe. Il processo
    figlio legge dalla pipe il valore della accelerazione da apportare all’auto: se è maggiore di zero la
    esegue, altrimenti stampa “NO ACTION”.
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>

int deltaSpeed;
FILE* fileLog;
int status;	//pipe status
int pipeArray[2]; //pipe array
pid_t pidChildWriter;

int openPipeOnRead(char *pipeName);
int readLine(int fileDescriptor, char *str);
void formattedTime(char *timeBuffer);
void writeMessage(FILE *fp, const char * format, ...);



void openFile(char filename[], char mode[], FILE **filePointer);
void sigTermHandler();
int readFromSocket (int fileDescriptor, char *string);
int extractString(char* data);

int main(int argc, char* argv[])
{
    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("PROCESSO THROTTLE BY CONTROL\n");

    printf("Tento di aprire il file di log throttle.log\n");
    fileLog = fopen("throttle.log", "w");
        
    if (fileLog == NULL) 
    {
        printf("Errore nell'apertura del file throttle.log\n");
        exit(EXIT_FAILURE);
    }

    printf("File throttle.log aperto correttamente\n");

    int ecuFileDescriptor = openPipeOnRead("../ipc/throttlePipe");
    char command[16];

    for (;;)
    {
        readLine(ecuFileDescriptor, command);
        if (strncmp(command, "INCREMENTO", 10) == 0)
        {
            int increment = atoi(command + 11);
            writeMessage(fileLog, "AUMENTO %d", increment);
            sleep(1);
        } 
        else
        {
            writeMessage(fileLog, "NO ACTION");
            sleep(1);
        }
    }

    fclose(fileLog);
    exit(EXIT_SUCCESS);
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

void formattedTime(char *timeBuffer) 
{
    time_t rawTime;
    struct tm *info;
    time(&rawTime);
    info = localtime(&rawTime);
    strftime(timeBuffer,256,"%x - %X", info);
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




int extractString(char* data) {							//estrae l'incremento di velocita' dall'input inviato dall'ECU
	char* acceleration;
	acceleration = strtok (data," ");
	acceleration = strtok (NULL, " ");					//split tramite " "
	return (int) strtol(acceleration, NULL, 10);		//converte la stringa in intero
}

void sigTermHandler()
{
    kill(pidChildWriter, SIGTERM);
    exit(EXIT_SUCCESS);
}

int readFromSocket (int fd, char *str) {
    int n;
    do {
        n = read (fd, str, 1);
    } while (n > 0 && *str++ != '\0');
    return (n > 0);
}


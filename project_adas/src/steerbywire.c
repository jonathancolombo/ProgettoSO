//
// Created by jonathan on 09/05/23.
//
/*
 * Componente steer-by-wire. Questo componente riceve dalla Central ECU il comando di girare a
DESTRA o SINISTRA, e conseguentemente attiva lo sterzo. L’azione di girare dura 4 secondi.
Ogni secondo, il componente stampa nel file di log steer.log: “NO ACTION”, “STO GIRANDO A DESTRA”,
“STO GIRANDO A SINISTRA”, sulla base dell’azione in corso.
 Alla creazione del processo steer by wire, quest'ultimo crea altri due processi interni uno
 dei quali si occupa dell'apertura e della scrittura del file di log, mentre il figlio si occupa di
 generare la socket e quindi di ricevere i messaggi da ECU client che vengono scritti nella pipe
 Il processo padre legge dalla pipe e avvia la procedura di sterzata, quindi scrive nel file di log.

 FATTO, DA TESTARE E MIGLIORARE
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>

FILE* fileLog;
int pipeArray[2];
pid_t processWriterToLog;

// new functions 
int openPipeOnRead(char *pipeName);
int readLine(int fileDescriptor, char *str);
void formattedTime(char *timeBuffer);
void writeMessage(FILE *fp, const char * format, ...);


int readFromSocket (int , char *);
void sigTermHandler();

int main(int argc, char* argv[])
{
    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("PROCESSO STEER BY WIRE\n");
    
    printf("Tento di aprire il file brake.log in scrittura\n");
    fileLog = fopen("steer.log", "w");

    if (fileLog ==  NULL)
    {
        printf("Errore nell'apertura del file brake.log\n");
        exit(EXIT_FAILURE);
    }

    printf("File di log aperto correttamente\n");
    
    printf("Faccio una read non bloccante su steer by wire\n");
    char command[16];
    int fileDescriptor = openPipeOnRead("../ipc/steerPipe");
    int readValue = 0;
    for (;;)
    {
        printf("Leggo una linea\n");
        if (readValue = readLine(fileDescriptor, command) == -1)
        {
            writeMessage(fileLog, "NO ACTION");
            sleep(1);
        }
        else if (strcmp(command, "DESTRA") == 0 || strcmp(command, "SINISTRA") == 0)
        {
            writeMessage(fileLog, "STO GIRANDO A %s", command);
            sleep(1);
        } 
    }

    fclose(fileLog);
    //wait(NULL);
    exit(EXIT_SUCCESS);
}


int openPipeOnRead(char *pipeName)
{
    int fileDescriptor;
    do
    {
        fileDescriptor = open(pipeName, O_RDONLY | O_NONBLOCK); // Opening named pipe for write
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



int readFromSocket (int fd, char *str) {
    int n;
    do {
        n = read (fd, str, 1);
    } while (n > 0 && *str++ != '\0');
    return (n > 0);
}

void sigTermHandler() {
    signal(SIGTERM,SIG_DFL);
    kill(processWriterToLog,SIGTERM);
    exit(EXIT_SUCCESS);
}


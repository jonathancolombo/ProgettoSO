
/*
 * Il componente riceve dalla Central ECU il comando di decelerazione o
un segnale di pericolo. Il comando di decelerazione è ricevuto nel formato “FRENO 5”, dove 5 indica la
riduzione di velocità richiesta. La velocità è decrementata, per ogni messaggio, di 5 KM/H. Alla ricezione del
messaggio dalla Central ECU, il componente stampa nel file di log brake.log la data attuale, e “FRENO 5”.
Se il componente riceve il segnale di ARRESTO dalla Central ECU, arresta l’auto (azione istantanea). Questo
equivale a scrivere “ARRESTO AUTO” nel file di log brake.log.


 Anche l’attuatore ​Brake By Wire​, non appena creato, genera due processi interni: il processo
padre ha il compito di generare la ​socket​, vi rimane in ascolto e scrive la differenza di velocità
che riceve dalla ECU ​client ​nella ​pipe​; il processo figlio, invece, si occupa di aprire e scrivere sul
file di log “NO ACTION” se non riceve messaggi di frenata, altrimenti scrive “DECREMENTO 5”. Il
processo figlio legge dalla ​pipe il valore della decelerazione da apportare all’auto: se è maggiore
di zero, la esegue, altrimenti stampa “NO ACTION”. Il processo padre, inoltre, si occupa di
gestire l’operazione di ​parcheggio​. Infatti, quando la ECU riceve il comando “PARCHEGGIO”,
invia, tramite ​socket, a ​Brake By Wire, ​il messaggio “PARCHEGGIO X” (X è la velocità corrente).
La stringa di messaggio viene ​splittata ​in “PARCHEGGIO” e “X”. Il processo padre, quando legge
“PARCHEGGIO” attua l’apposita procedura di arresto normale dell’auto. A questo punto, il
processo figlio decrementa la velocità normalmente fino a zero, e quando l’auto è ferma, il
processo padre comunica alla ECU, tramite ​signal​, che l’auto si è fermata e avvia la procedura di
terminazione del processo figlio e di se stesso. Infine, il processo padre gestisce l’operazione di
pericolo​. Quando la ECU rileva un pericolo invia una ​signal ​Brake By Wire​. Il processo padre
cattura la ​signal e scrive “ARRESTO AUTO” sul file di log. Dopodiché comunica, tramite ​signal​,
alla ECU di aver completato l’esecuzione, e questa inizierà la procedura di terminazione dei
sensori e attuatori.

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
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>
#define SIGPARK SIGUSR1
#define SIGWARNING SIGUSR1

// new functions
void handleStop();
int openPipeOnRead(char *pipeName);
int readLine(int fileDescriptor, char *str);
void formattedTime(char *timeBuffer);
void writeMessage(FILE *fp, const char * format, ...);

void openFile(char filename[], char mode[], FILE **filePointer);
void sigWarningHandler();
void sigTermHandler();
int readFromSocket (int fileDescriptor, char *string);
int extractString(char* data);

int pipeArray[2];
int status;

pid_t  pid;
pid_t ecuPid;

FILE *fileLog;
char *command;

int deltaSpeed;
int READ = 0;
int WRITE = 1;

int main(void)
{
    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("\n");

    printf("PROCESSO BRAKE BY WIRE\n");
    signal(SIGTSTP, handleStop);

    printf("Tento di aprire il file brake.log in scrittura\n");
    fileLog = fopen("brake.log", "w");

    if (fileLog ==  NULL)
    {
        printf("Errore nell'apertura del file brake.log\n");
        exit(EXIT_FAILURE);
    }

    printf("File di log aperto correttamente\n");
    
    printf("Faccio una read non bloccante dalla pipe\n");
    int ecuFileDescriptor = openPipeOnRead("brakePipe");
    char command[16];
    for(;;)
    {
        printf("Leggo una linea\n");
        readLine(ecuFileDescriptor, command);
        if (strncmp(command, "FRENO", 5) == 0)
        {
            int speed = atoi(command + 6);
            writeMessage(fileLog, "FRENO %d", speed);
        }
    }

    fclose(fileLog);
    exit(EXIT_SUCCESS);
}

void handleStop() 
{
    writeMessage(fileLog, "ARRESTO AUTO");
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

void openFile(char filename[], char mode[], FILE **filePointer) {
    *filePointer = fopen(filename, mode);
    if (*filePointer == NULL)
    {
        printf("Errore nell'apertura del file");
        exit(EXIT_FAILURE);
    }
}

void sigWarningHandler() {
    printf("Scrivo ARRESTO AUTO sul file di log brake.log\n");
    fprintf(fileLog, "ARRESTO AUTO\n");
    fflush(fileLog);
    //close(fileLog);
    kill(getpid(), SIGTERM);
}

void sigTermHandler() {
    kill(pid, SIGTERM);
    exit(EXIT_SUCCESS);
}

int readFromSocket (int fd, char *str) {
    int n;
	do {
		n = read (fd, str, 1);
	} while (n > 0 && *str++ != '\0');
return (n > 0);
}

int extractString(char* data) {							//estrae il decremento di velocita' dall'input inviato dall'ECU
    char* acceleration;
    command = strtok (data," ");
    acceleration = strtok (NULL, " ");					//split tramite " "
    return (int) strtol(acceleration, NULL, 10);		//converte la stringa in intero
}
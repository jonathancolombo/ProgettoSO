
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

void openFile(char filename[], char mode[], FILE **filePointer);
void sigWarningHandler();
void sigTermHandler();
int readFromSocket (int fileDescriptor, char *string);
int extractString(char* data);

int pipeArray[2];
pid_t  pid;
pid_t ecuPid;
FILE* fileLog;
char *command;


int main(void)
{
    int speed = 0;
    int READ = 0, WRITE = 1;
    ecuPid = getppid();
    int status = pipe(pipeArray); // pipe status
    if (status != 0) // controllo dello stato della pipe
    {
        printf("\nError with pipe");
        exit(EXIT_FAILURE);
    }

    openFile("brake.log", "w", &fileLog); // viene aperto il file di log brake.log su cui andare a scrivere
    fcntl(pipeArray[0], F_SETFL, O_NONBLOCK);	//rende la read non bloccante (da studiare a fondo il funzionamento di questa API)
    pid = fork();					//crea un processo figlio di scrittura
    if (pid == 0)
    {
        close(pipeArray[WRITE]); // chiude il processo pipe di scrittura
        //startLoop();
        for (;;)
        {
            int sentData = 0;
            if (read(pipeArray[READ], &sentData, sizeof (sentData) > 0))
            {
                speed = sentData;
            }
            if (speed > 0)
            {
                fprintf(fileLog, "DECREMENTO 5\n");
                fflush(fileLog);
                speed -= 5;
            }
            else
            {
                //fprintf(fileLog, "NO ACTION\n");
                fflush(fileLog);
            }
            sleep(1);
        }

        close(pipeArray[READ]); // chiude il processo pipe di lettura
        return EXIT_FAILURE;
    }
    else
    {
        signal(SIGUSR1, sigWarningHandler);
        signal(SIGTERM, sigTermHandler);
        close(pipeArray[READ]);
        //initSocket();					//il padre rimane in ascolto sulla socket

        int serverFileDescriptor = 0, clientFileDescriptor = 0, serverLength = 0, clientLength = 0;
        struct sockaddr_un serverAddress; /*Server address */
        struct sockaddr* ptrServerAddress; /*Ptr to server address*/

        struct sockaddr_un clientAddress; /*Client address */
        struct sockaddr* clientSockAddrPtr;/*Ptr to client address*/

        ptrServerAddress = (struct sockaddr*) &serverAddress;
        serverLength = sizeof (serverAddress);

        clientSockAddrPtr = (struct sockaddr*) &clientAddress;
        clientLength = sizeof (clientAddress);
        serverAddress.sun_family = AF_UNIX;

        char* socketName = "brakebywireSocket";

        serverFileDescriptor = socket (AF_UNIX, SOCK_STREAM, 0);
        strcpy (serverAddress.sun_path, socketName);
        unlink (socketName);

        bind (serverFileDescriptor, ptrServerAddress, serverLength);
        listen (serverFileDescriptor, 2);

        while (1)
        {/* Loop forever */ /* Accept a client connection */
            clientFileDescriptor = accept (serverFileDescriptor, clientSockAddrPtr, &clientLength);
            if (fork () == 0)
            { /* Create child */
                char data[100];
                while (readFromSocket(clientFileDescriptor, data)){
                    //startElaboration(data);
                    speed = extractString(strdup(data));
                    int waitingTime = speed/5;
                    write(pipeArray[1], &speed, sizeof(speed));
                    if (strcmp(command, "PARCHEGGIO") == 0) {
                        sleep(waitingTime);
                        kill(ecuPid, SIGUSR1); // SEGNALA CHE HA PARCHEGGIATO
                        sigTermHandler(); // MUORE
                    }
                }
                close (clientFileDescriptor); /* Close the socket */
                exit (/* EXIT_SUCCESS */ 0); /* Terminate */
            }
            else
                close (clientFileDescriptor); /* Close the client descriptor */
        }
    }

    exit(EXIT_SUCCESS);
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
    fprintf(fileLog, "ARRESTO AUTO\n");
    kill(getpid(), SIGTERM);
}

void sigTermHandler() {
    kill(pid, SIGTERM);
    exit(0);
}

int readFromSocket (int fileDescriptor, char *string) {
    int n;
    do {
        n = read (fileDescriptor, string, 1);
    } while (n > 0 && *string++ != '\0');
    return (n > 0);
}

int extractString(char* data) {							//estrae il decremento di velocita' dall'input inviato dall'ECU
    char* acceleration;
    command = strtok (data," ");
    acceleration = strtok (NULL, " ");					//split tramite " "
    return (int) strtol(acceleration, NULL, 10);		//converte la stringa in intero
}
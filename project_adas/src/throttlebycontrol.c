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
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <fcntl.h>

int deltaSpeed;
FILE* fileLog;
int status;	//pipe status
int pipeArray[2]; //pipe array
pid_t pid;

void openFile(char filename[], char mode[], FILE **filePointer);
void sigTermHandler();
int readFromSocket (int fd, char *str);

int main(void)
{
    int READ = 0;
    int WRITE = 1;

    status = pipe(pipeArray);
    if(status != 0)
    {
        printf("Error with pipe\n");
        exit(EXIT_FAILURE);
    }
    fcntl(pipeArray[READ], F_SETFL, O_NONBLOCK);	//rende la read non bloccante
    pid = fork();							//crea un processo figlio di scrittura su file di log
    if(pid == 0)
    {
        close(pipeArray[WRITE]);

        openFile("throttle.log", "w", &fileLog);

        for(;;)
        {
            int sentData;									//variabile d'appoggio per la lettura da pipe
            if (read(pipeArray[0], &sentData, 30) > 0) {
                deltaSpeed = sentData;
            }
            if (deltaSpeed > 0) {
                fprintf(fileLog, "AUMENTO 5\n");
                fflush(fileLog);
                deltaSpeed = deltaSpeed - 5;
            } else {
                fprintf(fileLog, "NO ACTION\n");
                fflush(fileLog);
            }
            sleep(1);
        }
        close(pipeArray[READ]);
        exit(EXIT_SUCCESS);
    }
    else // il padre rimane in ascolto sulla socket
    {
        signal(SIGTERM, sigTermHandler);
        close(pipeArray[READ]);
        // configurazione della socket per comunicazione client server
        //initSocket();						//il padre rimane in ascolto sulla socket
        int serverFileDescriptor, clientFileDescriptor, serverLength, clientLength;
        struct sockaddr_un serverAddress; /*Server address */
        struct sockaddr* ptrSocketServerAddress; /*pointer to server address*/

        struct sockaddr_un clientUnixAddress; /*Client address */
        struct sockaddr* ptrSocketClientAddress;/*Ptr to client address*/

        ptrSocketServerAddress = (struct sockaddr*) &serverAddress;
        serverLength = sizeof (serverAddress);
        
        ptrSocketClientAddress = (struct sockaddr*) &clientUnixAddress;
        clientLength = sizeof (clientUnixAddress);

        char* socketName = "throttlebycontrolSocket";
        serverFileDescriptor = socket (AF_UNIX, SOCK_STREAM, 0);
        serverAddress.sun_family = AF_UNIX;

        strcpy (serverAddress.sun_path, socketName);
        unlink (socketName);
        bind (serverFileDescriptor, ptrSocketServerAddress, serverLength);
        listen (serverFileDescriptor, 1);

        int speed = 0;

        for (;;)
        {/* Loop forever */ /* Accept a client connection */
            clientFileDescriptor = accept (serverFileDescriptor, ptrSocketClientAddress, &clientLength);
            if (fork() == 0)
            { /* Create child */
                char data[30];
                while (readFromSocket(clientFileDescriptor, data)) // finchè ci sono dati dai client
                {
                    //startElaboration(data);
                    char* acceleration;
                    acceleration = strtok (data," ");
                    acceleration = strtok (NULL, " ");
                    speed = (int) strtol(acceleration, NULL, 10);
                    write(pipeArray[WRITE], &speed, 30);
                }

                close (clientFileDescriptor); /* Close the socket */
                exit (/* EXIT_SUCCESS */ EXIT_SUCCESS); /* Terminate */
            }
            else
            {
                close (clientFileDescriptor); /* Close the client descriptor */
            }
        }
    }
}

void openFile(char filename[], char mode[], FILE **filePointer) {
    *filePointer = fopen(filename, mode);
    if (*filePointer == NULL) {
        printf("Errore nell'apertura del file");
        exit(1);
    }
}


void sigTermHandler()
{
    kill(pid, SIGTERM);
    exit(0);
}

int readFromSocket (int fd, char *str) {
    int n;
    do {
        n = read (fd, str, 1);
    } while (n > 0 && *str++ != '\0');
    return (n > 0);
}


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


int deltaSpeed;
FILE* fileLog;
int status;	//pipe status
int pipeArray[2]; //pipe array
pid_t pidChildWriter;

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

    int READ = 0;
    int WRITE = 1;

    status = pipe(pipeArray);

    if(status != 0)
    {
        printf("Error with pipe\n");
        exit(EXIT_FAILURE);
    }

    fcntl(pipeArray[READ], F_SETFL, O_NONBLOCK);	//rende la read non bloccante
    printf("Creo un processo figlio di scrittura sul file di log throttle.log\n");
    pidChildWriter = fork();							//crea un processo figlio di scrittura su file di log
    if(pidChildWriter == 0)
    {
        printf("Processo figlio di scrittura creato correttamente\n");
        close(pipeArray[WRITE]);


        printf("Tento di aprire il file di log throttle.log\n");
        fileLog = fopen("throttle.log", "w");
        
        if (fileLog == NULL) 
        {
            printf("Errore nell'apertura del file throttle.log\n");
            exit(EXIT_FAILURE);
        }

        for(;;)
        {
            int sentData;									//variabile d'appoggio per la lettura da pipe
            if (read(pipeArray[0], sentData, 30) > 0) 
            {
                deltaSpeed = sentData;
            }
            if (deltaSpeed > 0) 
            {
                printf("Stampo sul file di log di throttle by control, AUMENTO 5\n");
                fprintf(fileLog, "AUMENTO 5\n");
                fflush(fileLog);
                deltaSpeed = deltaSpeed - 5;
            } 
            else 
            {
                printf("Stampo sul file di log di throttle by control, NO ACTION\n");
                fprintf(fileLog, "NO ACTION\n");
                fflush(fileLog);
            }
            sleep(1);
        }

        close(pipeArray[READ]);
        close(fileLog);
        exit(EXIT_SUCCESS);
    }
    else // il padre rimane in ascolto sulla socket
    {
        printf("Processo figlio non creato\n");
        signal(SIGTERM, sigTermHandler);
        close(pipeArray[READ]);
        // configurazione della socket per comunicazione client server
        //initSocket();						//il padre rimane in ascolto sulla socket
        printf("Inizializzo la socket su throttle by control\n");
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

        printf("Socket inizializzata su throttle by control\n");
        int speed = 0;

        for (;;)
        {/* Loop forever */ /* Accept a client connection */
            clientFileDescriptor = accept (serverFileDescriptor, ptrSocketClientAddress, &clientLength);
            printf("Creo il processo figlio per leggere i dati dalla socket\n");
            if (fork() == 0)
            { /* Create child */
                char data[30];
                printf("Finchè ci sono dati provenienti dalla socket...");
                while (readFromSocket(clientFileDescriptor, data)) // finchè ci sono dati dai client
                {
                    deltaSpeed = extractString(strdup(data));
                    write(pipeArray[WRITE], &speed, 30);
                }
                close (clientFileDescriptor); /* Close the socket */
                exit (EXIT_SUCCESS); /* Terminate */
            }
            else
            {
                close (clientFileDescriptor); /* Close the client descriptor */
            }
        }
    }
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


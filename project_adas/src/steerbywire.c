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

FILE* fileLog;
int pipeArray[2];
pid_t processWriterToLog;

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
    int READ = 0;
    int WRITE = 1;
    int status = pipe(pipeArray);

    printf("Controllo lo stato della pipe su steer by wire\n");
    if (status != 0)
    {
        printf("Error pipe!\n");
        return (EXIT_FAILURE);
    }
    
    printf("Faccio una read non bloccante su steer by wire\n");
    fcntl(pipeArray[READ], F_SETFL, O_NONBLOCK); //lettura non bloccante, intanto testare la lettura classica
    
    printf("Creo il processo figlio che si preoccupa di scrivere su file di log le sterzate\n");
    processWriterToLog = fork(); // creo un processo figlio che scrive su file di log

    if (processWriterToLog == 0)
    {
        printf("Processo figlio steer by wire creato\n");
        close(pipeArray[READ]); // chiudo la pipe in lettura

        printf("Inizializzo la socket su steer by wire\n");

        int serverFileDescriptor = 0, clientFileDescriptor = 0, serverLength = 0, clientLength = 0;
            struct sockaddr_un serverAddress; /*Server address */
            struct sockaddr* ptrServerSocket; /*Ptr to server address*/

            struct sockaddr_un clientAddress; /*Client address */
            struct sockaddr* ptrClientSocket;/*Ptr to client address*/

            ptrServerSocket = (struct sockaddr*) &serverAddress;
            serverLength = sizeof (serverAddress);

            ptrClientSocket = (struct sockaddr*) &clientAddress;
            clientLength = sizeof (clientAddress);

            char* socketName = "steerbywireSocket";

            serverFileDescriptor = socket (AF_UNIX, SOCK_STREAM, 0);
            serverAddress.sun_family = AF_UNIX;

            strcpy (serverAddress.sun_path, socketName);
            unlink (socketName);
            bind (serverFileDescriptor, ptrServerSocket, serverLength);
            listen (serverFileDescriptor, 1);

            printf("Socket su steer by wire inizializzata\n");

            for(;;) // loop infinito che accetta possibili connessioni da parte di client
            {
                clientFileDescriptor = accept(serverFileDescriptor, ptrClientSocket, &clientLength);
                printf("Creo un processo figlio per leggere dalla socket su steer by wire\n");
                if (fork() == 0)
                {
                    char data[100];
                    while (readFromSocket(clientFileDescriptor, data) > 0)
                    {
                        write(pipeArray[WRITE], data, 30);
                    }
                    close(clientFileDescriptor);
                    exit(EXIT_SUCCESS);
                }
                else
                {
                    close(clientFileDescriptor); // chiuso il client descriptor
                }

            }
    } 
    else
    {
        signal(SIGTERM,sigTermHandler);
        close(pipeArray[WRITE]);
        char *direction;
        
        printf("Apro il file di log steer.log\n");

        fileLog = fopen("steer.log", "w");

        if (fileLog == NULL)
        {
            printf("Errore nell'apertura del file steer.log\n");
            exit(EXIT_FAILURE);
        }
    
        while(1) 
        {
            char sentData[30];
            if (read(pipeArray[0], sentData, 30) > 0) 
            {
                direction = sentData;
            }

            if (strcmp(direction,"DESTRA") == 0 || strcmp(direction, "SINISTRA") == 0) 
            {
                for (int i = 0; i<4; i++)
                {
                    printf("Scrivo sul file di log la sterzata\n");    
                    fprintf(fileLog,"STO GIRANDO A %s\n", direction);
                    fflush(fileLog);
                    sleep(1);   
                } 
            } 
            else 
            {
                fprintf(fileLog, "NO ACTION\n");
                fflush(fileLog);
            }

            sleep(1);
        }

        close(fileLog);
    }

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


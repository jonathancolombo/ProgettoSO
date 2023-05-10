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
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <signal.h>

int pipeArray[2];
pid_t processWriterToLog;

int readFromSocket (int , char *);
void sigTermHandler();
void openFile(char filename[], char mode[], FILE **filePointer);

int main(int argc, char* argv[])
{
    int READ = 0;
    int WRITE = 1;
    int status = pipe(pipeArray);
    if (status != 0)
    {
        printf("Error pipe!\n");
        return (EXIT_FAILURE);
    }
    //fcntl(pfd[READ], F_SETFL, O_NONBLOCK); lettura non bloccante, intanto testare la lettura classica
    char dataReceived[100];
    for(;;)
    {
        read(pipeArray[READ], dataReceived, 100);
        processWriterToLog = fork(); // processo figlio di scrittura su file log creato
        if (processWriterToLog == 0)
        {
            close(pipeArray[READ]);

            // inizializzazione socket per lettura dati
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

            for(;;) // loop infinito che accetta possibili connessioni da parte di client
            {
                clientFileDescriptor = accept(serverFileDescriptor, ptrClientSocket, &clientLength);
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

            exit(EXIT_FAILURE);
        } // da fare dopo
        else
        {
            signal(SIGTERM,sigTermHandler);
            close(pipeArray[WRITE]);
            char *direction;
            FILE* fileLog;
            //startLoop(); si inizia a scrivere sul file di log
            openFile("steer.log", "w", &fileLog);
            while(1) {
                char sentData[30];
                if (read(pipeArray[0], sentData, 30) > 0) {
                    direction = sentData;
                }

                if (strcmp(direction,"DESTRA") == 0 ||
                    strcmp(direction, "SINISTRA") == 0) {
                    fprintf(fileLog,"STO GIRANDO A %s\n", direction);
                    fflush(fileLog);
                    sleep(1);
                } else {
                    fprintf(fileLog, "NO ACTION\n");
                    fflush(fileLog);
                }
                sleep(1);
            }
        }
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
    exit(0);
}


void openFile(char filename[], char mode[], FILE **filePointer) {
    *filePointer = fopen(filename, mode);
    if (*filePointer == NULL) {
        printf("Errore nell'apertura del file");
        exit(1);
    }
}

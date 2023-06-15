#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "commonFunctions.h"

int ecuPipeFileDescriptor;

void cleanupAndExit() 
{
    close(ecuPipeFileDescriptor);
    unlink("./hmiInputToEcuPipe");
    exit(EXIT_SUCCESS);
}

void handleSignal(int signal) 
{
    cleanupAndExit();
}

void sendCommandToEcu(const char *command) 
{
    write(ecuPipeFileDescriptor, command, strlen(command) + 1);
}

int main(int argc, char *argv[]) 
{
    char inputCommand[32];
    int processId = getpid();

    signal(SIGINT, handleSignal);

    printf("Sistema di input HMI inizializzato\n\n");

    // Creazione del file descriptor per la pipe verso l'ECU
    ecuPipeFileDescriptor = createPipe("./hmiInputToEcuPipe");

    // Invio dell'ID del processo all'ECU tramite la pipe
    write(ecuPipeFileDescriptor, &processId, sizeof(int));

    while (1) 
    {
        printf("Inserisci un comando (INIZIO, PARCHEGGIO, ARRESTO): ");

        // Lettura del comando inserito dall'utente
        scanf("%s", inputCommand);

        if (strcmp(inputCommand, "INIZIO") == 0) 
        {
            // Invio del comando "INIZIO" all'ECU
            sendCommandToEcu(inputCommand);
        } 
        else if (strcmp(inputCommand, "PARCHEGGIO") == 0) 
        {
            // Invio del comando "PARCHEGGIO" all'ECU
            sendCommandToEcu(inputCommand);
        } 
        else if (strcmp(inputCommand, "ARRESTO") == 0) 
        {
            // Invio del comando "ARRESTO" all'ECU
            sendCommandToEcu(inputCommand);
        } 
        else 
        {
            printf("Comando non valido. Riprova.\n");
        }
    }
}

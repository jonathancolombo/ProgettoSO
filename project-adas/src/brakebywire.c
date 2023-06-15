#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include "commonFunctions.h"

FILE *brakeLog;

void start();
void handleStop();
void handleFailure();
void sendSignals();
void getCommandFromPipeAndWrite(int);

int main(int argc, char *argv[]) 
{
    start();
    exit(EXIT_SUCCESS);
}

void start()
{
    // invio dei segnali di stop e fallimento 
    sendSignals();
    createLog("./brake", &brakeLog);
     
    int ecuFileDescriptor = openPipeOnRead("./brakePipe");
    getCommandFromPipeAndWrite(ecuFileDescriptor);
}

void handleStop() 
{
    writeMessage(brakeLog, "ARRESTO AUTO");
}

void handleFailure() 
{
    fclose(brakeLog);
    exit(EXIT_FAILURE);
}

void sendSignals()
{
    // invio dei segnali di stop e fallimento 
    signal(SIGTSTP, handleStop);
    signal(SIGUSR1, handleFailure);
}

void getCommandFromPipeAndWrite(int ecuFileDescriptor)
{
    char command[16];
    int decrement; 
    for (;;) 
    {
        readline(ecuFileDescriptor, command);
        if(strncmp(command, "FRENO", 5) == 0) 
        {
            decrement = atoi(command + 6);
            writeMessage(brakeLog, "FRENO %d", decrement);
        }
    }
}
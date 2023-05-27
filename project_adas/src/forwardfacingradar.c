#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>


#include "functions.h"

FILE *sensorLog;

void handleFailure()
{
    fclose(sensorLog);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    printf("PROCESSO FORWARD FACING RADAR\n");

    signal(SIGUSR1, handleFailure);

    sensorLog = fopen("radar.log", "w");
    if (sensorLog == NULL)
    {
        printf("Errore nell'apertura del file radar.log\n");
        exit(EXIT_FAILURE);
    }

    printf("File radar.log aperto correttamente\n");
    writeMessage(sensorLog, "SENSOR LAUNCHED");

    fclose(sensorLog);
    wait(NULL);
    exit(EXIT_SUCCESS);
}
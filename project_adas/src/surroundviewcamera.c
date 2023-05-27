#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "functions.h"

// Variables
FILE *fileCamerasLog;

int main(int argc, char *argv[])
{
    printf("PROCESSO SURROUND VIEW CAMERA\n");

    fileCamerasLog = fopen("cameras.log", "w");
    if (fileCamerasLog == NULL)
    {
        printf("Errore nell'apertura del file cameras.log\n");
        return EXIT_FAILURE;
    }

    writeMessage(fileCamerasLog, "SENSOR LAUNCHED");
    fclose(fileCamerasLog);

    wait(NULL);
    exit(EXIT_SUCCESS);
}

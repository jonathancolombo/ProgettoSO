#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#define DEFAULT_PROTOCOL 0

#include "commonFunctions.h"
#include "socketFunctions.h"

int main(int argc, char *argv[]) {
    FILE *log;
    createLog("./cameras", &log);
    writeMessage(log, "SENSOR LAUNCHED");
    fclose(log);
    wait(NULL);
    exit(EXIT_SUCCESS);
}
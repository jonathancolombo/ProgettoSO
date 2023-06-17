#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */

#define DEFAULT_PROTOCOL 0

// Autenticazione del socket client
void socketAuth(int *clientFd, struct sockaddr_un *serverAddress, int *serverLen, char *serverName) {
    // Impostazione della lunghezza dell'indirizzo del server
    *serverLen = sizeof(*serverAddress);
    
    // Creazione del socket client
    *clientFd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
    
    // Configurazione dell'indirizzo del server
    struct sockaddr_un tmp = {.sun_family = AF_UNIX};
    strcpy(tmp.sun_path, serverName);
    *serverAddress = tmp;
}

// Connessione al server
void connectServer(int clientFd, struct sockaddr *serverAddressPtr, int serverLen) {
    // Tentativi di connessione al server con ritardo
    while(connect(clientFd, serverAddressPtr, serverLen) == -1) { 
        sleep(2);
    }
}

// Ricezione di una stringa dal socket
void receiveString(int fd, char *str) {
    do {
        // Ricezione di un singolo carattere alla volta
        while(recv(fd, str, 1, 0) < 0)
            sleep(1);
    } while(*str++ != '\0');
}

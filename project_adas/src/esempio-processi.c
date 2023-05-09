

/***
 * 
 * In questo esempio, il processo sorgente scrive una stringa di
 *  output nella pipe utilizzando la funzione write(), mentre il 
 * processo destinazione legge l'input dalla pipe utilizzando la
 *  funzione read(). La pipe viene creata utilizzando la funzione 
 * pipe() e il nuovo processo viene creato utilizzando la funzione fork().
 * 
 * 
*/

/*
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

__pid_t pid;

int main() {
    int fd[2];

    // crea la pipe
    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    // crea un nuovo processo
    pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // processo destinazione (legge l'input)
        close(fd[1]); // chiude il lato di scrittura della pipe

        char buf[100];
        int nbytes = read(fd[0], buf, sizeof(buf));
        printf("Input ricevuto: %.*s\n", nbytes, buf);
    } else {
        // processo sorgente (scrive l'output)
        close(fd[0]); // chiude il lato di lettura della pipe

        char *output = "Questo è un output di esempio.";
        write(fd[1], output, strlen(output));
    }

    return 0;
}
*/
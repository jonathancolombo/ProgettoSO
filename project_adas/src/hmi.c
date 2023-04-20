#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

__pid_t ecuProcessID; // pid del processo ECU

int main(int argc, char *argv[])
{
    /*
    implementare controllo se NORMALE o ARTIFICIALE
    */

    ecuProcessID = fork();
    if (ecuProcessID == -1)
    {
        perror("fork error");
        exit (EXIT_FAILURE);
    } 
    else if (ecuProcessID == 0)
    {
        printf("Sono il processo figlio\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("Sono il processo padre, il PID del figlio è %d\n", ecuProcessID);
        exit(EXIT_SUCCESS);
    }

    return EXIT_SUCCESS;
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

__pid_t ecuProcessPID; // pid del processo ECU
__pid_t tailProcessPID; // pid del processo coda

int main(int argc, char *argv[])
{
    if ((argc < 2) || (strcmp(argv[1] , "ARTIFICIALE") != 0 && strcmp(argv[1] , "NORMALE")))
    {
        printf("Inserisci se vuoi una tipologia di avvio NORMALE o ARTIFICIALE");
        exit(EXIT_SUCCESS);
    }


    ecuProcessPID = fork();
    if (ecuProcessPID == -1)
    {
        perror("fork error");
        exit (EXIT_FAILURE);
    } 
    else if (ecuProcessPID == 0)
    {
        printf("Sono il processo figlio\n");
        setpgid(0, 0);
        execv(argv[0], argv);
        exit(EXIT_SUCCESS);
    }
    else
    {
        tailProcessPID = fork();
        if (tailProcessPID < 0)
        {
            perror("fork error");
            exit (EXIT_FAILURE);
        }
        if (tailProcessPID == 0)
        {
            system("rm -f ECU.log; touch ECU.log; gnome-terminal -- sh -c \"echo OUTPUT:; tail -f ECU.log; bash\"");
        }

        //printf("Sono il processo padre, il PID del figlio è %d\n", ecuProcessID);
        //exit(EXIT_SUCCESS);
    }

    return EXIT_SUCCESS;
}
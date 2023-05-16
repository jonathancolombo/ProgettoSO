
// LIBRERIE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#define SIGSTART SIGUSR1
#define SIGPARK SIGUSR1
#define SIGWARNING SIGUSR2

FILE *fileUtility;
FILE *fileEcuLog;

pid_t ecuProcessPID; // pid del processo ECU
pid_t tailProcessPID; // pid del processo coda
int started = 0;
char **g_argv;

void sigParkHandler();
void sigWarningHandler();
void openFile(char filename[], char mode[], FILE **filePointer);

int main(int argc, char *argv[])
{
    g_argv = argv;
    /*
    * Controlla la tipologia di AVVIO impostata e restituisce un errore
     */

    if ((argc < 2) || (strcmp(argv[1] , "ARTIFICIALE") != 0 && strcmp(argv[1] , "NORMALE") != 0))
    {
        printf("Inserisci il comando di avvio NORMALE o ARTIFICIALE\n");
        exit(EXIT_FAILURE);
    }

    // fork del processo ECU
    ecuProcessPID = fork();
    // controllo se il processo ECU è stato creato correttamente
    if (ecuProcessPID < 0)
    {
        printf("Errore della fork sul processo figlio\n");
        perror("fork error\n");
        exit (EXIT_FAILURE);
    } 
    else if (ecuProcessPID == 0)
    {
        // imposto il pgid ed eseguo una execv
        printf("Sono il processo figlio\n");
        printf("Cambio gruppo impostando il setpgid a 0,0\n");
        //per cambiare gruppo
        setpgid(0, 0);
        argv[0] = "./ecu";
        printf("Eseguo la exec dal processo figlio\n");
        execv(argv[0], argv);
        //exit(EXIT_SUCCESS);
    }
    else
    {
        tailProcessPID = fork();
        if (tailProcessPID < 0)
        {
            printf("Errore della fork sul processo tail\n");
            perror("fork error\n");
            exit (EXIT_FAILURE);
        }
        if (tailProcessPID == 0)
        {
            printf("Sono il processo figlio di tail\n");
            system("rm -f ECU.log; touch ECU.log; gnome-terminal -- sh -c \"echo OUTPUT:; tail -f ECU.log; bash\"");
        }
        else
        { 
            printf("Effettuo 3 segnali\n");
            // 3 signal da fare
            signal(SIGUSR1, sigParkHandler);
            signal(SIGWARNING, sigWarningHandler);
            signal(SIGINT, sigParkHandler);

            printf("Cerco di aprire utility.data\n");

            fileUtility = fopen("utility.data", "w");

            if (fileUtility == NULL)
            {
                printf("Errore sull'apertura del file utility.data\n");
                perror("open file error!\n");
                exit (EXIT_FAILURE);
            }

            printf("Faccio una fprintf sul file utility.data\n");
            fprintf(fileUtility, "%d\n", 0);
            printf("Chiudo il file utility.data\n");
            fclose(fileUtility); // chiusura file Utility

            printf("Cerco di aprire ECU.log\n"); 
            
            fileEcuLog = fopen("ECU.log", "w");

            if (fileEcuLog == NULL)
            {
                printf("Errore sull'apertura del file ECU.log\n");                
                perror("open file error!\n");
                exit (EXIT_FAILURE);
            }

            printf("Faccio una fprintf sul file di Log della ECU\n");
            fprintf(fileEcuLog, "%d\n", 0);
            printf("Chiudo il file di Log della ECU\n");
            fclose(fileEcuLog); // chiusura file ECU.log
                        

            printf("Ciao! Benvenuto nel simulatore di sistemi di guida autonoma. \nDigita INIZIO per avviare il veicolo,\no digita PARCHEGGIO per avviare la procedura di parcheggio e concludere il percorso.\n\n");

            // controllo comando d'input iniziale
            char input[30];
            started = 0;
            while(1)
            {
                if(fgets(input, 30, stdin) != NULL)
                {
                    if((started) == 0)
                    {
                        if(strcmp(input, "INIZIO\n") == 0)
                        {
                            printf("Veicolo avviato\n");
                            printf("Lancio il segnale di start sul processo ECU\n");
                            kill(ecuProcessPID, SIGSTART);
                            started = 1;
                        }
                        else if (strcmp(input, "PARCHEGGIO\n") == 0)
                        {
                            printf("Prima di poter parcheggiare devi avviare il veicolo.\nDigita INIZIO per avviare il veicolo.\n\n");
                        }
                        else
                        {
                            printf("Comando non ammesso.\n\n");
                        }
                    }
                    else
                    {
                        if(strcmp(input, "PARCHEGGIO\n") == 0)
                        {
                            printf("Sto fermando il veicolo...\n");
                            kill(ecuProcessPID, SIGPARK);
                            printf("Processo ECU ucciso\n");
                            started = 0;
                        }
                        else
                        {
                            printf("Comando non ammesso. \nDigita PARCHEGGIO per parcheggiare il veicolo\n\n");
                        }
                    }
                }
            }
        }

        printf("Sono il processo padre, il PID del figlio è %d\n", ecuProcessPID);
        
    }
    exit(EXIT_SUCCESS);
}


void sigParkHandler() {
    kill(-ecuProcessPID, SIGTERM);
    kill(tailProcessPID, SIGTERM);
    kill(0, SIGTERM);

}

void recreateEcu() {
	ecuProcessPID = fork();
    if(ecuProcessPID<0) {
        printf("Errore sulla fork di ricreazione della ECU\n");
        perror("fork error\n");
        exit(EXIT_FAILURE);
    }
    if(ecuProcessPID == 0) {
    	setpgid(0, 0);
        execv("./ecu", g_argv);
    }
}

void sigWarningHandler() {
	signal(SIGWARNING, sigWarningHandler);
	kill(-ecuProcessPID, SIGTERM);
	recreateEcu();
	printf("La macchina è stata arrestata per evitare un pericolo. \nPremi INIZIO per ripartire\n");
	started = 0;
}


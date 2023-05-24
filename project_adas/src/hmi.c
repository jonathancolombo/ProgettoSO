
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
#include <fcntl.h>

#define SIGSTART SIGUSR1
#define SIGPARK SIGUSR1
#define SIGWARNING SIGUSR2

FILE *fileUtility;
FILE *fileEcuLog;

pid_t ecuProcessPID;  // pid del processo ECU
pid_t tailProcessPID; // pid del processo coda
int started = 0;
char **g_argv;

/*
void sigParkHandler();
void sigWarningHandler();
void openFile(char filename[], char mode[], FILE **filePointer);
*/

int openPipeOnRead(char *pipeName);
int readLine (int fileDescriptor, char *str);
int readFromPipe(int fileDescriptor);

int main(int argc, char *argv[])
{
    /*
     * Controlla la tipologia di AVVIO impostata e restituisce un errore
     */
    if ((argc < 2) || (strcmp(argv[1], "ARTIFICIALE") != 0 && strcmp(argv[1], "NORMALE") != 0))
    {
        printf("Inserisci il comando di avvio NORMALE o ARTIFICIALE\n");
        exit(EXIT_FAILURE);
    }

    printf("Eseguo la system per la gnome-terminal");
    sleep(2);
    system("gnome-terminal -- ./hmiInput");
    // fork del processo ECU
    ecuProcessPID = fork();
    // controllo se il processo ECU è stato creato correttamente
    if (ecuProcessPID < 0)
    {
        printf("Errore della fork sul processo figlio\n");
        perror("fork error\n");
        exit(EXIT_FAILURE);
    }
    else if (ecuProcessPID == 0)
    {
        // imposto il pgid ed eseguo una execv
        printf("Sono il processo figlio\n");
        sleep(1);
        printf("Cambio gruppo impostando il setpgid a 0,0\n");
        // per cambiare gruppo
        setpgid(0, 0);
        argv[0] = "./ecu";
        sleep(1);
        printf("Eseguo la exec dal processo figlio\n");
        printf("argv[0]: %s, argv: %s", argv[0], *argv);
        sleep(1);
        execv(argv[0], argv);
    }

    printf("HMI Output system initialized\n\n");
    int n;
    int fileDescriptor = openPipeOnRead("../../ipc/ecuToHmiPipe");
    printf("Named pipe found.\n\n");
    printf("Inizio a leggere dalla pipe\n");
    for (;;)
    {
        while (readFromPipe(fileDescriptor) < 0);
    }

    /*
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
            // se nel caso fosse riaperto, usare modalità a = append

            sleep(10);
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

    }*/
    exit(EXIT_SUCCESS);
}

int openPipeOnRead(char *pipeName)
{
    int fileDescriptor;
    do
    {
        fileDescriptor = open(pipeName, O_RDONLY); // Opening named pipe for write
        if (fileDescriptor == -1)
        {
            printf("Pipename:%s non trovata. Riprova ancora...\n", pipeName);
            sleep(1);
        }
    } while (fileDescriptor == -1);
    return fileDescriptor;
}

int readLine (int fileDescriptor, char *str)
{
	int n;
	do {
		n = read (fileDescriptor, str, 1);
	} while (n > 0 && *str++ != '\0');
    return (n > 0);
}

int readFromPipe(int fileDescriptor) 
{
    char str[256];
    if(readLine(fileDescriptor, str) < 0) {
        return -1;
    }
    printf("%s\n", str);
    return 0;
}


/*
void sigParkHandler()
{
    kill(ecuProcessPID, SIGTERM);
    kill(tailProcessPID, SIGTERM);
    kill(0, SIGTERM);
}
*/

/*
void sigWarningHandler()
{
    signal(SIGWARNING, sigWarningHandler);
    kill(ecuProcessPID, SIGTERM);
    // recreateEcu();
    printf("La macchina è stata arrestata per evitare un pericolo. \nPremi INIZIO per ripartire\n");
    started = 0;
}
*/



/*
void recreateEcu()
{
    ecuProcessPID = fork();
    if (ecuProcessPID < 0)
    {
        printf("Errore sulla fork di ricreazione della ECU\n");
        perror("fork error\n");
        exit(EXIT_FAILURE);
    }
    if (ecuProcessPID == 0)
    {
        setpgid(0, 0);
        execv("./ecu", g_argv);
    }
}
*/

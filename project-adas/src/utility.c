#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Crea un file di log con il nome specificato
// Restituisce 0 in caso di successo, -1 in caso di fallimento
int createLog(char *name, FILE **log) {
    // Genera il nome del file di log
    char *fileName = (char*) malloc((strlen(name)+5)*sizeof(char));
    sprintf(fileName, "%s.log", name);
    
    // Rimuove il file di log se esiste
    remove(fileName);
    
    // Apre il file di log in modalità scrittura
    *log = fopen(fileName, "w");
    free(fileName);
    
    if (*log == NULL) {
        return -1; // Indica il fallimento nell'apertura del file
    }
    
    return 0; // Indica il successo
}

// Ottiene la data e l'ora corrente formattate come stringa
void formattedTime(char *timeBuffer) {
    time_t rawTime;
    struct tm *info;
    time(&rawTime);
    info = localtime(&rawTime);
    strftime(timeBuffer, 256, "%x - %X", info);
}

// Scrive un messaggio formattato nel file di log specificato
void writeMessage(FILE *fp, const char *format, ...) {
    char buffer[256];
    char date[256];
    formattedTime(date);
    
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);
    
    fprintf(fp, "[%s] - [%s]\n", date, buffer);
    fflush(fp);
}

// Crea una pipe con il nome specificato
int createPipe(char *pipeName) {
    int fd;
    
    // Rimuove la pipe se esiste
    unlink(pipeName);
    
    // Crea la pipe
    if(mknod(pipeName, __S_IFIFO, 0) < 0 ) {    
        exit(EXIT_FAILURE);
    }//Creating named pipe
    
    // Imposta i permessi sulla pipe
    chmod(pipeName, 0660);
    
    // Apre la pipe in modalità scrittura
    do {
        fd = open(pipeName, O_WRONLY);    //Apro la pipe in scrittura
        if(fd == -1){
            printf("%s non trovata. Riprovo ancora...\n", pipeName);
            sleep(1);
        }
    } while(fd == -1);
    
    return fd;
}

// Apre una pipe in modalità lettura
int openPipeOnRead(char *pipeName) {
    int fd;
    
    // Apre la pipe in modalità lettura
    do {
        fd = open(pipeName, O_RDONLY);    //Apro la pipe in lettura
        if(fd == -1){
            printf("%s non trovata. Riprovo ancora...\n", pipeName);
            sleep(1);
        }
    } while(fd == -1);
    
    return fd;
}

// Legge una riga dalla pipe specificata e la memorizza nella stringa str
// Restituisce 0 in caso di successo, -1 in caso di errore
int readline(int fd, char *str) {
    int n;
    
    do {
        if(read(fd, str, 1) < 0) {
            return -1;
        }
    } while(*str++ != '\0');
    
    return 0;
}

// Scrive un messaggio formattato nella pipe specificata
void writeMessageToPipe(int pipeFd, const char *format, ...) {
    char buffer[256];
    
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);
    
    write(pipeFd, buffer, strlen(buffer)+1);
}

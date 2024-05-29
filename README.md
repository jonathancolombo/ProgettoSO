## README

# Progetto A.A. 2022/2023 – ADAS made trivial

### Descrizione del Progetto

Il progetto consiste nella costruzione di un'architettura stilizzata e rivisitata per sistemi Advanced Driver Assistance Systems (ADAS), rappresentando le possibili interazioni e comportamenti tra componenti in scenari specifici di un sistema di guida autonoma.

### Struttura del Sistema

Il sistema è composto dai seguenti componenti, ciascuno rappresentato da almeno un processo (non sono ammessi thread):

- **Interfaccia**
  - Human-Machine Interface (HMI)

- **Attuatori**
  - Steer-by-wire
  - Throttle control
  - Brake-by-wire

- **Sensori**
  - Front windshield camera
  - Forward facing radar (facoltativo)
  - Park assist
  - Surround view cameras (facoltativo)

- **Controllo**
  - Central ECU

### Descrizione dei Componenti

#### Human-Machine Interface (HMI)
- Interagisce con la Central ECU per mostrare gli output a video e inviare input ricevuti dall'utente.
- Gestione di input/output tramite due terminali: uno per l'output e uno per l'input.
- Comandi possibili: `INIZIO`, `PARCHEGGIO`, `ARRESTO`.

#### Steer-by-wire
- Riceve dalla Central ECU comandi di girare a `DESTRA` o `SINISTRA`.
- Azione di girare dura 4 secondi con logging nel file `steer.log`.

#### Throttle control
- Riceve comandi di accelerazione dalla Central ECU nel formato `INCREMENTO X`.
- Logging delle azioni nel file `throttle.log`.
- **Facoltativo**: 10^-5 probabilità di fallimento dell'acceleratore.

#### Brake-by-wire
- Riceve comandi di decelerazione o arresto dalla Central ECU.
- Logging delle azioni nel file `brake.log`.

#### Front windshield camera
- Legge dati da una sorgente e li invia alla Central ECU ogni secondo.
- Logging dei dati nel file `camera.log`.

#### Forward facing radar (facoltativo)
- Legge dati da `/dev/urandom` e li invia alla Central ECU.
- Logging dei dati nel file `radar.log`.

#### Park assist
- Attivato dalla Central ECU, legge dati da `/dev/urandom` per 30 secondi.
- Logging dei dati nel file `assist.log`.
- **Facoltativo**: Interazione con `surround view cameras`.

#### Surround view cameras (facoltativo)
- Attivo solo quando `park assist` è attivo, legge dati da `/dev/urandom` e li invia a `park assist`.
- Logging dei dati nel file `cameras.log`.

#### Central ECU
- Gestisce i comandi inviati a tutti i componenti.
- Logging dei comandi nel file `ECU.log`.
- Interpreta i dati ricevuti dai sensori per gestire la velocità e le sterzate del veicolo.

### Modalità di Avvio

Il programma può essere eseguito in due modalità, selezionate tramite un parametro di input:

- **NORMALE**: i componenti forward facing radar, park assist e surround view cameras leggono i dati da `/dev/urandom`.
- **ARTIFICIALE**: i componenti leggono i dati dal file `urandomARTIFICIALE.binary`.

### Compilazione ed Esecuzione

1. **Compilazione**
   - Posizionarsi nella directory `src/`.
   - Utilizzare il comando `make` per compilare tutti i componenti.

2. **Esecuzione**
   - Eseguire il programma passando come argomento `NORMALE` o `ARTIFICIALE`.
   - Esempio: `./main NORMALE`

### Struttura dei File

- `src/`: Contiene i file sorgente `.c`, i file header e il `Makefile`.
- `logs/`: Directory dove verranno salvati i file di log.
- `Makefile`: Script per la compilazione del progetto.

### Consegna

- Il progetto deve essere consegnato in un archivio `.zip` o `.tar.gz` contenente:
  - Codice sorgente
  - Relazione in formato PDF (massimo 8 pagine)

### Informazioni di Contatto

- Nome, Cognome, Numero di matricola, Indirizzo e-mail
- Data di consegna

### Relazione del Progetto

La relazione deve includere:

- Informazioni sugli autori e data di consegna
- Istruzioni dettagliate per la compilazione ed esecuzione
- Descrizione del sistema obiettivo
- Indicazione degli elementi facoltativi realizzati
- Progettazione ed implementazione
- Esecuzione: esempi e commenti sul funzionamento del programma

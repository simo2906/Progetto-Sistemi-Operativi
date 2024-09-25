#include <stdio.h>
#include <string.h>
#include "file_system.h"

#define FILE_SIZE (5 * block_size) // Definire una dimensione del file di 5 blocchi

int main() {
    // Inizializzazione del file system
    initFileSystem();

    // Creazione di un nuovo file
    printf("Creazione del file 'large_file.txt'...\n");
    if (createFile("large_file.txt") != 0) {
        printf("Errore nella creazione del file.\n");
        return -1;
    }

    // Apertura del file
    printf("Apertura del file 'large_file.txt'...\n");
    FileHandle* file = openFile("large_file.txt");
    if (file == NULL) {
        printf("Errore nell'aprire il file.\n");
        return -1;
    }

    // Creazione di dati di grandi dimensioni
    char data[FILE_SIZE]; // Senza +1, non vogliamo un terminatore di stringa nei dati
    for (int i = 0; i < FILE_SIZE; i++) {
        data[i] = 'A'; // Riempie il file con il carattere 'A'
    }

    // Scrittura nel file
    printf("Scrittura di %d byte nel file.\n", FILE_SIZE);
    if (write(file, data, FILE_SIZE) != 0) {
        printf("Errore nella scrittura nel file.\n");
        return -1;
    }

    // Reimposta la posizione all'inizio del file
    seek(file, 0);

    // Lettura dal file
    char buffer[FILE_SIZE + 1];  // Buffer per leggere i dati (incluso terminatore)
    memset(buffer, 0, sizeof(buffer));  // Inizializza il buffer a 0
    if (read(file, buffer, FILE_SIZE) != 0) {
        printf("Errore nella lettura dal file.\n");
        return -1;
    }
    buffer[FILE_SIZE] = '\0';  // Aggiungi un terminatore di stringa al buffer

    // Stampa i dati letti
    printf("Dati letti dal file: %s\n", buffer);

    // Verifica che i dati letti siano corretti
    if (strncmp(buffer, data, FILE_SIZE) == 0) {
        printf("Dati letti correttamente.\n");
    } else {
        printf("Errore nei dati letti.\n");
    }

    return 0;
}

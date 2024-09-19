#include <stdio.h>
#include "file_system.h"

int main() {
    // Inizializza il file system
    initFileSystem();
    
    // Nome del file da creare
    const char* filename1 = "testfile.txt";
    const char* filename2 = "testfile2.txt";
    const char* filename3 = "testfile3.txt";
    
    // Crea il file
    int result1 = createFile(filename1);
    
    // Verifica il risultato della creazione
    if (result1 != -1) {
        printf("File '%s' creato con successo.\n", filename1);
    } else {
        printf("Errore nella creazione del file '%s'.\n", filename1);
    }

    // Crea il file
    int result2 = createFile(filename2);
    
    // Verifica il risultato della creazione
    if (result2 != -1) {
        printf("File '%s' creato con successo.\n", filename2);
    } else {
        printf("Errore nella creazione del file '%s'.\n", filename2);
    }

    // Crea il file
    int result3 = createFile(filename3);
    
    // Verifica il risultato della creazione
    if (result3 != -1) {
        printf("File '%s' creato con successo.\n", filename3);
    } else {
        printf("Errore nella creazione del file '%s'.\n", filename3);
    }
    
    // Stampa la lista dei file nella directory corrente
    listDir();
    
    // Stampa lo stato della FAT per ulteriori informazioni
    printFAT();
    
    return 0;
}


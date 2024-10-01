#include <stdio.h>
#include <string.h>
#include "file_system.h"

int main() {
    // Inizializzazione del file system
    initFileSystem();

    // Creazione di una directory
    if (createDir("documents") == 0) {
        printf("Directory 'documents' creata con successo.\n");
    } else {
        printf("Errore nella creazione della directory 'documents'.\n");
    }

    // Creazione di una directory
    if (createDir("documents") == 0) {
        printf("Directory 'documents' creata con successo.\n");
    } else {
        printf("Errore nella creazione della directory 'documents'.\n");
    }

    // Elencare il contenuto della directory corrente
    printf("Contenuto della directory:\n");
    listDir(); // Chiamata alla funzione per elencare directory e file

    // Creazione di un file
    const char* filename = "example.txt";
    if (createFile(filename) == 0) {
        printf("File '%s' creato con successo.\n", filename);
    } else {
        printf("Errore nella creazione del file '%s'.\n", filename);
    }

    // Apertura del file
    FileHandle* handle = openFile(filename);
    if (handle != NULL) {
        printf("File '%s' aperto con successo.\n", filename);
    } else {
        printf("Errore nell'apertura del file '%s'.\n", filename);
    }

    // Scrittura nel file
    const char* text_to_write = "Ciao, mondo!";
    if (my_write(handle, text_to_write, strlen(text_to_write)) == 0) {
        printf("Dati scritti nel file '%s'.\n", filename);
    } else {
        printf("Errore nella scrittura nel file '%s'.\n", filename);
    }

    // Lettura dal file
    char buffer[256]; // Buffer per la lettura
    handle->position = 0; // Resetta la posizione a 0 per la lettura
    if (my_read(handle, buffer, sizeof(buffer)) == 0) {
        printf("Dati letti dal file '%s': %s\n", filename, buffer);
    } else {
        printf("Errore nella lettura dal file '%s'.\n", filename);
    }

    // Chiudere il file
    if (closeFile(handle) == 0) {
        printf("File '%s' chiuso con successo.\n", filename);
    } else {
        printf("Errore nella chiusura del file '%s'.\n", filename);
    }

    // Creazione di un file
    if (createFile(filename) == 0) {
        printf("File '%s' creato con successo.\n", filename);
    } else {
        printf("Errore nella creazione del file '%s'.\n", filename);
    }


    // Elencare il contenuto della directory corrente
    printf("Contenuto della directory:\n");
    listDir(); // Chiamata alla funzione per elencare directory e file

    if(changeDir("documents") != 0){
        printf("Errore nel cambio directory\n");
    }

    if (createFile(filename) == 0) {
        printf("File '%s' creato con successo.\n", filename);
    } else {
        printf("Errore nella creazione del file '%s'.\n", filename);
    }

    // Elencare il contenuto della directory corrente
    printf("Contenuto della directory:\n");
    listDir(); // Chiamata alla funzione per elencare directory e file

    if(changeDir("..") != 0){
        printf("Errore nel cambio directory\n");
    }

    // Elencare il contenuto della directory corrente
    printf("Contenuto della directory:\n");
    listDir(); // Chiamata alla funzione per elencare directory e file

    // Eliminazione del file
    if (eraseFile(filename) == 0) {
        printf("File '%s' eliminato con successo.\n", filename);
    } else {
        printf("Errore nell'eliminazione del file '%s'.\n", filename);
    }

    // Eliminazione della directory
    if (eraseDir("documents") == 0) {
        printf("Directory 'documents' eliminata con successo.\n");
    } else {
        printf("Errore nell'eliminazione della directory 'documents'.\n");
    }

    return 0;
}
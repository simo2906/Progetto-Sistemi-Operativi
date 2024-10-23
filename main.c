#include <stdio.h>
#include <string.h>
#include "file_system.h"

int main() {
    // Inizializza il file system
    initFileSystem();

    loadFileSystemState();

    
    // Stampa lo stato del file system (opzionale)
    printFat();

    saveFileSystemState();

    return 0;

}
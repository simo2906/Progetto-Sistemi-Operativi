#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "file_system.h"

int main(int argc, char *argv[]) {

    initFileSystem();

    loadFileSystemState();

    char input[256], arg1[128], arg2[128], filename[256], text[256];;

    filename[0] = '\0';
;
    FileHandle* file = NULL;

    do{
        printf("--------------------------------------------\n");

        fgets(input, sizeof(input), stdin);

        

        int err_arg = sscanf(input, "%s %s", arg1, arg2);


        if(strcmp(arg1, "exit") == 0){
            break;
        
        } else if(strcmp(arg1, "createFile") == 0 &&  err_arg == 2){
            
            if(createFile(arg2) == -1) continue;

            printf("File creato correttamente\n");
            
        } else if(strcmp(arg1, "eraseFile") == 0 && err_arg == 2){
            
            if(eraseFile(arg2) == -1){
                printf("File non presente o impossibile da eliminare\n");
                continue;
            }

            if(file != NULL && strcmp(filename, arg2) == 0){
                filename[0] = '\0';
                file = NULL;
            }

            printf("File eliminato con successo\n");
        
        } else if(strcmp(arg1, "write") == 0 && err_arg == 2){

            if(file == NULL || strcmp(filename, arg2) != 0){
                printf("File non aperto\n");
                continue;
            }

            

            printf("Inserire il testo da scrivere\n");
            fgets(text, sizeof(text), stdin);

            text[strlen(text)] += '\0';

            if(my_write(file, text, strlen(text)) == -1) continue;

            text[0] = '\0';
        
        } else if(strcmp(arg1, "open") == 0 && err_arg == 2){

            if(filename[0 != '\0']){
                printf("Chiudere il file aperto in precedenza\n");
                continue;
            }
            
            file = openFile(arg2);

            if(file == NULL){
                printf("Errore nell'apertura del file\n");
                continue;
            }

            strcpy(filename, arg2);

            
            printf("File aperto correttamente\n");

        } else if(strcmp(arg1, "read") == 0 &&  err_arg == 2){
            
            if(file == NULL || strcmp(filename, arg2) != 0){
                printf("File non aperto\n");
                continue;
            }

            char buffer[256];

            seek(file, 0);

            if(my_read(file, buffer, sizeof(buffer)) == -1){
                continue;
            }

            printf("%s", buffer);

        } else if(strcmp(arg1, "seek") == 0 &&  err_arg == 2){
            
            if(file == NULL || strcmp(filename, arg2) != 0){
                printf("File non aperto\n");
                continue;
            }

            char position[256];

            printf("Inserire la posizione del file: ");
            fgets(position, sizeof(position), stdin);

            char* endptr;
            int pos = strtol(position, &endptr, 10);

            if(seek(file, pos) == -1){
                printf("Errore nello spostamento all'interno del file\n");
                continue;
            }

            printf("Spostamento nel file completato\n");
        
        } else if(strcmp(arg1, "createDir") == 0 &&  err_arg == 2){

            if(createDir(arg2) == -1){
                continue;
            }

            printf("Creazione directory completata\n");
        
        } else if(strcmp(arg1, "eraseDir") == 0 &&  err_arg == 2){

            if(eraseDir(arg2) == -1){
                printf("Errore nella rimozione della directory\n");
                continue;
            }

            filename[0] = '\0';
            file = NULL;

            printf("Eliminazione directory avvenuta con successo\n");
        
        } else if(strcmp(arg1, "cd") == 0 &&  err_arg == 2){

            if(changeDir(arg2) == -1){
                continue;
            }

        } else if(strcmp(arg1, "listDir") == 0){
            listDir();

        } else if(strcmp(arg1, "printFat") == 0){
            printFat();

        } else if(strcmp(arg1, "close") == 0 && err_arg == 2){
            
            if(file == NULL || strcmp(filename, arg2) != 0){
                printf("File non aperto\n");
                continue;
            }

            closeFile(file);
            printf("File chiuso correttamente\n");

            filename[0] = '\0';
            file = NULL;

        } else {
            printf("comando non valido\n");
        }

    } while(1);
    
    saveFileSystemState();

    return 0;

}
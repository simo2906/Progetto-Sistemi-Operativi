#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_system.h"

FileEntry fat[max_fat];
char disk[block_size * max_block];
int block_map[max_block];

void initFileSystem(){

    for(int i = 0; i < max_fat; i++){
        fat[i].start_block = -1;
        fat[i].next_block = -1;
    }

    for(int i = 0; i < max_block; i++){
        block_map[i] = 0;
    }
}


int findFreeBlock(){
    
    for(int i = 0; i < max_block; i++){
        if(block_map[i] == 0){
            block_map[i] = 1;
            return i;
        } 
    }

    return -1;
}


void freeBlock(int block_index){

    if(block_index <= max_block && block_index >= 0) block_map[block_index] = 0;

}

void freeBlocks(int index){

    int block = fat[index].start_block;
    fat[index].start_block = -1;

    freeBlock(block);
    
    block = fat[index].next_block;
    fat[index].next_block = -1;

    while(block != -1){
        int next_block = fat[block].next_block;
        fat[block].next_block = -1;
        freeBlock(block);
        block = next_block;
    }

}

FileHandle* openFile(const char* filename){

    for(int i = 0; i < max_fat; i++){
        if(strcmp(fat[i].name, filename) == 0){
            FileHandle* file = (FileHandle *) malloc (sizeof(FileHandle));
            file->fat_position = fat[i].start_block;
            file->position = 0;

            return file;
        }
    }

    return NULL;
}

int createFile(const char* filename){

    for(int i = 0; i < max_fat; i++){
        if(fat[i].start_block == -1 && fat[i].next_block == -1){
            int block = findFreeBlock();
            if(block == -1) return -1;
        

        strncpy(fat[i].name, filename, max_filename);
        fat[i].start_block = block;
        fat[i].is_directory = 0;
        fat[i].next_block = -1;
        fat[i].size = 0;

        return i;

        }
    }
    return -1;
}

int eraseFile(const char* filename){

    for(int i = 0; i < max_fat; i++){
        if(strcmp(fat[i].name, filename) == 0 && fat[i].is_directory == 0){
            freeBlocks(i);
            fat[i].name[0] = '\0';
            fat[i].size = 0;
            return 0;
        }
    }
    return -1;
}

int write(FileHandle* handle, const char* buffer, int size){
    FileEntry* original_file = &fat[handle->fat_position];
    FileEntry* file = original_file;
    int block = file->start_block;
    int pos = handle->position;

    printf("Inizio scrittura: dimensione richiesta = %d, posizione = %d\n", size, pos);

    while(size > 0){

        while(pos > block_size){

            printf("Posizione oltre il blocco: pos = %d, blocco attuale = %d\n", pos, block);

            if(file->next_block == -1){

                int new_block = findFreeBlock();
                if(new_block == -1){
                    printf("Errore: Nessun blocco libero disponibile per l'allocazione.\n");
                    return -1;
                } 

                printf("Assegnato nuovo blocco: %d\n", new_block);
                file->next_block = new_block;
                
            }
            
            block = file->next_block;
            if(block >= max_fat){
                printf("Errore: Il blocco %d è fuori limiti.\n", block);
                return -1;
            } 

            printf("Passaggio al blocco successivo: %d\n", block);
            file = &fat[block];
            pos -= block_size;
        }



        int block_offset = pos % block_size;
        int write_size = block_size - block_offset;
        if(write_size > size) write_size = size;

        printf("Scrittura: blocco = %d, offset = %d, dimensione da scrivere = %d\n", block, block_offset, write_size);

        memcpy(&disk[block_size * block + block_offset], buffer, write_size);

        size -= write_size;
        buffer += write_size;
        pos += write_size;

        handle->position += write_size;

        printf("Aggiornamento: nuova posizione = %d, dimensione rimanente = %d\n", handle->position, size);

    }

    if (handle->position > original_file->size) {
        original_file->size = handle->position;
        printf("Dimensione del file aggiornata a: %d\n", file->size);
    }
    
    printf("Scrittura completata con successo.\n");
    return 0;
}


int read(FileHandle* handle, char* buffer, int size){
    FileEntry* file = &fat[handle->fat_position];
    int block = file->start_block;
    int pos = handle->position;

     printf("Inizio lettura: dimensione richiesta = %d, posizione = %d, dimensione file = %d\n", size, pos, file->size);

    if(pos + size > file->size){
        size = file->size - pos;
        printf("Dimensione regolata a = %d\n", size);
    } 
    
    while(size > 0){

        int block_offset = pos % block_size;
        int read_size = block_size - block_offset;
        if(read_size > size) read_size = size;

        printf("Lettura: blocco = %d, offset = %d, dimensione da leggere = %d\n", block, block_offset, read_size);

        memcpy(buffer, &disk[block_size * block + block_offset], read_size);

        size -= read_size;
        buffer += read_size;
        pos += read_size;
        handle->position += read_size;

        printf("Aggiornamento: nuova posizione = %d, dimensione rimanente = %d\n", handle->position, size);

        if(pos > block_size) {
            if (file->next_block == -1) {
                printf("Errore: nessun blocco successivo disponibile.\n");
                return -1;
            }

            printf("Passaggio al blocco successivo: %d\n", block);

            block = file->next_block;
            if(block > max_fat) return -1;

            file = &fat[block];
            pos = 0;
        }

        
    }

    printf("Lettura completata con successo.\n");

    return 0;
}


int seek(FileHandle* handle, int position){
    FileEntry* file = &fat[handle->fat_position];

    if(position < 0 || position > file->size) return -1;

    handle->position = position;
    return 0;
}

int createDir(const char* filename){
    for(int i = 0; i < max_fat; i++){
        if(fat[i].start_block == -1){
            
            int block = findFreeBlock();
            if (block == -1) return -1;

            strncpy(fat[i].name, filename, max_filename);
            fat[i].start_block = block;
            fat[i].size = 0;
            fat[i].is_directory = 1;
            fat->next_block = -1;

            return i;
        }
    }

    return -1;
}

int eraseDir(const char* filename){

    for(int i = 0; i < max_fat; i++){
        if(strcmp(filename, fat[i].name) == 0 && fat[i].is_directory == 1){
            freeBlocks(fat[i].start_block);
            fat[i].start_block = -1;
            return 0;
        }
    }

    return -1;
}

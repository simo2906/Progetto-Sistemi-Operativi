#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For mmap and munmap
#include <unistd.h>     // For close
#include "file_system.h"

FileEntry fat[max_fat];
char disk[block_size * max_block];
int block_map[max_block];
int current_directory;
int count_dir = 0;
linked_dir* dir, *dir_temp;

void initFileSystem(){

    for(int i = 0; i < max_fat; i++){
        fat[i].start_block = -1;
        fat[i].next_block = -1;
        fat[i].size = 0;
    }

    for(int i = 0; i < max_block; i++){
        block_map[i] = 0;
    }

    dir = (linked_dir*) malloc(sizeof(linked_dir));
    dir->dir = -1;
    dir->name_dir = "root";
    dir->next = NULL;

    dir_temp = dir;

    current_directory = -1;
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

    for (int i = 0; i < max_fat; i++) {
        if (strcmp(fat[i].name, filename) == 0 && fat[i].directory->prev == current_directory) {
            printf("Errore: Esiste già una directory o un file con questo nome.\n");
            return -1;
        }
    }
    for(int i = 0; i < max_fat; i++){
        if(fat[i].start_block == -1 && fat[i].next_block == -1){
            int block = findFreeBlock();
            if(block == -1) return -1;
        

        strncpy(fat[i].name, filename, max_filename);
        fat[i].start_block = block;
        fat[i].directory = (Dir*)malloc(sizeof(Dir));
        fat[i].directory->prev = current_directory;
        fat[i].directory->curr = 0;
        fat[i].next_block = -1;
        fat[i].size = 0;

        return 0;

        }
    }
    return -1;
}

int eraseFile(const char* filename){

    for(int i = 0; i < max_fat; i++){
        if(strcmp(fat[i].name, filename) == 0 && fat[i].directory->curr == 0){
            freeBlocks(i);
            fat[i].name[0] = '\0';
            fat[i].size = 0;
            fat[i].directory->prev = 0;
            fat[i].directory->curr = 0;
            return 0;
        }
    }
    return -1;
}

int my_write(FileHandle* handle, const char* buffer, int size){
    FileEntry* original_file = &fat[handle->fat_position];
    FileEntry* file = original_file;
    int block = file->start_block;
    int pos = handle->position;

    int fd = open("virtual_disk", O_RDWR | O_CREAT, 0666);
    if(fd == -1){
        printf("Errore nell'apertura\n");
        return -1;
    }

    if(ftruncate(fd,block_size * max_block) == -1){
        printf("Errore nell'estensione file\n");
        return -1;
    }

    char* mapped_disk = mmap(NULL, block_size * max_block, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(mapped_disk == MAP_FAILED) {
        printf("errore nel mappare il disco\n");
        return -1;
    }


    while(size > 0){

        while(pos > block_size){


            if(file->next_block == -1){

                int new_block = findFreeBlock();
                if(new_block == -1){
                    printf("Nessun blocco disponibile\n");
                    munmap(mapped_disk, block_size * max_block);
                    close(fd);
                    return -1;
                } 

                file->next_block = new_block;
                
            }
            
            block = file->next_block;
            if(block >= max_fat){
                printf("Errore: Il blocco %d è fuori limiti.\n", block);
                munmap(mapped_disk, block_size * max_block);
                close(fd);
                return -1;
            } 

            file = &fat[block];
            pos -= block_size;
        }



        int block_offset = pos % block_size;
        int write_size = block_size - block_offset;
        if(write_size > size) write_size = size;


        memcpy(&disk[block_size * block + block_offset], buffer, write_size);

        size -= write_size;
        buffer += write_size;
        pos += write_size;

        handle->position += write_size;

    }

    if (handle->position > original_file->size) {
        original_file->size = handle->position;
        printf("Dimensione del file aggiornata a: %d\n", file->size);
    }

    if(msync(mapped_disk, block_size * max_block, MS_SYNC) == -1){
        printf("Errore durante la sincronizzazione\n");
        return -1;
    }

    if(munmap(mapped_disk, block_size * max_block) == -1){
        printf("Errore durante la smappatura");
        return -1;
    }
    
    close(fd);

    printf("Scrittura completata con successo.\n");
    return 0;
}


int my_read(FileHandle* handle, char* buffer, int size){
    FileEntry* file = &fat[handle->fat_position];
    int block = file->start_block;
    int pos = handle->position;

    int fd = open("virtual_disk", 0666);
    if(fd == -1){
        printf("Errore nell'apertura\n");
        return -1;
    }

    char* mapped_disk = mmap(NULL, block_size * max_block, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(mapped_disk == MAP_FAILED) {
        printf("errore nel mappare il disco\n");
        return -1;
    }


    if(pos + size > file->size){
        size = file->size - pos;
    } 
    
    while(size > 0){

        int block_offset = pos % block_size;
        int read_size = block_size - block_offset;
        if(read_size > size) read_size = size;

        memcpy(buffer, &disk[block_size * block + block_offset], read_size);

        size -= read_size;
        buffer += read_size;
        pos += read_size;
        handle->position += read_size;

        if(pos > block_size) {
            if (file->next_block == -1) {
                printf("Errore: nessun blocco successivo disponibile.\n");
                munmap(mapped_disk, block_size * max_block);
                close(fd);
                return -1;
            }


            block = file->next_block;
            if(block > max_fat) return -1;

            file = &fat[block];
            pos = 0;
        }

        
    }

    if(munmap(mapped_disk, block_size * max_block) == -1){
        printf("Errore durante la smappatura");
        return -1;
    }

    close(fd);

    printf("Lettura completata con successo.\n");

    return 0;
}


int seek(FileHandle* handle, int position){
    FileEntry* file = &fat[handle->fat_position];

    if(position < 0 || position > file->size) return -1;

    handle->position = position;
    return 0;
}

int createDir(const char* dirname){

    for (int i = 0; i < max_fat; i++) {
        if (strcmp(fat[i].name, dirname) == 0 && fat[i].directory->prev == current_directory) {
            printf("Errore: Esiste già una directory o un file con questo nome.\n");
            return -1;
        }
    }

    for(int i = 0; i < max_fat; i++){
        if(fat[i].start_block == -1 && fat[i].next_block == -1){
            
            int block = findFreeBlock();
            if (block == -1) return -1;

            strncpy(fat[i].name, dirname, max_filename);
            fat[i].start_block = block;
            fat[i].size = 0;
            fat[i].directory = (Dir*)malloc(sizeof(Dir));
            fat[i].directory->prev = current_directory;
            count_dir++;
            fat[i].directory->curr = count_dir;
            fat[i].next_block = -1;

            linked_dir* temp = (linked_dir*) malloc(sizeof(linked_dir));
            temp->name_dir = strdup(fat[i].name);
            temp->dir = fat[i].directory->curr;

            dir_temp->next = temp;
            dir_temp = dir_temp->next;

            return i;
        }
    }

    return -1;
}

int eraseDir(const char* dirname){

    for(int i = 0; i < max_fat; i++){
        if(strcmp(dirname, fat[i].name) == 0 && fat[i].directory->curr != 0){
            freeBlocks(i);
            fat[i].name[0] = '\0';
            fat[i].size = 0;
            fat[i].directory->prev = 0;
            fat[i].directory->curr = 0;
            return 0;
        }
    }

    return -1;
}

int changeDir(const char* dirname){

    if(strcmp(dirname, "..") == 0){

        if(current_directory == -1){
            printf("Sei gia nella root\n");
            return 0;
        }

        for(int i = 0; i < max_fat; i++){

            if(fat[i].directory != NULL && fat[i].directory->curr == current_directory){

                

                current_directory = fat[i].directory->prev;

                linked_dir* temp = dir;
                
                while(temp != NULL) {
                    if(temp->dir == current_directory){
                        printf("Sei nella cartella %s\n", temp->name_dir);
                        return 0;
                    }

                    temp = temp->next;
                }                
                
            }
        }

        printf("Impossibile tornare nella cartella precedente\n");
        return -1;

        
    }

    for(int i = 0; i < max_fat; i++){

        if(strncmp(fat[i].name, dirname, max_filename) == 0){

            current_directory = fat[i].directory->curr;
            printf("Sei nella directory %s\n", fat[i].name);
            return 0;

        }
    }

    printf("La directory non esiste\n");
    return -1;
}


int closeFile(FileHandle* handle) {

    if (handle == NULL) {
        printf("Errore: handle del file nullo.\n");
        return -1;
    }

    free(handle);

    return 0;
}


int listDir(){

    for(int i = 0; i < max_fat; i++){
        if(fat[i].directory != NULL && fat[i].directory->prev == current_directory){

            printf("%s\n", fat[i].name);

        }
    }

    return 0;
}

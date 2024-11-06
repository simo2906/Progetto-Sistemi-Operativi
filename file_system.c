#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>      
#include <sys/mman.h>   
#include <unistd.h>     
#include "file_system.h"

int capacity_fat;
FileEntry* fat = NULL;

char* mapped_disk;
int block_map[max_block];
int current_directory;
int count_dir = 0;
linked_dir* dir, *dir_temp;

int create_block_fat(int block){

    FileEntry* new_fat = (FileEntry *) realloc(fat, block * sizeof(FileEntry));
    
    
    if (!new_fat) {
        printf("Errore durante la riallocazione della memoria per FAT.\n");
        return -1;
    }
    
    fat = new_fat;

    

    for(int i = capacity_fat; i < block; i++){
        fat[i].name[0] = '\0';
        fat[i].start_block = -1;
        fat[i].next_block = -1;
        fat[i].size = 0;
    }
    
    capacity_fat = block;

    return 0;
    
}

void initFileSystem(){


    for(int i = 0; i < max_block; i++){
        block_map[i] = 0;
    }

    dir = (linked_dir*) malloc(sizeof(linked_dir));
    dir->dir = -1;
    dir->name_dir = "root";
    dir->next = NULL;

    dir_temp = dir;

    current_directory = -1;

    int fd = open("virtual_disk", O_RDWR);
    
    if(fd == -1){

        fd = open("virtual_disk", O_RDWR | O_CREAT, 0666);
        if(fd == -1){
            perror("Errore nell'apertura\n");
            exit(EXIT_FAILURE);
        }

        size_t total_size = block_size * max_block + sizeof(int) + sizeof(FileEntry);

        if (ftruncate(fd, total_size) == -1) {
            perror("Errore nell'estensione del file virtual_disk");
            close(fd);
            exit(EXIT_FAILURE);
        }
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

    for(int i = 0; i < capacity_fat; i++){
        if(strcmp(fat[i].name, filename) == 0 && fat[i].curr == 0 && fat[i].prev == current_directory){
            FileHandle* file = (FileHandle *) malloc (sizeof(FileHandle));
            file->fat_position = i;
            file->position = 0;

            return file;
        }
    }

    return NULL;
}

int createFile(const char* filename){

    for (int i = 0; i < capacity_fat; i++) {
        if (strcmp(fat[i].name, filename) == 0 && fat[i].prev == current_directory) {
            printf("Errore: Esiste già una directory o un file con questo nome.\n");
            return -1;
        }
    }
    for(int i = 0; i < capacity_fat; i++){
        if(fat[i].start_block == -1 && fat[i].next_block == -1){
            int block = findFreeBlock();
            if(block == -1) return -1;
        

            strncpy(fat[i].name, filename, max_filename);
            fat[i].start_block = block;
            fat[i].prev = current_directory;
            fat[i].curr = 0;
            fat[i].next_block = -1;
            fat[i].size = 0;

            return 0;

        }

        
    }

    if(create_block_fat(capacity_fat + 1) == -1){
        printf("Errore nella riallocazione\n");
        return -1;
    }

    int block = findFreeBlock();
    if(block == -1) return -1;

    strncpy(fat[capacity_fat - 1].name, filename, max_filename);
    fat[capacity_fat - 1].start_block = block;
    fat[capacity_fat - 1].prev = current_directory;
    fat[capacity_fat - 1].curr = 0;
    fat[capacity_fat - 1].next_block = -1;
    fat[capacity_fat - 1].size = 0;

    return 0;
}

int eraseFile(const char* filename){

    

    for(int i = 0; i < capacity_fat; i++){
        if(strcmp(fat[i].name, filename) == 0 && fat[i].curr == 0){

            for(int j = 0; j < capacity_fat; j++){
                if(fat[j].curr == fat[i].prev){
                    fat[j].size -= fat[i].size;
                    break;
                }
            }
            freeBlocks(i);
            fat[i].name[0] = '\0';
            fat[i].size = 0;
            fat[i].prev = 0;
            fat[i].curr = 0;
            return 0;
        }
    }
    return -1;
}

int size_handle(FileHandle* handle){
    FileEntry* file = &fat[handle->fat_position];
    return file->size;
}

int my_write(FileHandle* handle, const char* buffer, int size){
    FileEntry* original_file = &fat[handle->fat_position];
    FileEntry* file = original_file;

    if(file->curr != 0){
        printf("Quello su cui stai scrivendo non è un file\n");
        return -1;
    }
    int block = file->start_block;
    int pos = handle->position;

    int fd = open("virtual_disk", O_RDWR, 0666);
    if(fd == -1){
        printf("Errore nell'apertura\n");
        return -1;
    }

    mapped_disk = mmap(NULL, block_size * max_block, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(mapped_disk == MAP_FAILED) {
        printf("errore nel mappare il disco\n");
        return -1;
    }


    if (pos > original_file->size) {
        pos = original_file->size;
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
            if(block >= capacity_fat){


                if(create_block_fat(block + 1) == -1){
                    printf("Errore nella riallocazione\n");
                    return -1;
                }
            } 
            
            file = &fat[block];
            pos -= block_size;
        }


        
        int block_offset = pos % block_size;
        int write_size = block_size - block_offset;
        if(write_size > size) write_size = size;


        memcpy(mapped_disk + block * block_size + block_offset, buffer, write_size);
        
        size -= write_size;
        buffer += write_size;
        pos += write_size;

        handle->position += write_size;

    }
    
    if (handle->position > original_file->size) {
        original_file->size = handle->position;
        printf("Dimensione del file aggiornata a: %d\n", original_file->size);
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

    if(file->prev != -1){
        for(int i = 0; i < capacity_fat; i++){
            if(fat[i].curr == file->prev){
                fat[i].size += file->size;
                break;
            }
        }
    }

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

    mapped_disk = mmap(NULL, block_size * max_block, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
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

        memcpy(buffer, mapped_disk + block * block_size + block_offset, read_size);

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
            if(block > capacity_fat) return -1;

            file = &fat[block];
            pos = 0;
        }

        
    }

    if(munmap(mapped_disk, block_size * max_block) == -1){
        printf("Errore durante la smappatura");
        return -1;
    }

    close(fd);

    return 0;
}


int seek(FileHandle* handle, int position){
    FileEntry* file = &fat[handle->fat_position];

    if(position < 0 || position > file->size) return -1;

    handle->position = position;
    return 0;
}

int createDir(const char* dirname){

    for (int i = 0; i < capacity_fat; i++) {
        if (strcmp(fat[i].name, dirname) == 0 && fat[i].prev == current_directory) {
            printf("Errore: Esiste già una directory o un file con questo nome.\n");
            return -1;
        }
    }

    for(int i = 0; i < capacity_fat; i++){
        if(fat[i].start_block == -1 && fat[i].next_block == -1){
            
            int block = findFreeBlock();
            if (block == -1) return -1;

            strncpy(fat[i].name, dirname, max_filename);
            fat[i].start_block = block;
            fat[i].size = 0;
            fat[i].prev = current_directory;
            count_dir++;
            fat[i].curr = count_dir;
            fat[i].next_block = -1;

            linked_dir* temp = (linked_dir*) malloc(sizeof(linked_dir));
            temp->name_dir = strdup(fat[i].name);
            temp->dir = fat[i].curr;

            dir_temp->next = temp;
            dir_temp = dir_temp->next;

            return i;
        }
    }

    if(create_block_fat(capacity_fat + 1) == -1){
        printf("Errore nella riallocazione\n");
        return -1;
    }

    int block = findFreeBlock();
    if (block == -1) return -1;

    strncpy(fat[capacity_fat - 1].name, dirname, max_filename);
    fat[capacity_fat - 1].start_block = block;
    fat[capacity_fat - 1].size = 0;
    fat[capacity_fat - 1].prev = current_directory;
    count_dir++;
    fat[capacity_fat - 1].curr = count_dir;
    fat[capacity_fat - 1].next_block = -1;

    linked_dir* temp = (linked_dir*) malloc(sizeof(linked_dir));

    temp->name_dir = strdup(fat[capacity_fat - 1].name);
    temp->dir = fat[capacity_fat - 1].curr;
    dir_temp->next = temp;
    dir_temp = dir_temp->next;

    return capacity_fat - 1;
    
}

int eraseDir(const char* dirname){


    for(int i = 0; i < capacity_fat; i++){
        if(strcmp(dirname, fat[i].name) == 0 && fat[i].curr != 0){

            for(int j = 0; j < capacity_fat; j++){
                if(fat[i].curr == fat[j].prev){
                    eraseFile(fat[j].name);
                }
            }

            freeBlocks(i);
            fat[i].name[0] = '\0';
            fat[i].size = 0;
            fat[i].prev = 0;
            fat[i].curr = 0;
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

        for(int i = 0; i < capacity_fat; i++){

            if(fat[i].curr == current_directory){

                

                current_directory = fat[i].prev;

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

        printf("Impossibile tornare nella cartella precedente");
        return -1;

        
    }


    for(int i = 0; i < capacity_fat; i++){

        if(strncmp(fat[i].name, dirname, max_filename) == 0){

            if(fat[i].curr != 0){

                current_directory = fat[i].curr;
                printf("Sei nella directory %s\n", fat[i].name);
                return 0;

            } else {

                printf("Errore non è una directory\n");
                return -1;
            }

            
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

    for(int i = 0; i < capacity_fat; i++){
        if(fat[i].prev == current_directory){
            if(fat[i].curr == 0){
                printf("[File]  %s  [%d bit]\n", fat[i].name, fat[i].size);
            } else {
                printf("[Dir]   %s  [%d bit]\n", fat[i].name, fat[i].size);
            }
            

        }
    }

    return 0;
}

int saveFileSystemState() {

    int fd = open("virtual_disk", O_RDWR);
    if (fd == -1) {
        perror("Errore nell'apertura del file virtual_disk");
        return -1;
    }
    
    size_t total_size = block_size * max_block + sizeof(int) + sizeof(FileEntry) * capacity_fat;

    if (ftruncate(fd, total_size) == -1) {
        perror("Errore nell'estensione del file virtual_disk");
        close(fd);
        return -1;
    }


    mapped_disk = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_disk == MAP_FAILED) {
        perror("Errore nel mappare il disco");
        close(fd);
        return -1;
    }

    memcpy(mapped_disk + block_size * max_block, &capacity_fat, sizeof(int));
    memcpy(mapped_disk + block_size * max_block + sizeof(int), fat, sizeof(FileEntry) * capacity_fat);
    


    if (msync(mapped_disk, total_size, MS_SYNC) == -1) {
        perror("Errore durante la sincronizzazione");
        munmap(mapped_disk, block_size * max_block);
        close(fd);
        return -1;
    }


    if (munmap(mapped_disk, total_size) == -1) {
        perror("Errore durante la smappatura");
        close(fd);
        return -1;
    }

    close(fd);

    printf("Stato del filesystem salvato con successo.\n");
    
    return 0;
}


int loadFileSystemState() {
    
    int fd = open("virtual_disk", O_RDWR);
    if (fd == -1) {
        perror("Errore nell'apertura del file virtual_disk");
        return -1;
    }

    size_t total_size = block_size * max_block + sizeof(int) + sizeof(FileEntry);

    
    mapped_disk = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_disk == MAP_FAILED) {
        perror("Errore nel mappare il disco");
        close(fd);
        return -1;
    }
    
    memcpy(&capacity_fat, mapped_disk + block_size * max_block, sizeof(int));

    total_size = block_size * max_block + sizeof(int) + sizeof(FileEntry) * capacity_fat;

    mapped_disk = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_disk == MAP_FAILED) {
        perror("Errore nel mappare il disco");
        close(fd);
        return -1;
    }

    fat = realloc(fat, sizeof(FileEntry) * capacity_fat);
    if (fat == NULL) {
        perror("Errore nella riallocazione della FAT");
        munmap(mapped_disk, total_size);
        close(fd);
        return -1;
    }

    memcpy(fat, mapped_disk + block_size * max_block + sizeof(int), sizeof(FileEntry) * capacity_fat);
    


    if (munmap(mapped_disk, total_size) == -1) {
        perror("Errore durante la smappatura");
        close(fd);
        return -1;
    }

    close(fd);

    printf("Stato del filesystem caricato con successo.\n");

    return 0;
}

void printFat(){

    for (int i = 0; i < capacity_fat; i++) {
        printf("FileEntry %d: start_block=%d, size=%d, next_block=%d name=%s\n",
               i, fat[i].start_block, fat[i].size, fat[i].next_block, fat[i].name);
    }

}

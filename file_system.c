#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_system.h"

FileEntry fat[max_file];
char disk[block_size * max_block];
int block_map[max_block];

void initFileSystem(){

    for(int i = 0; i < max_file; i++){
        fat[i].start_block = -1;
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

void freeBlocks(int start_block){

    int block = start_block;
    int next_block;
    while(block != -1){
        next_block = fat[block].next_block;
        freeBlock(block);
        block = next_block;
    }

}


int createFile(const char* filename){

    for(int i = 0; i < max_file; i++){
        if(fat[i].start_block == -1){
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

    for(int i = 0; i < max_file; i++){
        if(strcmp(fat[i].name, filename) == 0 && fat[i].is_directory == 0){
            freeBlocks(fat[i].start_block);
            fat[i].start_block = -1;
            return 0;
        }
    }
    return -1;
}

void listDir() {
    printf("File nella directory corrente:\n");
    for (int i = 0; i < max_file; i++) {
        if (fat[i].start_block != -1 && fat[i].is_directory == 0) {  
            printf("Nome: %s, Blocco iniziale: %d, Dimensione: %d bytes\n",
                fat[i].name, fat[i].start_block, fat[i].size);
        }
    }
}

void printFAT() {
    printf("Stato della FAT:\n");
    for (int i = 0; i < max_file; i++) {
        if (fat[i].start_block != -1) { 
            printf("File: %s, Blocco iniziale: %d, Prossimo blocco: %d, Dimensione: %d bytes\n",
                fat[i].name, fat[i].start_block, fat[i].next_block, fat[i].size);
        }
    }
}

int write(FileHandle* handle, const char* buffer, int size){
    FileEntry* file = &fat[handle->fat_position];
    int block = file->start_block;
    int pos = handle->position;

    while(size > 0){
        int block_offset = pos % block_size;
        int write_size = block_size - block_offset;
        if(write_size > size) write_size = size;

        memcpy(&disk[block_size * block + block_offset], buffer, write_size);

        size -= write_size;
        pos += write_size;

        if(pos >= block_size){
            if(file->next_block == -1){
                int new_block = findFreeBlock();
                if(new_block == -1) return -1;
                file->next_block = new_block;
            }
        }

        block = file->next_block;
    }

    file->size = pos > file->size? pos : file->size;
    handle->position = pos;
    
    return 0;
}




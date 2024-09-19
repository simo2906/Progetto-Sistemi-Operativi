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
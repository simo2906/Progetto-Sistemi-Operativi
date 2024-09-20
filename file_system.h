
#define max_filename 256
#define max_block 1024
#define block_size 512
#define max_file 100

typedef struct{

    char name[max_filename];
    int size;
    int start_block;
    int is_directory;
    int next_block;

} FileEntry;

typedef struct{
    
    int position;
    int fat_position;

} FileHandle;

void initFileSystem();
int createFile(const char* filename);
int eraseFile(const char* filename);
int findFreeBlock();
void freeBlock(int block_index);
void freeBlocks(int start_block);
void listDir();
void printFAT();
int write(FileHandle* handle, const char* buffer, int size);
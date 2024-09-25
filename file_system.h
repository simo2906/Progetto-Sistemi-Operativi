
#define max_filename 256
#define max_block 1024
#define block_size 512
#define max_fat 100

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
FileHandle* openFile(const char* filename);
int eraseFile(const char* filename);
int findFreeBlock();
void freeBlock(int block_index);
void freeBlocks(int start_block);
void listDir();
void printFAT();
int write(FileHandle* handle, const char* buffer, int size);
int read(FileHandle* handle, char* buffer, int size);
int seek(FileHandle* handle, int position);
int createDir(const char* filename);
int eraseDir(const char* filename);
int changeDir(const char* filename);
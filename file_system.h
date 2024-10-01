
#define max_filename 256
#define max_block 1024
#define block_size 512
#define max_fat 100

typedef struct FileEntry{

    char name[max_filename];
    int size;
    int start_block;
    struct Dir* directory;
    int next_block;

} FileEntry;

typedef struct Dir{
    
    int prev;
    int curr;

} Dir;

typedef struct linked_dir{

    int dir;
    char* name_dir;
    struct linked_dir* next;

} linked_dir;

typedef struct FileHandle{
    
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
int listDir();
int my_write(FileHandle* handle, const char* buffer, int size);
int my_read(FileHandle* handle, char* buffer, int size);
int seek(FileHandle* handle, int position);
int createDir(const char* dirname);
int eraseDir(const char* dirname);
int changeDir(const char* dirname);
int closeFile(FileHandle* handle);
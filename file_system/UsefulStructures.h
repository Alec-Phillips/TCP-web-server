

typedef struct File {
    char *name;                 // the name of the file (w/ extension)
    char *data;                 // assumes we store file data as byte array
} File;

typedef struct FileNode {
    /*  the Node structure will point to either a File or Directory
        (hence the void ptr)
        implements a simple linked list node, basically
    */
    struct File *data;
    struct FileNode *next;
} FileNode;

typedef struct DirNode {
    /*  the Node structure will point to either a File or Directory
        (hence the void ptr)
        implements a simple linked list node, basically
    */
    struct Directory *data;
    struct DirNode *next;
} DirNode;

typedef struct Directory {
    char *name;                 // the name of this directory
    struct FileNode *files;                // pts to list of files in this directory
    struct DirNode *childDirs;            // pts to list of child directories
    struct Directory *parentDir;       // pts to its parent directory
    int numFiles;
    int numChildDirs;
} Directory;






FileNode* createFileNode();
void addFileNode(FileNode *head, FileNode *newNode);
int removeFileNode(FileNode *head, char *targetName);
void freeFileNode(FileNode *targetNode);

DirNode* createDirNode();
void addDirNode(DirNode *head, DirNode *newNode);
Directory* initializeRootDirectory();
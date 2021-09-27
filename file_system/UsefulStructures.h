


typedef struct FileNode {
    /*  the Node structure will point to either a File or Directory
        (hence the void ptr)
        implements a simple linked list node, basically
    */
    File *data;
    FileNode *next;
} FileNode;

typedef struct DirNode {
    /*  the Node structure will point to either a File or Directory
        (hence the void ptr)
        implements a simple linked list node, basically
    */
    Directory *data;
    DirNode *next;
} DirNode;

typedef struct Directory {
    char *name;                 // the name of this directory
    FileNode *files;                // pts to list of files in this directory
    DirNode *childDirs;            // pts to list of child directories
    Directory *parentDir;       // pts to its parent directory
    int numFiles;
    int numChildDirs;
} Directory;


typedef struct File {
    char *name;                 // the name of the file (w/ extension)
    char *data;                 // assumes we store file data as byte array
} File;



FileNode* createFileNode();
void addFileNode(FileNode *head, FileNode *newNode);

DirNode* createDirNode();
void addDirNode(DirNode *head, DirNode *newNode);
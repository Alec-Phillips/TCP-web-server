


typedef struct Node {
    /*  the Node structure will point to either a File or Directory
        (hence the void ptr)
        implements a simple linked list node, basically
    */
    void *data;
    Node *next;
} Node;


typedef struct Directory {
    char *name;                 // the name of this directory
    Node *files;                // pts to list of files in this directory
    Node *childDirs;            // pts to list of child directories
    Directory *parentDir;       // pts to its parent directory
    int numFiles;
    int numChildDirs;
} Directory;


typedef struct File {
    char *name;                 // the name of the file (w/ extension)
    char *data;                 // assumes we store file data as byte array
} File;



Node* createNode();
void addNode(Node *head, Node *newNode);

#include <stdio.h>
#include <stdlib.h>
#include "UsefulStructures.h"


File* fopen(char *path, Directory *root);

void makeDirectory(char *name, Directory *parentDir);

void createFile(char *name, Directory *pwd);

Directory* changeDirectory(char *targetDir, Directory *pwd);


File* fopen(char *path, Directory *root) {
    int nextDir = 1;
    for (int i = 1; i < strlen(path); i++) {
        if (*(path + i) == "/") {
            *(path + i) = '\0';
            root = changeDirectory(path + nextDir, root);
            if (root == NULL) {
                return NULL;
            }
            nextDir = i + 1;
        }
    }
    *(path + strlen(path)) = "\0";
    FileNode *files = root->files;
    while (files != NULL) {
        File *file = files->data;
        if (strcmp(file->name, path + nextDir)) {
            return file;
        }
        files = files->next;
    }
    return NULL;
}

void makeDirectory(char* name, Directory *parentDir) {
    /*
        creates a new Directory with the given name
        creates a new Node to hold the Directory so that
            it can be added to its parent directory's
            childDirectories list
    */
    Directory *newDir = malloc(sizeof(Directory));
    newDir->name = name;
    newDir->parentDir = parentDir;
    newDir->files = NULL;
    newDir->childDirs = NULL;
    newDir->numFiles = 0;
    newDir->numChildDirs = 0;
    DirNode *newDirNode = createDirNode();
    newDirNode->data = newDir;
    if (parentDir->childDirs == NULL) {
        parentDir->childDirs = newDirNode;
    }
    else {
        addDirNode(parentDir->childDirs, newDirNode);
    }
}

void createFile(char *name, Directory *pwd) {
    /*
        creates a new File holder with the given name
        creates a new Node to hold the File, so that it
            can be added to the current directory's files list
    */
    File *newFile = malloc(sizeof(File));
    newFile->name = name;
    newFile->data = malloc(20000);
    FileNode *newFileNode = createFileNode();
    newFileNode->data = newFile;
    if (pwd->files == NULL) {
        pwd->files = newFileNode;
    }
    else {
        addFileNode(pwd->files, newFileNode);
    }
}

Directory* changeDirectory(char *targetDir, Directory *pwd) {
    DirNode *childDirs = pwd->childDirs;
    while (childDirs != NULL) {
        Directory *currChild = childDirs->data;
        if (strcmp(currChild->name, targetDir)) {
            return currChild;
        }
        currChild = childDirs->next;
    }
    return NULL;
}



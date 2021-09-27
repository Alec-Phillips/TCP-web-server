
#include <stdio.h>
#include <stdlib.h>
#include "UsefulStructures.h"


void makeDirectory(char *name, Directory *parentDir);

void createFile(char *name, Directory *pwd);

Directory* changeDirectory(char *targetDir, Directory *pwd);


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
    Node *newDirNode = createNode();
    newDirNode->data = newDir;
    if (parentDir->childDirs == NULL) {
        parentDir->childDirs = newDirNode;
    }
    else {
        addNode(parentDir->childDirs, newDirNode);
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
    Node *newFileNode = createNode();
    newFileNode->data = newFile;
    if (pwd->files == NULL) {
        pwd->files = newFileNode;
    }
    else {
        addNode(pwd->files, newFileNode);
    }
}



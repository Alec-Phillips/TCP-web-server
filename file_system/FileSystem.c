
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FileSystem.h"
#include "UsefulStructures.h"





Directory* initializeRoot() {
    Directory* root = initializeRootDirectory();
    return root;
}

char* openFile(char *path, Directory *root) {
    int nextDir = 1;
    puts(path);
    for (int i = 1; i < strlen(path); i++) {
        if (*(path + i) == '/') {
            // *(path + i) = '\0';
            char currPath[i - nextDir];
            strncpy(currPath, path + nextDir, i - nextDir);
            puts(currPath);
            puts(root->name);
            root = changeDirectory(currPath, root);
            if (root == NULL) {
                return NULL;
            }
            nextDir = i + 1;
        }
    }
    // *(path + strlen(path)) = '\0';
    FileNode *files = root->files;
    // printf("%s\n", files->data->name);
    while (files != NULL) {
        File *file = files->data;
        if (!strcmp(file->name, path + nextDir)) {
            return file->data;
        }
        files = files->next;
    }
    return NULL;
}

Directory* getDirectoryFromPath(char *path, Directory *root) {
    if (strlen(path) == 1){
        return root;
    }
    int nextDir = 1;
    for (int i = 1; i < strlen(path); i++) {
        if (*(path + i) == '/') {
            // *(path + i) = '\0';
            // root = changeDirectory(path + nextDir, root);
            // *(path + i) = '\0';
            char currPath[i - nextDir];
            strncpy(currPath, path + nextDir, i - nextDir);
            root = changeDirectory(currPath, root);
            if (root == NULL) {
                return NULL;
            }
            nextDir = i + 1;
        }
    }

    root = changeDirectory(path + nextDir, root);

    return root;
}

int uploadFile(char *path, Directory *root, char *fileData, char *fileName) {
    root = getDirectoryFromPath(path, root);
    if (root == NULL) {
        return 1;
    }
    File *newFile = malloc(sizeof(File));
    newFile->name = fileName;
    newFile->data = fileData;
    FileNode *newFileNode = malloc(sizeof(FileNode));
    newFileNode->data = newFile;
    newFileNode->next = NULL;
    if (root->files == NULL) {
        root->files = newFileNode;
    }
    else {
        FileNode *curr = root->files;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = newFileNode;
    }
    // newFile->data = realloc(newFile->data, sizeof(char) * strlen(fileData));
    // strncpy(newFile->data, fileData, strlen(fileData));

    return 0;
}

int deleteFile(char *path, Directory *root, char *fileName) {
    root = getDirectoryFromPath(path, root);
    if (root == NULL) {
        return 1;
    }
    FileNode *filesHead = root->files;
    removeFileNode(filesHead, fileName);
    return 0;
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

int deleteDirectory(char *path, Directory *root) {
    Directory *targetDir = getDirectoryFromPath(path, root);
    DirNode *head = targetDir->parentDir->childDirs;
    if (!strcmp(head->data->name, targetDir->name)) {
        targetDir->parentDir->childDirs = head->next;
        freeDirectoryNode(head);
    }
    else {
        removeDirNode(head, targetDir->name);
    }
    return 0;
}

File* createFile(char *name, Directory *pwd) {
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
    return newFile;
}

Directory* changeDirectory(char *targetDir, Directory *pwd) {
    DirNode *childDirs = pwd->childDirs;
    while (childDirs != NULL) {
        Directory *currChild = childDirs->data;
        puts(currChild->name);
        if (!strcmp(currChild->name, targetDir)) {
            return currChild;
        }
        childDirs = childDirs->next;
    }
    printf("returning null\n");
    return NULL;
}



int printFileTree(Directory *root) {

    puts(root->name);
    root = root->childDirs->data;
    while (root != NULL) {
        printf("%s\n", root->name);
        root = root->childDirs->data;
    }


    return 0;
}



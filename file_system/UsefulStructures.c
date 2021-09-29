
#include <stdlib.h>
#include <string.h>
#include "UsefulStructures.h"


FileNode* createFileNode() {
    FileNode *newNode = malloc(sizeof(FileNode));
    newNode->next = NULL;
    return newNode;
}

void addFileNode(FileNode *head, FileNode *newNode) {
    while (head->next != NULL) {
        head = head->next;
    }
    head->next = newNode;
}

int removeFileNode(FileNode *head, char *targetName) {
    FileNode *prev = NULL;
    while (strcmp(head->data->name, targetName) != 0) {
        prev = head;
        head = head->next;
        if (head == NULL) {
            return 1;
        }
    }
    prev->next = head->next;
    freeFileNode(head);
    return 0;
}

void freeFileNode(FileNode *targetNode) {
    /*
        frees the FileNode as well as the
        dynamicallly allocated File that it holds
        and the actual file contents held by the File
    */
    File *fileNodeData = targetNode->data;
    char *fileData = fileNodeData->data;
    free(fileData);
    free(fileNodeData);
    free(targetNode);
}

DirNode* createDirNode() {
    DirNode *newNode = malloc(sizeof(DirNode));
    newNode->next = NULL;
    newNode->data = NULL;
    return newNode;
}

void addDirNode(DirNode *head, DirNode *newNode) {
    while (head->next != NULL) {
        head = head->next;
    }
    head->next = newNode;
}

Directory* initializeRootDirectory() {
    Directory *root = malloc(sizeof(Directory));
    root->name = "root";
    // root->files = createFileNode();
    // root->childDirs = createDirNode();
    root->files = NULL;
    root->childDirs = NULL;
    root->parentDir = NULL;
    root->numFiles = 0;
    root->numChildDirs = 0;
    return root;
}

#include <stdlib.h>
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

DirNode* createDirNode() {
    DirNode *newNode = malloc(sizeof(DirNode));
    newNode->next = NULL;
    return newNode;
}

void addDirNode(DirNode *head, DirNode *newNode) {
    while (head->next != NULL) {
        head = head->next;
    }
    head->next = newNode;
}
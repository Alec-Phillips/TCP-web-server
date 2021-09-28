
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
    return newNode;
}

void addDirNode(DirNode *head, DirNode *newNode) {
    while (head->next != NULL) {
        head = head->next;
    }
    head->next = newNode;
}
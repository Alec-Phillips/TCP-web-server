
#include <stdlib.h>
#include "UsefulStructures.h"


Node* createNode() {
    Node *newNode = malloc(sizeof(Node));
    newNode->next = NULL;
    return newNode;
}

void addNode(Node *head, Node *newNode) {
    while (head->next != NULL) {
        head = head->next;
    }
    head->next = newNode;
}
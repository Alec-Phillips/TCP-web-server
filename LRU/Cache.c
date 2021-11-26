#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Cache.h"

LRUCache* initCache(int capacity) {
    LRUCache* newCache = malloc(sizeof(LRUCache));
    QueueNode* head = initQueue(capacity);
    HashMap* map = initMap(8 * sizeof(char)); // the datasize will be the size of a pointer
    newCache->head = head;
    newCache->map = map;
    return newCache;
}

QueueNode* addQueueNode(QueueNode* head) {
    QueueNode* curr = head;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    QueueNode* newNode = malloc(sizeof(QueueNode));
    newNode->contents = NULL;
    newNode->next = NULL;
    newNode->prev = curr;
    curr->next = newNode;
    return newNode;
}

QueueNode* initQueue(int size) {
    QueueNode* head = malloc(sizeof(QueueNode));
    head->next = NULL;
    head->prev = NULL;
    head->contents = NULL;
    for (int i = 1; i < size; i ++) {
        addQueueNode(head);
    }
    return head;
}

int updateNodeContents(QueueNode* node, char* contents) {
    node->contents = malloc(strlen(contents));
    strcpy(node->contents, contents);
    return 0;
}

char* checkCache(LRUCache* cache, char* filePath) {
    QueueNode* node = get(cache->map, filePath);
    if (node != NULL) {
        return node->contents;
    }
    return NULL;
}

int updateCache(LRUCache* cache, QueueNode* target, QueueNode* head, char* filePath, char* fileContents, int type) {
    if (type == 1) { // this is for a GET request
                     // no data needs to change
        if (target == head) {
            return 0;
        }
        if (target->next == NULL) {
            target->prev->next = NULL;
        }
        else {
            target->prev->next = target->next;
            target->next->prev = target->prev;
        }
        target->prev = NULL;
        target->next = head;
        head->prev = target;
        cache->head = target;
    }
    else if (type == 2) { // this is for a POST request
                          // override data at the tail and move to head
        QueueNode* curr = head;
        while (curr->next != NULL && curr->contents != NULL) {
            curr = curr->next;
        }
        curr->contents = malloc(strlen(fileContents));
        strcpy(curr->contents, fileContents);
        if (curr->next == NULL) {
            curr->prev->next = NULL;
            curr->next = head;
            head->prev = curr;
            cache->head = curr;
            // puts("here");
            // del(cache->map, filePath);
            // puts("didn't fail");
        }
        else if (curr != head){
            curr->prev->next = curr->next;
            curr->next->prev = curr->prev;
            curr->next = head;
            curr->prev = NULL;
            head->prev = curr;
            cache->head = curr;
        }
        put(cache->map, filePath, curr);
    }
    else if (type == 3) { // this is for a DELETE request
        target = get(cache->map, filePath);
        target->contents = NULL;
        QueueNode* tail = head;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        if (target->next != NULL) {
            if (target == head) {
                target->next->prev = NULL;
                tail->next = target;
                target->prev = tail;
                cache->head = target->next;
            }
            else {
                target->prev->next = target->next;
                target->next->prev = target->prev;
                tail->next = target;
                target->prev = tail;
            }
            target->next = NULL;
        }
    }
    else {
        return 1;
    }
}

void printCache(LRUCache* cache) {
    QueueNode* curr = cache->head;
    while (curr != NULL) {
        printf("%s -> ", curr->contents);
        curr = curr->next;
    }
    printf("NULL\n");
}

#ifndef CACHE_H
#define CACHE_H

#include "../hashmap/HashMap.h"

typedef struct QueueNode {
    char *contents;
    struct QueueNode *prev;
    struct QueueNode *next;
} QueueNode;

typedef struct LRUCache {
    HashMap *map;
    QueueNode *head;
} LRUCache;

LRUCache* initCache(int capacity);
QueueNode* addQueueNode(QueueNode* head);
QueueNode* initQueue(int size);
int updateNodeContents(QueueNode* node, char* contents);
char* checkCache(LRUCache* cache, char* filePath);
int updateCache(LRUCache* cache, QueueNode* target, QueueNode* head, char* filePath, char* fileContents, int type);
void printCache(LRUCache* cache);

#endif
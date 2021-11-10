#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct KeyValPair {
    char *key;  // this will be the filepath
    void *val;  // this will be either the sem_t or the queue node ptr
    struct KeyValPair* next;
                // this will be the next node, in case there is collision

} KeyValPair;

typedef struct HashMap {
    struct KeyValPair* buckets[16];
    int type;   // 1 indicates mapping to semaphores, 2 will be for queue nodes
} HashMap;

// this is purely for testing that map works with our own user defined types
typedef struct TestObj {
    int val;
} TestObj;

KeyValPair* initKeyValPair(char* key, void* val, int typeFlag);
HashMap* initMap(int typeFlag);
int put(HashMap* map, char* filePath, void* data);
void* get(HashMap* map, char* filePath);
int del(HashMap* map, char* filePath);

#endif
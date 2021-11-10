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
    size_t dataSize; // the size of the data that will be stored in this map
} HashMap;

// this is purely for testing that map works with our own user defined types
typedef struct TestObj {
    int val;
} TestObj;

KeyValPair* initKeyValPair(char* key, void* val, size_t dataSize);
HashMap* initMap(size_t dataSize);
int put(HashMap* map, char* filePath, void* data);
void* get(HashMap* map, char* filePath);
int del(HashMap* map, char* filePath);

#endif
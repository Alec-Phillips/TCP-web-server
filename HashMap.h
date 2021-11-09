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
} HashMap;

KeyValPair* initKeyValPair(char* key, void* val);
HashMap* initMap();
int put(HashMap* map, char* filePath, void* data);
void* get(HashMap* map, char* filePath);

#endif
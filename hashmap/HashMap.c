
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include "HashMap.h"

/* Hash map implementation that maps strings to some other data type
    will be used for:
        - mapping filenames to semaphores
        - mapping filenames to queue nodes for the LRU cache
*/

// -----------------------------------------------------------------------------
// HASH FUNCTION:
// SOURCE - https://stackoverflow.com/questions/7666509/hash-function-for-string
unsigned long
hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
// -----------------------------------------------------------------------------


KeyValPair* initKeyValPair(char* key, void* val, int typeFlag) {
    KeyValPair* keyVal = malloc(sizeof(KeyValPair));
    keyVal->key = malloc(strlen(key));
    strcpy(keyVal->key, key);
    if (typeFlag == 1) {
        keyVal->val = malloc(sizeof(sem_t));
        memcpy(keyVal->val, val, sizeof(sem_t));
    }
    else if (typeFlag == 3) {
        keyVal->val = malloc(sizeof(TestObj));
        memcpy(keyVal->val, val, sizeof(TestObj));
    }
    else {
        // this is for if we are adding a different data type
        // we will need to malloc a different sizeof amount
    }
    keyVal->next = NULL;
    return keyVal;
}

HashMap* initMap(int type) {
    HashMap* newMap = malloc(sizeof(HashMap));
    for (int i = 0; i < 16; i ++) {
        newMap->buckets[i] = NULL;
    }
    newMap->type = type;
    return newMap;
}

/*
    add an entry to the hashmap
        - get the hash of the key
        - mod by 16 to determine which bucket to insert to
        - initialize the key-val object
        - insert this into the linked list for that bucket
*/
int put(HashMap* map, char* filePath, void* data) {
    if (get(map, filePath) != NULL) {
        return 1;
    }
    unsigned long h = hash(filePath);
    int ind = h % 16;
    KeyValPair* keyVal = initKeyValPair(filePath, data, map->type);
    
    if (map->buckets[ind] == NULL) {
        map->buckets[ind] = keyVal;
    }
    else {
        KeyValPair* head = map->buckets[ind];
        while (head->next != NULL) {
            head = head->next;
        }
        head->next = keyVal;
    }
    return 0;
}

void* get(HashMap* map, char* filePath) {
    unsigned long h = hash(filePath);
    int ind = h % 16;
    KeyValPair* head = map->buckets[ind];
    while (head != NULL) {
        if (! strcmp(head->key, filePath)) {
            return head->val;
        }
        head = head->next;
    }
    return NULL;
}

// TODO
int del(HashMap* map, char* filePath) {
    unsigned long h = hash(filePath);
    int ind = h % 16;
    KeyValPair* head = map->buckets[ind];
    KeyValPair* prev = NULL;
    while (strcmp(head->key, filePath)) {
        prev = head;
        head = head->next;
    }
    if (prev == NULL) {
        map->buckets[ind] = head->next;
    }
    else {
        prev->next = head->next;
    }
    free(head);
    return 0;
}

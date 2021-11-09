
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


KeyValPair* initKeyValPair(char* key, void* val) {
    KeyValPair* keyVal = malloc(sizeof(KeyValPair));
    keyVal->key = malloc(strlen(key));
    strcpy(keyVal->key, key);

    // memcpy(keyVal->val, val, sizeof(sem_t));
    
    keyVal->val = val;  // NOT SURE IF THIS WILL WORK
                        // 'val' may get changed and alter what is pointed to here
                        // can ideally somehow memcopy over?
    keyVal->next = NULL;
    return keyVal;
}

HashMap* initMap() {
    HashMap* newMap = malloc(sizeof(HashMap));
    for (int i = 0; i < 16; i ++) {
        newMap->buckets[i] = NULL;
    }
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
    unsigned long h = hash(filePath);
    // puts("1");
    int ind = h % 16;
    KeyValPair* keyVal = initKeyValPair(filePath, data);
    
    if (map->buckets[ind] == NULL) {
        map->buckets[ind] = keyVal;
    }
    // puts("3");
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
int del(void) {
    return 0;
}

int main(void) {

    HashMap* map = initMap();

    char *filePath = "root/desktop/file1.txt";

    sem_t sem;
    sem_init(&sem, 0, 1);

    put(map, filePath, &sem);
    void* val = get(map, filePath);

    printf("%d\n", &sem);
    sem = NULL;
    printf("%d\n", val);
    printf("%d\n", &sem);


    return 0;
}
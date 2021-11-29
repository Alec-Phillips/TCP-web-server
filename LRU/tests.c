#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "Cache.h"
#include "../hashmap/HashMap.h"

// Compilation Command:
//      gcc ../hashmap/HashMap.c Cache.c tests.c -Wall -o tests

int testInit(LRUCache* cache) {
    char* contents = checkCache(cache, "this/is/a/path");
    assert (contents == NULL);
    int status = updateCache(cache, NULL, cache->head, "root/desktop", "file contents", 2);
    contents = checkCache(cache, "root/desktop");
    assert (!strcmp(contents, "file contents"));
    status = updateCache(cache, NULL, cache->head, "root/desktop", NULL, 3);
    contents = checkCache(cache, "root/desktop");
    assert(contents == NULL);

    printCache(cache);
    updateCache(cache, NULL, cache->head, "root/desktop/f1.txt", "f1", 2);
    checkCache(cache, "root/desktop/f1.txt");
    printCache(cache);
    updateCache(cache, NULL, cache->head, "root/desktop/f2.txt", "f2", 2);
    checkCache(cache, "root/desktop/f1.txt");
    printCache(cache);
    updateCache(cache, NULL, cache->head, "root/desktop/f3.txt", "f3", 2);
    checkCache(cache, "root/desktop/f1.txt");
    printCache(cache);
    updateCache(cache, NULL, cache->head, "root/desktop/f4.txt", "f4", 2);
    checkCache(cache, "root/desktop/f1.txt");
    printCache(cache);
    updateCache(cache, NULL, cache->head, "root/desktop/f0.txt", "f0", 2);
    checkCache(cache, "root/desktop/f1.txt");
    printCache(cache);
    printf("END OF TEST1\n\n\n");

    return 0;
}

int testGet(LRUCache* cache) {
    updateCache(cache, NULL, cache->head, "root/desktop/f3.txt", NULL, 1);
    printCache(cache);
    updateCache(cache, NULL, cache->head, "root/desktop/f2.txt", NULL, 1);
    printCache(cache);
    updateCache(cache, NULL, cache->head, "root/desktop/f0.txt", NULL, 1);
    printCache(cache);
    updateCache(cache, NULL, cache->head, "root/desktop/f4.txt", NULL, 1);
    printCache(cache);
    printf("END OF TEST2\n\n\n");

    return 0;
}

int testDel(LRUCache* cache) {
    updateCache(cache, NULL, cache->head, "root/desktop/f0.txt", NULL, 3);
    printCache(cache);
    updateCache(cache, NULL, cache->head, "root/desktop/f3.txt", NULL, 3);
    printCache(cache);
    updateCache(cache, NULL, cache->head, "root/desktop/f2.txt", NULL, 3);
    printCache(cache);
    updateCache(cache, NULL, cache->head, "root/desktop/f4.txt", NULL, 3);
    printCache(cache);
    printf("END OF TEST3\n\n\n");

    return 0;
}

int main(void) {
    LRUCache* cache = initCache(4);
    testInit(cache);
    testGet(cache);
    testDel(cache);
    return 0;
}
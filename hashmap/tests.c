#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <semaphore.h>
#include "HashMap.h"

// Compilation Command:
//      gcc HashMap.c tests.c -Wall -o tests

HashMap* setupMap(int type) {
    
    return NULL;
}

int testPutGet() {
    HashMap* map = initMap(3);
    TestObj* testObj = malloc(sizeof(TestObj));
    testObj->val = 5;
    put(map, "fp1", testObj);
    put(map, "fp2", testObj);
    put(map, "fp3", testObj);
    put(map, "fp4", testObj);
    put(map, "fp5", testObj);
    put(map, "fp6", testObj);
    put(map, "fp7", testObj);
    put(map, "fp8", testObj);
    put(map, "fp9", testObj);
    put(map, "fp10", testObj);
    put(map, "fp11", testObj);
    put(map, "fp12", testObj);
    put(map, "fp13", testObj);
    put(map, "fp14", testObj);
    put(map, "fp15", testObj);
    put(map, "fp16", testObj);
    put(map, "fp17", testObj);

    testObj->val = 10;
    TestObj* returned = get(map, "fp16");
    assert (returned->val == 5);
    returned->val = 7;
    returned = get(map, "fp16");
    assert (returned->val == 7);
    returned = get(map, "fp15");
    assert (returned->val == 5);

    return 0;
}

int testPutRepeat() {
    HashMap* map = initMap(3);
    TestObj* testObj = malloc(sizeof(TestObj));
    testObj->val = 5;
    int putStatus;
    putStatus = put(map, "file path", testObj);
    assert (putStatus == 0);
    putStatus = put(map, "file path", testObj);
    assert (putStatus == 1);

    return 0;
}

int testGetNull() {
    HashMap* map = initMap(3);
    void* returned = get(map, "fake file path");
    assert (returned == NULL);

    return 0;
}

int testDel() {
    HashMap* map = initMap(3);
    TestObj* testObj = malloc(sizeof(TestObj));
    testObj->val = 5;
    put(map, "fp1", testObj);
    put(map, "fp2", testObj);
    put(map, "fp3", testObj);
    put(map, "fp4", testObj);
    put(map, "fp5", testObj);
    put(map, "fp6", testObj);
    put(map, "fp7", testObj);
    put(map, "fp8", testObj);
    put(map, "fp9", testObj);
    put(map, "fp10", testObj);
    put(map, "fp11", testObj);
    put(map, "fp12", testObj);
    put(map, "fp13", testObj);
    put(map, "fp14", testObj);
    put(map, "fp15", testObj);
    put(map, "fp16", testObj);
    put(map, "fp17", testObj);

    del(map, "fp1");
    del(map, "fp2");
    del(map, "fp3");
    del(map, "fp4");
    del(map, "fp5");
    del(map, "fp6");
    del(map, "fp7");
    del(map, "fp8");
    del(map, "fp9");
    del(map, "fp10");
    del(map, "fp11");
    del(map, "fp12");
    del(map, "fp13");
    del(map, "fp14");
    del(map, "fp15");
    del(map, "fp16");
    del(map, "fp17");

    assert (get(map, "fp1") == NULL);
    assert (get(map, "fp2") == NULL);
    assert (get(map, "fp3") == NULL);
    assert (get(map, "fp4") == NULL);
    assert (get(map, "fp5") == NULL);
    assert (get(map, "fp6") == NULL);
    assert (get(map, "fp7") == NULL);
    assert (get(map, "fp8") == NULL);
    assert (get(map, "fp9") == NULL);
    assert (get(map, "fp10") == NULL);
    assert (get(map, "fp11") == NULL);
    assert (get(map, "fp12") == NULL);
    assert (get(map, "fp13") == NULL);
    assert (get(map, "fp14") == NULL);
    assert (get(map, "fp15") == NULL);
    assert (get(map, "fp16") == NULL);
    assert (get(map, "fp17") == NULL);

    put(map, "fp1", testObj);
    put(map, "fp2", testObj);
    put(map, "fp3", testObj);
    put(map, "fp4", testObj);
    put(map, "fp5", testObj);
    put(map, "fp6", testObj);
    put(map, "fp7", testObj);
    put(map, "fp8", testObj);
    put(map, "fp9", testObj);
    put(map, "fp10", testObj);
    put(map, "fp11", testObj);
    put(map, "fp12", testObj);
    put(map, "fp13", testObj);
    put(map, "fp14", testObj);
    put(map, "fp15", testObj);
    put(map, "fp16", testObj);
    put(map, "fp17", testObj);

    testObj->val = 10;
    TestObj* returned = get(map, "fp16");
    assert (returned->val == 5);
    returned->val = 7;
    returned = get(map, "fp16");
    assert (returned->val == 7);
    returned = get(map, "fp15");
    assert (returned->val == 5);
    
    return 0;
}

// test with semaphores as vals, since we will be using it for this
int semTest() {
    HashMap* map = initMap(1);

    char *filePath = "root/desktop/file1.txt";

    sem_t sem;
    sem_init(&sem, 0, 1);

    put(map, filePath, &sem);
    void* val = get(map, filePath);
    sem_wait(val);
    sem_post(val);

    printf("%d\n", &sem);
    sem = NULL;
    printf("%d\n", val);
    printf("%d\n", &sem);
    return 0;
}


int main(void) {

    testPutGet();
    testPutRepeat();
    testGetNull();
    testDel();
    // semTest();

    return 0;
}
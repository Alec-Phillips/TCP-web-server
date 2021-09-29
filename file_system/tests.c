#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "UsefulStructures.h"
#include "FileSystem.h"




int testCreateDirectories(Directory *root) {
    makeDirectory("documents", root);
    makeDirectory("desktop", root);
    Directory *documents = changeDirectory("documents", root);
    Directory *desktop = changeDirectory("desktop", root);
    makeDirectory("files", documents);
    Directory *files = changeDirectory("files", documents);

    assert(strcmp(documents->name, "documents") == 0);
    assert(strcmp(desktop->name, "desktop") == 0);
    assert(strcmp(files->name, "files") == 0);

    return 0;
}


int main(void) {
    printf("hello\n");
    Directory *root = initializeRoot();
    int result = testCreateDirectories(root);
    if (result == 0){
        printf("pass\n");
    }
    // makeDirectory("files", )
    
    return 0;
}
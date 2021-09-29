#include <stdio.h>
#include <stdlib.h>
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

    // free(documents);
    // free(desktop);
    // free(files);

    return 0;
}

int testUploadFile(Directory *root) {
    char path[] = "/desktop";
    char *fileName = "test_file.txt";
    char *fileData = "This is a test file";
    uploadFile(path, root, fileData, fileName);
    // char *fileContents = root->childDirs->next->data->files->data->data;

    char filePath[] = "/desktop/test_file.txt";
    char *fileContents = openFile(filePath, root);
    printf("%s\n", fileContents);
    return 0;
}


int main(void) {
    printf("hello\n");
    Directory *root = initializeRoot();
    int result = testCreateDirectories(root);
    if (result == 0){
        printf("pass\n");
    }
    testUploadFile(root);
    // makeDirectory("files", )
    
    return 0;
}
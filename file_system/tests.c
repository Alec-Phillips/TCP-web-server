#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "UsefulStructures.h"
#include "FileSystem.h"

// Compilation command: 
//      gcc UsefulStructures.c FileSystem.c tests.c -Wall -o tests


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

    char filePath[] = "/desktop/test_file.txt";
    char *fileContents = openFile(filePath, root);

    assert(!strcmp(fileContents, "This is a test file"));

    char newPath[] = "/documents/files";
    uploadFile(newPath, root, fileData, fileName);

    char newFilePath[] = "/documents/files/test_file.txt";
    fileContents = openFile(newFilePath, root);
    printf("%s\n", fileContents);
    assert(!strcmp(fileContents, "This is a test file"));

    return 0;
}

int testUploadToRoot(Directory *root) {

    char path[] = "/";
    char *fileName = "test_file.txt";
    char *fileData = "This is a test file";
    uploadFile(path, root, fileData, fileName);

    char filePath[] = "/test_file.txt";
    char *fileContents = openFile(filePath, root);

    assert(!strcmp(fileContents, "This is a test file"));

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
    testUploadToRoot(root);
    // makeDirectory("files", )
    
    return 0;
}
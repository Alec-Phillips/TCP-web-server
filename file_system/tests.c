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

int testFakePost(Directory *root) {
    char *fileQuery = "/desktop";
    char *fileData = "this is some cool text";
    char *fileName = "cooltext.txt";
    if (getDirectoryFromPath(fileQuery, root) == NULL) {
        printf("1\n");
        makeDirectory(fileQuery + 1, root);
    }
    getDirectoryFromPath(fileQuery, root);
    uploadFile(fileQuery, root, fileData, fileName);
    char *path = "/desktop/cooltext.txt";
    char *fileContents = openFile(path, root);
    puts(openFile(path, root));
    printf("%s\n", fileContents);
    puts(fileContents);
    return 1;
}


int main(void) {
    printf("hello\n");
    Directory *root = initializeRoot();
    // int result = testCreateDirectories(root);
    // if (result == 0){
    //     printf("pass\n");
    // }
    // testUploadFile(root);
    // testUploadToRoot(root);
    testFakePost(root);
    printf("---\n");
    puts(openFile("/desktop/cooltext.txt", root));
    return 0;
}
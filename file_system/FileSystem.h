#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "UsefulStructures.h"

Directory* initializeRoot();

char* openFile(char *path, Directory *root);

Directory* getDirectoryFromPath(char *path, Directory *root);

int uploadFile(char *path, Directory *root, char *fileData, char *fileName);

int deleteFile(char *path, Directory *root, char *fileName);

void makeDirectory(char *name, Directory *parentDir);

int deleteDirectory(char *path, Directory *root);

File* createFile(char *name, Directory *pwd);

Directory* changeDirectory(char *targetDir, Directory *pwd);

int printFileTree(Directory *root);


#endif
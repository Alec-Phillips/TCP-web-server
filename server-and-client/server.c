#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include "/Users/alecphillips/Documents/Fall 2021/COMP379/TCP-web-server/file_system/FileSystem.h" // Change
#include "/Users/alecphillips/Documents/Fall 2021/COMP379/TCP-web-server/file_system/UsefulStructures.h" // Change
#include <assert.h>

/* 
	Note:
	- You need to change the two includes above to match where the files are for you (absolute path I believe)

	Compilation command:
	gcc server.c ../file_system/FileSystem.c ../file_system/UsefulStructures.c -Wall -o se

	Run with ./se
*/


/*
	C socket server example
	https://www.binarytides.com/server-client-example-c-sockets-linux/
*/

/*
	HTTP Response Creator
*/

// Originally made by Geeks for Geeks: https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/
char* replaceWord(const char* s, const char* oldW,
                  const char* newW)
{
    char* result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);
  
    // Counting the number of times old word
    // occur in the string
    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], oldW) == &s[i]) {
            cnt++;
  
            // Jumping to index after the old word.
            i += oldWlen - 1;
        }
    }
  
    // Making new string of enough length
    result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1);
  
    i = 0;
    while (*s) {
        // compare the substring with the result
        if (strstr(s, oldW) == s) {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }
  
    result[i] = '\0';
    return result;
}

// implement here before I implement POST
// int testUploadFile(Directory *root) {
//     char path[] = "/desktop";
//     char *fileName = "test_file.txt";
//     char *fileData = "This is a test file";
//     uploadFile(path, root, fileData, fileName);

//     char filePath[] = "/desktop/test_file.txt";
//     char *fileContents = openFile(filePath, root);

//     assert(!strcmp(fileContents, "This is a test file"));

//     char newPath[] = "/documents/files";
//     uploadFile(newPath, root, fileData, fileName);

//     char newFilePath[] = "/documents/files/test_file.txt";
//     fileContents = openFile(newFilePath, root);
//     printf("%s\n", fileContents);
//     assert(!strcmp(fileContents, "This is a test file"));

//     return 0;
// }
int testUploadToRoot(Directory *root) {

    char path[] = "/desktop";
	makeDirectory("desktop", root);
    char *fileName = "test_file.txt";
    char *fileData = "This is a test file";
	
    uploadFile(path, root, fileData, fileName);
    char filePath[] = "/desktop/test_file.txt";
    char *fileContents = openFile(filePath, root);

    assert(!strcmp(fileContents, "This is a test file"));

    return 0;
}

void requestTypeString(char* requestString, int httpStatus) {
	switch(httpStatus) {
		case 200:
			sprintf(requestString, "OK");
			break;
		case 400:
			sprintf(requestString, "Bad Request");
			break;
		case 404:
			sprintf(requestString, "Not Found");
			break;
		default:
			sprintf(requestString, "Unknown");
	}
}

void createHTTPResponse(char *httpResponse, int httpStatus, char* message) {
	//Time stuff may be implemented later.

	// time_t t = time(NULL);
	// struct tm tm = *localtime(&t);
	// printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	char RTString[10];
	requestTypeString(RTString, httpStatus);
	sprintf(httpResponse, "HTTP/1.1 %d %s\nDate: Mon, 27 Jul 2009 12:28:53 GMT\nServer: Apache/2.2.14 (Win32)\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\nContent-Length: %d\nContent-Type: text/html\nConnection: Closed\n\n<html><body><p>%s</p></body><html>", httpStatus, RTString, strlen(message) + 29, message);
	puts(httpResponse);
}

int main(int argc, char *argv[])
{
	/*
	server_socket: ID of socket of web server
	client_sock: ID of socket of web client
	size_of_addr: size of socket address
	server: socket address of server
	client: socket address of client
	server_backlog: number of clients possible
	server_port: where server will listen
	*/
	int server_socket, client_sock, size_of_addr;
	struct sockaddr_in server, client;
	char client_message[3000];
	int server_backlog = 10;
	int server_port = atoi(argv[1]);
	Directory* root = initializeRoot();
	// testUploadToRoot(root);
	// printFileTree(root);

	//Create socket
	server_socket = socket(AF_INET , SOCK_STREAM , 0); //assigns ID to server_socket
	if (server_socket == -1) {
		printf("Could not create socket");
		exit(1);
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET; //ipv4 address
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( server_port ); //converts a port (like 3000) to network port
	
	//Bind server to the address and network port
	if(bind(server_socket, (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("bind failed. Error");
		return 1;
	}
	
	//Begin listening on given port with a max number of clients:
	listen(server_socket, server_backlog);

	printf("Listening on port: %d\n", server_port);

	char HTTPResponse[4092];
	
	char* error_message = "Couldn't get request type.";
	
	//Accept incoming connections
	puts("Waiting for incoming connections...");
	while(1) {
		size_of_addr = sizeof(struct sockaddr_in);
		//accept connection from an incoming client
		client_sock = accept(server_socket, (struct sockaddr *)&client, (socklen_t*)&size_of_addr);
		if (client_sock < 0)
		{
			perror("Client connection couldn't be accepted.");
			exit(1);
		}
		printf("Connection accepted, connection number: %d\n", client_sock);

		char buffer[4096];
		int msgsize = 0;
		size_t bytesread;

		// Begin receiving HTTP requests (either from the browser, Postman, or client)
		while((bytesread = read(client_sock, buffer+msgsize, sizeof(buffer)-msgsize-1)) > 0)
		{	
			msgsize += bytesread;
			if(msgsize > 4095) break;
			buffer[msgsize-1] = 0;
			// Send the message back to client
			if(msgsize == bytesread) printf("REQUEST IS:\n%s\n", buffer);

			//HTTP Parsing
			char* requestType = strtok(buffer, " "); // GET, POST, DELETE
			char* path;

			if(requestType != NULL) {
				if(root->childDirs != NULL){
					printf("WE IN HERE");
					printf("%s\n", root->childDirs->data->name);
					// if(root->childDirs->data->name == "desktop") puts("THERE IS A DESKTOP HOMIE");
					puts(root->childDirs->data->files->data);
				}
				path = strtok(NULL, " ");
				// printf("%d\n", strcmp(path, "/"));
				if(strcmp(requestType, "GET") == 0) { //get request
					puts(path);
					printf("%d\n", strlen(path));
					char* fileContents = openFile(path, root);
					puts(fileContents);
					if(fileContents == NULL) {
						createHTTPResponse(HTTPResponse, 404, "File could not be found. Maybe somewhere else?");
					}
					else {
						createHTTPResponse(HTTPResponse, 200, fileContents);
					}
					send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
				}
				else if(strcmp(requestType, "POST") == 0) { //post request

					char* fileQuery = strtok(path, "?");
					char* dataQuery = strtok(NULL, " ");
					char* fileData = replaceWord(strstr(dataQuery, "fileData=") + 9, "%20", " ");
					char* fileName = strtok(strstr(dataQuery, "fileName=") + 9, "&");
					printf("%s\n", fileQuery);			
					printf("%s\n", fileData);
					printf("%s\n", fileName);	
					printf("%d\n", strlen(fileQuery));
					char rootQuery[100];
					char fileNameCopy[strlen(fileName)]; 
					if(strlen(fileQuery) > 1) { //Are we adding to sub-directory?
						fileNameCopy[0] = '/';
						puts(fileQuery);
						if(getDirectoryFromPath(fileQuery, root) == NULL) {
							makeDirectory(fileQuery + 1, root);
						}
						printf("figidfgdfigid %s\n", root->childDirs->data->name);
					}
					strcpy(rootQuery, fileQuery);
					strcat(fileNameCopy, fileName);
					strcat(rootQuery, fileNameCopy);

					//uploadFile("/cooltext.txt", root, "..", cooltext.txt);
					uploadFile(fileQuery, root, fileData, fileName);
					// puts(openFile(rootQuery, root));
					// puts(fileData);
					// printf("%d\n", strcmp(openFile(rootQuery, root), fileData));

					char HTMLResponse[100];
					// puts(rootQuery);
					if(strcmp(openFile(rootQuery, root), fileData) == 0) {
						sprintf(HTMLResponse, "File %s created successfully at path %s", fileName, rootQuery);
						createHTTPResponse(HTTPResponse, 200, HTMLResponse);
						printf("==================\n");
						printf("%s\n", openFile(rootQuery, root));
					}
					else {
						sprintf(HTMLResponse, "File %s not created successfully. Something went wrong.", fileName);
						createHTTPResponse(HTTPResponse, 400, HTMLResponse);
					}
					printf("RESPONSE:\n%s\n", HTTPResponse);
					send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
				}
				else if(strcmp(requestType, "DELETE") == 0) { //delete request

				}
				else {
					// send(client_sock, hello, strlen(hello), 0);
				}
			}
			else {
				// write(client_sock, error_message, strlen(error_message));
			}
			// printf("REALPATH IS: %s\n", realpath(buffer, actualpath));
			// // if(realpath(buffer, actualpath) == NULL) {
			// // 	printf("ERROR: %s is an incorrect path.\n", buffer);
			// // 	close(client_sock);
			// // 	return 0;
			// // }
			// FILE *fp = fopen(actualpath, "r");
			// if(fp == NULL) {
			// 	printf("Error in opening file.");
			// 	close(client_sock);
			// 	return 0;
			// }

			// while((bytesread = fread(buffer, 1, 4096, fp)) > 0) {
			// 	write(client_sock, buffer, bytesread);
			// }
			// if(msgsize == bytesread) write(client_sock, hello, strlen(hello));
			// send(server_socket, message, strlen(message), 0);
			
			// fclose(fp);
		}
		close(client_sock);
		fflush(stdout);
		puts("Waiting for incoming connections...");


		// if(read_size == 0)
		// {
		// 	puts("Client disconnected");
		// 	fflush(stdout);
		// }
		// else if(read_size == -1)
		// {
		// 	perror("recv failed");
		// }
	}
	
	return 0;
}

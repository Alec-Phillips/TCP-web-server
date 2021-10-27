#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
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

void connection_handler(void* socket_desc);
void createDirectories(char* path);
void getRealPath(char buffer[], char actualpath[], char* requestType);
void getFileContents(char actualpath[], char fileContents[]);
void createHTTPResponse(char *httpResponse, int httpStatus, char* message);
void requestTypeString(char* requestString, int httpStatus);

/*
	HTTP Response Creator
*/



// Here is our global array (eventually hashmap, hopefully)
// holds the filenames and their corresponding semaphore

typedef struct FileSem {
	char *filePath;
	sem_t mutex;
} FileSem;

FileSem fileSemaphores[200];
int numFiles = 0;

FileSem* addFileSem(char *filePath) {
	FileSem newFileSem;
	newFileSem.filePath = (char *) malloc(strlen(filePath));
	strcpy(newFileSem.filePath, filePath);
	fileSemaphores[numFiles] = newFileSem;
	numFiles ++;
	return &newFileSem;
}

FileSem* getFileSem(char *target) {
	for (int i = 0; i < numFiles; i ++) {
		FileSem curr = fileSemaphores[i];
		if (! strcmp(curr.filePath, target)) {
			return &curr;
		}
	}
	return NULL;
}

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
	int server_socket, client_sock, size_of_addr, *new_sock;
	struct sockaddr_in server, client;
	int server_backlog = 10;
	int server_port = atoi(argv[1]);

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
		
		pthread_t sniffer_thread;
		new_sock = malloc(1); 
		*new_sock = client_sock;

		if(pthread_create(&sniffer_thread, NULL, connection_handler, (void*) new_sock) < 0) {
			perror("Couldn't make new thread.");
			exit(1);
		}

		pthread_join(sniffer_thread, NULL);
		
		// Begin receiving HTTP requests (either from the browser, Postman, or client)
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

void connection_handler(void* socket_desc) {
	char buffer[4096];
	char actualpath[PATH_MAX + 1];
	char HTTPResponse[4092];
	int client_sock = *(int*)socket_desc;
	int msgsize = 0;
	size_t bytesread;
	
	while((bytesread = read(client_sock, buffer+msgsize, sizeof(buffer)-msgsize-1)) > 0) {	
		msgsize += bytesread;
		if(msgsize > 4095) break;
		buffer[msgsize+1] = 0;
		// Send the message back to client
		if(msgsize == bytesread) printf("REQUEST IS:\n%s\n", buffer);
		char* requestType = strtok(buffer, " "); // GET, POST, DELETE
		char* path;
		if(requestType != NULL) {
			path = strtok(NULL, " ");
			char fileContents[10000];
			if(strcmp(requestType, "GET") == 0) { //get request
				char* rootPointer = "../root";
				char relativePath[strlen(rootPointer) + strlen(path)];
				strcpy(relativePath, rootPointer);
				strcat(relativePath, path);
				getRealPath(relativePath, actualpath, requestType); //realpath gets stored to actualpath
				FileSem* file_sem = getFileSem(actualpath);
				if(file_sem == NULL) {
					createHTTPResponse(HTTPResponse, 404, "File could not be found. Maybe somewhere else?");
					send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
				}
				else {
					sem_wait(&(file_sem->mutex));
					getFileContents(actualpath, fileContents);
					if(fileContents == NULL) {
						createHTTPResponse(HTTPResponse, 404, "File found, but file contents weren't found. May be a problem on our end.");
					}
					else {
						createHTTPResponse(HTTPResponse, 200, fileContents);
					}
					send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
					sem_post(&(file_sem->mutex));
				}
			}
			else if(strcmp(requestType, "PUT") == 0) { //PUT request
				char* requestBody = strtok(NULL, "\n");
				int contentLength = 0;
				int i = 0;
				while(i < 10) {
					if(strstr(requestBody, "Content-Length")) {
						char* contentLengthString = strstr(requestBody, " ") + 1;
						contentLength = atoi(contentLengthString);
					}
					requestBody = strtok(NULL, "\n");
					printf("%s\n", requestBody);
					i++;
				}
				
				printf("REQUEST BODY: %s\n", requestBody);
				printf("REQUEST TYPE %s\n", requestType);
				printf("CONTENT LENGTH: %d\n", contentLength);
				
				char* rootPointer = "../root";
				char relativePath[strlen(rootPointer) + strlen(path)];
				
				strcpy(relativePath, rootPointer);
				strcat(relativePath, path);

				
				puts(relativePath);
				createDirectories(relativePath);
				getRealPath(relativePath, actualpath, requestType);

				FileSem* fileSem;
				if(getFileSem(actualpath) == NULL) {
					printf("MAKING NEW FILE SEM AT PATH: %s\n", actualpath);
					fileSem = addFileSem(actualpath);
				}
				else {
					fileSem = getFileSem(actualpath);
				}

				sem_wait(&(fileSem->mutex));

				FILE *fp;
				puts(actualpath);
				fp = fopen(actualpath, "w");
				// puts(requestBody);
				char fileContents[contentLength + 2];
				memcpy(fileContents, requestBody, strlen(requestBody) + 1);
				fileContents[strlen(requestBody) + 2] = '\0';
				// fileContents[-1]
				// puts(fileContents);
				fputs(fileContents, fp);
				// while(strcmp(requestBody, "\n") != 0) {
				// 	puts(requestBody);
				
				// 	requestBody = strtok(NULL, "\n");
				// }
				// puts(fileContents);
				fclose(fp);
				sem_post(&(fileSem->mutex));

				
				char HTMLResponse[PATH_MAX + 1];
				// puts(rootQuery);
				// if(strcmp(openFile(rootQuery, root), fileData) == 0) {
				sprintf(HTMLResponse, "File created successfully at path %s", actualpath);
				// 	createHTTPResponse(HTTPResponse, 200, HTMLResponse);
				// 	printf("==================\n");
				// 	printf("%s\n", openFile(rootQuery, root));
				// }
				// else {
				// sprintf(HTMLResponse, "File %s not created successfully. Something went wrong.", fileName);
				// 	createHTTPResponse(HTTPResponse, 400, HTMLResponse);
				// }
				// printf("RESPONSE:\n%s\n", HTTPResponse);
				// send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
				send(client_sock, HTMLResponse, strlen(HTTPResponse), 0);
			}
			else if(strcmp(requestType, "DELETE") == 0) { //delete request

			}
			else {
				send(client_sock, "Error", 5, 0);
			}
		}
	}
}

void createDirectories(char* path) {
	char directoryString[strlen(path) + 10];
	puts("PATH:");
	puts(path);
	char* lastDot = strrchr(path, '/');
	puts(lastDot);
	strncpy(directoryString, path, strlen(path) - strlen(lastDot));
	directoryString[strlen(path) - strlen(lastDot) + 1] = '\0';
	char mkdirString[strlen(directoryString)]; 
	sprintf(mkdirString, "mkdir -p %s", directoryString);
	puts(mkdirString);
	system(mkdirString);
}

void getRealPath(char* relativePath, char* actualpath, char* requestType) {
	printf("Relative path is: %s\n", relativePath);
	if(realpath(relativePath, actualpath) == NULL && strcmp(requestType, "GET") == 0) {
		puts(actualpath);
		printf("ERROR: %s is an incorrect path.\n", relativePath);
		exit(1);
	}
	else {
		printf("Path is actually: %s\n", actualpath);
	}
} 

void getFileContents(char actualpath[], char fileContents[]) {
	// puts(actualpath);
	FILE *fp = fopen(actualpath, "r");
	if(fp == NULL) {
		puts("Error in opening file.\n");
		exit(1);
	}
	else {
		int c;
		int i = 0;
		while ((c = getc(fp)) != EOF){
			// putchar(c);
			fileContents[i] = c;
			i++;
		}
		fileContents[i] = "\0";
		fclose(fp);
	}
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


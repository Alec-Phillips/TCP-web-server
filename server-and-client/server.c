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
int getRealPath(char buffer[], char actualpath[], char* requestType); //realpath gets stored to actualpath
void getFileContents(char actualpath[], char fileContents[]);
void createHTTPResponse(char *httpResponse, int httpStatus, char* message);
void requestTypeString(char* requestString, int httpStatus);
char* replaceWord(const char* s, const char* oldW, const char* newW);


// Here is our global array (eventually hashmap, hopefully)
// holds the filenames and their corresponding semaphore

typedef struct FileSem {
	char *filePath;
	sem_t mutex;
	int numWaiting;
} FileSem;

FileSem fileSemaphores[200];
int numFiles = 0;


FileSem* addFileSem(char *filePath) {
	FileSem newFileSem;
	newFileSem.filePath = (char *) malloc(strlen(filePath));
	strcpy(newFileSem.filePath, filePath);
	sem_init(&(newFileSem.mutex), 0, 1);
	newFileSem.numWaiting = 0;
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

/*
	wraps the sem_wait function
	returns 0 if the wait is over and the file is available
	returns 1 if the file is not available
		(if the file was deleted by one of the prior clients)
*/
int semWait(FileSem* fileSem) {
	fileSem->numWaiting ++;
	sem_wait(&(fileSem->mutex));
	fileSem->numWaiting --;
	if (&(fileSemaphores[numFiles].filePath) == &(fileSem->filePath)) {
		return 1;
	}
	return 0;
}

int removeFileSem(char *target) {
	FileSem toRemove;
	int removedInd = -1;
	for (int i = 0; i < numFiles; i ++){
		FileSem curr = fileSemaphores[i];
		if (! strcmp(curr.filePath, target)) {
			toRemove = curr;
			removedInd = i;
			break;
		}
	}
	if (removedInd == -1) {
		return 1;
	}
	fileSemaphores[numFiles] = toRemove;
	free(toRemove.filePath);	// must free this bc it was dynamically allocated
	for (int i = removedInd; i < numFiles - 1; i ++) {
		fileSemaphores[i] = fileSemaphores[i + 1];
	}
	numFiles --;
	return 0;
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
	int server_backlog = 100;
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
		puts("left handler function");

		pthread_join(sniffer_thread, NULL);
		puts("thread deleted");
		
		close(client_sock);
		puts("client closed");
		fflush(stdout);
		puts("Waiting for incoming connections...");

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

	// stuff needed for timeout logic with client socket
	fd_set readfds, masterfds;
	struct timeval timeout;
	timeout.tv_sec = 2; // set the timeout to 2 seconds
  	timeout.tv_usec = 0;
	FD_ZERO(&masterfds);
   	FD_SET(client_sock, &masterfds);

	memcpy(&readfds, &masterfds, sizeof(fd_set));

	while((bytesread = read(client_sock, buffer+msgsize, sizeof(buffer)-msgsize-1)) > 0) {	
		msgsize += bytesread;
		if(msgsize > 4095) break;
		buffer[msgsize+1] = 0;
		puts("at top of while loop");
		// Send the message back to client
		if(msgsize == bytesread) printf("REQUEST IS:\n%s\n", buffer);
		
		char* requestType = strtok(buffer, " "); // GET, POST, DELETE
		char* path; // will store path requested
		if(requestType != NULL) {
			path = strtok(NULL, " ");
			char fileContents[10000];
			if(strcmp(requestType, "GET") == 0) { //GET Request
				char* rootPointer = "../root";
				char relativePath[strlen(rootPointer) + strlen(path)];
				strcpy(relativePath, rootPointer);
				strcat(relativePath, path);
				if(!getRealPath(relativePath, actualpath, requestType)) { //If path is incorrect
					createHTTPResponse(HTTPResponse, 404, "Incorrect path. Please try again with a different path.");
					send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
				} 
				else {

					FileSem* file_sem = getFileSem(actualpath);
					if(file_sem == NULL) { // If no file sem is found to associate with file at path
						createHTTPResponse(HTTPResponse, 404, "File could not be found. Maybe somewhere else?");
						send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
					}
					// sem_wait(&(file_sem->mutex));
					else if (semWait(file_sem) == 1) { 
						createHTTPResponse(HTTPResponse, 404, "THE REQUESTED FILE DOES NOT EXIST");
						send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
					}
					else {
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
			}
			else if(strcmp(requestType, "PUT") == 0) { //PUT request
				char* requestBody = strtok(NULL, "\n");
				int contentLength = 0;
				while(strcmp(requestBody, "\r") != 0 && requestBody != NULL) {
					// puts(requestBody);
					if(strstr(requestBody, "Content-Length")) {
						char* contentLengthString = strstr(requestBody, " ") + 1;
						contentLength = atoi(contentLengthString);
					}
					// if(strstr(requestBody, "Connection")) {
					// 	char* contentLengthString = strstr(requestBody, " ") + 1;
					// }
					requestBody = strtok(NULL, "\n");
				}
				requestBody = strtok(NULL, "\n");
				if(requestBody == NULL) requestBody = ""; //allowance for empty files if request body is empty
				
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
				if((fileSem = getFileSem(actualpath)) == NULL) { //if the filesem doesn't exist yet, we make a new one!
					printf("MAKING NEW FILE SEM AT PATH: %s\n", actualpath);
					fileSem = addFileSem(actualpath);
				}

				sem_wait(&(fileSem->mutex));
				FILE *fp;
				puts(actualpath);
				fp = fopen(actualpath, "w");
				char fileContents[contentLength + 2];
				printf("Content Length: %d\n", contentLength);
				puts(fileContents);
				puts(requestBody);
				memcpy(fileContents, requestBody, strlen(requestBody) + 1);
				fileContents[strlen(requestBody) + 2] = '\0';
				fputs(fileContents, fp);
				fclose(fp);
				sem_post(&(fileSem->mutex));				
				char HTMLResponse[PATH_MAX + 1];
				sprintf(HTMLResponse, "File created successfully at path %s", actualpath);
				puts("FILE CONTENTS: ");
				getFileContents(actualpath, fileContents);
				createHTTPResponse(HTTPResponse, 200, HTMLResponse);
				puts("about to send response to client");
				send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
				puts("sent response to client");
			}
			else if(strcmp(requestType, "DELETE") == 0) { //delete request
				char* rootPointer = "../root";
				char relativePath[strlen(rootPointer) + strlen(path)];
				strcpy(relativePath, rootPointer);
				strcat(relativePath, path);
				getRealPath(relativePath, actualpath, requestType);
				// get the path to the file to delete
				// check that the file actaully exists
				FileSem* fileSem = getFileSem(actualpath);
				if(fileSem == NULL) {
					createHTTPResponse(HTTPResponse, 404, "The requested file doesn't even exist.");
					send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);	
				}
				else if (semWait(fileSem) == 1) {
					createHTTPResponse(HTTPResponse, 404, "Sorry, the requested file was deleted by a previous client.");
					send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);	
				}
				else {
					remove(actualpath);
					removeFileSem(actualpath);
					FileSem removed = fileSemaphores[numFiles];
					// call sem_post for each waiting process
					// they will each then return the 404 file to their client
					while (removed.numWaiting > 0) {
						sem_post(&(removed.mutex));
					}
					// destroy the semaphore
					sem_destroy(&(removed.mutex));
					char HTMLResponse[PATH_MAX + 1];
					sprintf(HTMLResponse, "File at path %s deleted successfully", actualpath);
					createHTTPResponse(HTTPResponse, 200, HTMLResponse);
					send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
					puts("sent response to client");
				}
					
			}
			else {
				createHTTPResponse(HTTPResponse, 501, "That request type isn't implemented. The available request types are GET, PUT, and DELETE.");
				send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
			}
		}
		else {
			createHTTPResponse(HTTPResponse, 400, "Invalid request type. Please try again.");
			send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
		}
		puts("at end of while loop");
		// timeout after 2 seconds if nothing received from client
		if (select(client_sock+1, &readfds, NULL, NULL, &timeout) < 0) {
				perror("on select");
				exit(1);
		}
		if (FD_ISSET(client_sock, &readfds)) {
			// read from the socket
			continue;
		}
		else {
			// the socket timedout
			break;
		}
	}
	puts("at end of the function");
	free(socket_desc);	
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

int getRealPath(char* relativePath, char* actualpath, char* requestType) {
	printf("Relative path is: %s\n", relativePath);
	if(realpath(relativePath, actualpath) == NULL && (strcmp(requestType, "GET") == 0 || strcmp(requestType, "DELETE") == 0)) {
		printf("ERROR: %s is an incorrect path.\n", relativePath);
		return 0;
	}
	else {
		printf("Path is actually: %s\n", actualpath);
		return 1;
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
			fileContents[i] = c;
			i++;
		}
		fileContents[i] = 0;
		puts(fileContents);
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
		case 501:
			sprintf(requestString, "Internal Server Error");
			break;
		default:
			sprintf(requestString, "Unknown");
			break;
	}
}

void createHTTPResponse(char *httpResponse, int httpStatus, char* message) {
	//Time stuff may be implemented later.

	// time_t t = time(NULL);
	// struct tm tm = *localtime(&t);
	// printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	char RTString[10];
	requestTypeString(RTString, httpStatus);
	sprintf(httpResponse, "HTTP/1.1 %d %s\nDate: Mon, 27 Jul 2009 12:28:53 GMT\nServer: Apache/2.2.14 (Win32)\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\nContent-Length: %d\nContent-Type: text/plain\nConnection: Closed\n\n%s", httpStatus, RTString, strlen(message), message);
	puts(httpResponse);
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

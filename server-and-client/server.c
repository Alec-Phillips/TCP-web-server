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
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include "zlib.h"

#include "../hashmap/HashMap.h"

/* 
	Note:
	- You need to change the two includes above to match where the files are for you (absolute path I believe)

	Compilation command:
	gcc server.c ../file_system/FileSystem.c ../file_system/UsefulStructures.c -lz -Wall -o se
	gcc server.c ../hashmap/HashMap.c -lz -Wall -o se

	Run with ./se
*/


/*
	C socket server example
	https://www.binarytides.com/server-client-example-c-sockets-linux/
*/

int reconcile_fragmented_request(char* buffer, char* request_start);
void connection_handler(void* socket_desc);
void createDirectories(char* path);
int getRealPath(char buffer[], char actualpath[], char* requestType); //realpath gets stored to actualpath
void getFileContents(char actualpath[], char fileContents[]);
void createHTTPResponse(char *httpResponse, int httpStatus, char* message, bool is_persistent_connection, char* compression_alg, char* contentType);
void requestTypeString(char* requestString, int httpStatus);
char* replaceWord(const char* s, const char* oldW, const char* newW);


// init the content type mapping
HashMap* fileExtMap;

void initFileExtMap(HashMap** map) {
	*map = initMap(8 * sizeof(char));
	char* ext1 = "text/plain";
    char* ext2 = "text/html";
	char* ext3 = "application/json";
	char* ext4 = "text/css";    
	char* ext5 = "text/csv";
	char* ext6 = "application/javascript";
	char* ext7 = "application/xml";
    char* val1 = ".txt";
    char* val2 = ".html";
	char* val3 = ".json";
	char* val4 = ".css";
	char* val5 = ".csv";
	char* val6 = ".js";
	char* val7 = ".xml";
    put(*map, ext1, val1, 1);
    put(*map, ext2, val2, 1);
	put(*map, ext3, val3, 1);
	put(*map, ext4, val4, 1);
    put(*map, ext5, val5, 1);
	put(*map, ext6, val6, 1);
	put(*map, ext7, val7, 1);
	// char* test = get(*map, ext1);
	// puts(test);
	// test = get(*map, ext2);
	// puts(test);
	// test = get(*map, ext4);
	// puts(test);
}


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

	initFileExtMap(&fileExtMap);

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
	char buffer[15000]; 
	char actualpath[PATH_MAX + 1];
	char HTTPResponse[15000];
	int client_sock = *(int*)socket_desc;
	size_t bytesread;
	int num_transactions = 0;
	char* request_start;
	int spillover_offset = 0;

	// Keep-Alive header vals. Source: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Keep-Alive
	int max_transactions = 1000;
	struct timeval timeout;
	timeout.tv_sec = 5; // set the timeout to 5 seconds
  	timeout.tv_usec = 0;

	// set up timeout for client socket
	fd_set readfds, masterfds;
	FD_ZERO(&masterfds);
   	FD_SET(client_sock, &masterfds);

	memcpy(&readfds, &masterfds, sizeof(fd_set));

	// read, at max, the size of buffer many bytes from the socket buffer into the userspace buffer
	// combine first request we're gonna parse with the leftover request of last read() cycle (is there's something to work with)
	
	while((bytesread = read(client_sock, buffer+spillover_offset, sizeof(buffer)-1-spillover_offset)) > 0) {	
		puts("at top of while loop");
		buffer[bytesread+1] = 0;
		char *main_save_ptr = buffer;
		// parse through all requests placed into buffer. Start at buffer and end when main_save_ptr is at buffer+bytesread
		while (main_save_ptr != buffer + bytesread) {
			request_start = main_save_ptr;
			printf("entering nested while loop\n");
			printf("first char is %c\n", *(main_save_ptr));
			// Send the message back to client
			printf("REQUEST IS:\n%s\n", main_save_ptr);
			// extract header values from request
			
			char* requestType = strtok_r(NULL, " ", &main_save_ptr); // GET, POST, DELETE
			char* path = strtok_r(NULL, " ", &main_save_ptr); // stores the path requested

			if (path == NULL) {spillover_offset = reconcile_fragmented_request(buffer, request_start); break;}
			char* http_version = strtok_r(NULL, "\n", &main_save_ptr);
			if (http_version == NULL) {spillover_offset = reconcile_fragmented_request(buffer, request_start); break;}
			printf("buffer is: \n%s\n", main_save_ptr);
			printf("request type is %s\n", requestType);
			printf("path is %s\n", path);
			printf("version is %s\n", http_version);
			int contentLength = 0;
			bool is_persistent_connection = false;
			char* compression_alg = NULL;
			char* contentType;

			// strtok_r can only use 1 char as a delimiter, so we have to use pointer 
			// addition parse the request properly
			char* curr_header = strtok_r(NULL, "\n", &main_save_ptr);
			if (curr_header == NULL) {spillover_offset = reconcile_fragmented_request(buffer, request_start); break;}
			char* header_key;
			char* header_val;
			bool incomplete_header_section = false;
			 // curr_header marks the \r\n in between the header and body. Use Content-Length (if it exists) to go to end of body
			while(strlen(curr_header) > 1) {
				printf("curr header is %d\n", strlen(curr_header));
				// still headers left to parse! 
				char *tmp_save_ptr;
				header_key = strtok_r(curr_header, ":", &tmp_save_ptr);
				header_val = strtok_r(NULL, "\r", &tmp_save_ptr);
				if (header_val == NULL) {spillover_offset = reconcile_fragmented_request(buffer, request_start); incomplete_header_section = true; break;}
				printf("key is %s, ", header_key);
				printf("val is %s\n", header_val+1);
				if (strcmp(header_key, "Content-Length") == 0) contentLength = atoi(header_val+1); // need + 1 since there's a " " after colon
				else if (strcmp(header_key, "Connection") == 0) {
					if ((strcmp(header_val+1, "keep-alive") == 0)) is_persistent_connection = true;
				}
				else if (strcmp(header_key, "Accept-Encoding") == 0) compression_alg = header_val+1; // need + 1 since there's a " " after colon
				else if (strcmp(header_key, "Content-Type") == 0) contentType = header_val+1; // need + 1 since there's a " " after colon
				curr_header = strtok_r(NULL, "\n", &main_save_ptr);
			}
			// finish fragmented buffer checks
			if (incomplete_header_section == true) break;
			if ((request_start + contentLength) > (buffer + sizeof(buffer)-1)) {spillover_offset = reconcile_fragmented_request(buffer, request_start); break;}

			printf("main save ptr is %d, bytesread is %d, max index is %d\n", main_save_ptr, buffer+bytesread, (int) (buffer+sizeof(buffer)));
			printf("first char is %c\n", *main_save_ptr);
			printf("content length is %d\n", contentLength);
			printf("content type is %s\n", contentType);
			// seems to be able to copy an empty string into a size 0 char array successfully for GET requests?
			char* requestBody[contentLength];
			strncpy(requestBody, main_save_ptr, contentLength);
			printf("request body is %s\n", requestBody);

			main_save_ptr += contentLength;

			if(requestType != NULL) {
				// path = strtok(NULL, " ");
				char fileContents[10000];
				if(strcmp(requestType, "GET") == 0) { //GET Request
					char* rootPointer = "../root";
					char relativePath[strlen(rootPointer) + strlen(path)];
					strcpy(relativePath, rootPointer);
					strcat(relativePath, path);
					if(!getRealPath(relativePath, actualpath, requestType)) { //If path is incorrect
						createHTTPResponse(HTTPResponse, 404, "Incorrect path. Please try again with a different path.", is_persistent_connection, NULL, "text/plain");
						send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
					} 
					else {

						*(contentType + strlen(contentType)) = '\0';
						puts(contentType);
						char* fileExt = get(fileExtMap, contentType);
						char actualPathWExt[strlen(actualpath) + strlen(fileExt)];
						strcpy(actualPathWExt, actualpath);
						strcat(actualPathWExt, fileExt);
						puts("\n\n\n\n");
						puts(actualPathWExt);

						if(! access(actualPathWExt, F_OK) == 0) {
							createHTTPResponse(HTTPResponse, 404, "Incorrect path. Please try again with a different path.", is_persistent_connection, NULL, "text/plain");
							send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
						}

						FileSem* file_sem = getFileSem(actualPathWExt);
						if(file_sem == NULL) { // If no file sem is found to associate with file at path
							createHTTPResponse(HTTPResponse, 404, "File could not be found. Maybe somewhere else?", is_persistent_connection, NULL, "text/plain");
							send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
						}
						// sem_wait(&(file_sem->mutex));
						else if (semWait(file_sem) == 1) { 
							createHTTPResponse(HTTPResponse, 404, "THE REQUESTED FILE DOES NOT EXIST", is_persistent_connection, NULL, "text/plain");
							send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
						}
						else {
							getFileContents(actualPathWExt, fileContents);
							if(fileContents == NULL) { //If the file is found, but there are no file contents.
								createHTTPResponse(HTTPResponse, 404, "File found, but file contents weren't found. May be a problem on our end.", is_persistent_connection, NULL, "text/plain");
							}
							else {
								if (compression_alg != NULL) {
									printf("starting\n");
									int srcLen = strlen(fileContents);
									int destLen = compressBound(srcLen); // this is how you should estimate size needed for the buffer
									printf("lengths calculated\n");
																
									char* compressedFileContents = malloc(destLen);
									int res = compress(compressedFileContents, &destLen, fileContents, srcLen);
									printf("compression done\n"); 
									// destLen is now the size of actual buffer needed for compression
									// you don't want to uncompress whole buffer later, just the used part
									if(res == Z_BUF_ERROR){
										printf("Buffer was too small!\n");
									}
									else if(res ==  Z_MEM_ERROR){
										printf("Not enough memory for compression!\n");
									}
									createHTTPResponse(HTTPResponse, 200, compressedFileContents, is_persistent_connection, compression_alg, contentType);
									printf("response is %s\n", HTTPResponse);
								}
								else {printf("creating response\n"); createHTTPResponse(HTTPResponse, 200, fileContents, is_persistent_connection, compression_alg, contentType);}
							}
							send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
							sem_post(&(file_sem->mutex));
						}
					}
				}
				else if(strcmp(requestType, "PUT") == 0) { //PUT request
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

					*(contentType + strlen(contentType)) = '\0';
 					puts(contentType);
 					char* fileExt = get(fileExtMap, contentType);
 					char actualPathWExt[strlen(actualpath) + strlen(fileExt)];
 					strcpy(actualPathWExt, actualpath);
 					strcat(actualPathWExt, fileExt);
 					puts("\n\n\n\n");
 					puts(actualPathWExt);

					FileSem* fileSem;
					if((fileSem = getFileSem(actualPathWExt)) == NULL) { //if the filesem doesn't exist yet, we make a new one!
						printf("MAKING NEW FILE SEM AT PATH: %s\n", actualPathWExt);
						fileSem = addFileSem(actualPathWExt);
					}

					sem_wait(&(fileSem->mutex));
					FILE *fp;
					puts(actualPathWExt);
					fp = fopen(actualPathWExt, "w");
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
					sprintf(HTMLResponse, "File created successfully at path %s", actualPathWExt);
					createHTTPResponse(HTTPResponse, 200, HTMLResponse, is_persistent_connection, NULL, contentType);
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

					*(contentType + strlen(contentType)) = '\0';
					puts(contentType);
					char* fileExt = get(fileExtMap, contentType);
					char actualPathWExt[strlen(actualpath) + strlen(fileExt)];
					strcpy(actualPathWExt, actualpath);
					strcat(actualPathWExt, fileExt);
					puts("\n\n\n\n");
					puts(actualPathWExt);

					if(! access(actualPathWExt, F_OK) == 0) {
						createHTTPResponse(HTTPResponse, 404, "Incorrect path. Please try again with a different path.", is_persistent_connection, NULL, "text/plain");
						send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
					}

					// get the path to the file to delete
					// check that the file actaully exists
					FileSem* fileSem = getFileSem(actualPathWExt);
					if(fileSem == NULL) {
						createHTTPResponse(HTTPResponse, 404, "The requested file doesn't even exist.", is_persistent_connection, NULL, "text/plain");
						send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);	
					}
					else if (semWait(fileSem) == 1) {
						createHTTPResponse(HTTPResponse, 404, "Sorry, the requested file was deleted by a previous client.", is_persistent_connection, NULL, "text/plain");
						send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);	
					}
					else {
						remove(actualPathWExt);
						removeFileSem(actualPathWExt);
						FileSem removed = fileSemaphores[numFiles];
						// call sem_post for each waiting process
						// they will each then return the 404 file to their client
						while (removed.numWaiting > 0) {
							sem_post(&(removed.mutex));
						}
						// destroy the semaphore
						sem_destroy(&(removed.mutex));
						char HTMLResponse[PATH_MAX + 1];
						sprintf(HTMLResponse, "File at path %s deleted successfully", actualPathWExt);
						createHTTPResponse(HTTPResponse, 200, HTMLResponse, is_persistent_connection, NULL, contentType);
						send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
						puts("sent response to client");
					}
						
				}
				else {
					createHTTPResponse(HTTPResponse, 501, "That request type isn't implemented. The available request types are GET, PUT, and DELETE.", is_persistent_connection, NULL, "text/plain");
					send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
				}
			}
			else {
				createHTTPResponse(HTTPResponse, 400, "Invalid request type. Please try again.", is_persistent_connection, NULL, "text/plain");
				send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
			}
			// increment [num_transactions] by 1. If equal to max allowed, break out of while loop
			num_transactions++;
			printf("at end of smallest while loop. Transaction count: %d/%d\n", num_transactions, max_transactions);
			if (num_transactions == max_transactions) {
					free(socket_desc); 
					return;
				}
			// if closed connection, exit out of thread handler function
			if (is_persistent_connection == false) {
				free(socket_desc);	
				return;
			}
		}
		puts("at end of while loop");
		// if persistent connection, time out after [timeout] seconds if nothing received from client
		if (select(client_sock+1, &readfds, NULL, NULL, &timeout) < 0) {
			perror("on select");
			exit(1);
		}
		// something still in socket buffer
		if (FD_ISSET(client_sock, &readfds)) continue;
		else {
			// the socket timedout
			free(socket_desc); 
			return;
		}
	}
	puts("at end of the function");
	free(socket_desc);	
}
	
int reconcile_fragmented_request(char* buffer, char* request_start) {
	// fill start of buffer with chars from request_start until the end of the buffer. Then return the spillover offset for the buffer
	int last_buffer_index = buffer + sizeof(buffer)-1;
	int spillover_offset = 0;
	while ((int) request_start < last_buffer_index) {
		*(buffer+spillover_offset) = *request_start;
		request_start++;
	}
	return spillover_offset;
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
	// if(realpath(relativePath, actualpath) == NULL && (strcmp(requestType, "GET") == 0 || strcmp(requestType, "DELETE") == 0)) {
	// 	printf("ERROR: %s is an incorrect path.\n", relativePath);
	// 	return 0;
	// }
	if(realpath(relativePath, actualpath) == NULL && 0) {
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

void createHTTPResponse(char *httpResponse, int httpStatus, char* message, bool is_persistent_connection, char* compression_alg, char* contentType) {
	//Time stuff may be implemented later.

	// time_t t = time(NULL);
	// struct tm tm = *localtime(&t);
	// printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	printf("at start of response function\n");
	char RTString[10];
	requestTypeString(RTString, httpStatus);
	printf("entering if statements\n");
	if (is_persistent_connection == false) {
		printf("no persistent connection\n");
		if (compression_alg == NULL) sprintf(httpResponse, "HTTP/1.1 %d %s\nDate: Mon, 27 Jul 2009 12:28:53 GMT\nServer: Apache/2.2.14 (Win32)\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\nContent-Length: %d\nContent-Type: %s\nConnection: closed\n\n%s", httpStatus, RTString, strlen(message), contentType, message);
		else {
			printf("doing sprintf\n");
			sprintf(httpResponse, "HTTP/1.1 %d %s\nDate: Mon, 27 Jul 2009 12:28:53 GMT\nServer: Apache/2.2.14 (Win32)\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\nContent-Length: %d\nContent-Type: %s\nContent-Encoding: %s\nConnection: closed\n\n%s", httpStatus, RTString, strlen(message), contentType, compression_alg, message);
		}
	}
	else {
		printf("yes persistent connection\n");
		if (compression_alg == NULL) sprintf(httpResponse, "HTTP/1.1 %d %s\nDate: Mon, 27 Jul 2009 12:28:53 GMT\nServer: Apache/2.2.14 (Win32)\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\nContent-Length: %d\nContent-Type: %s\nConnection: keep-alive\nKeep-Alive:timeout=5, max=1000\n\n%s", httpStatus, RTString, strlen(message), contentType, message);
		else {
			printf("doing sprintf\n");
			sprintf(httpResponse, "HTTP/1.1 %d %s\nDate: Mon, 27 Jul 2009 12:28:53 GMT\nServer: Apache/2.2.14 (Win32)\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\nContent-Length: %d\nContent-Type: %s\nContent-Encoding: %s\nConnection: keep-alive\nKeep-Alive:timeout=5, max=1000\n\n%s", httpStatus, RTString, strlen(message), contentType, compression_alg, message);
		}
	}
	// puts(httpResponse);
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

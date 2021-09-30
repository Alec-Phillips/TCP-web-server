#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>


/*
	C socket server example
	https://www.binarytides.com/server-client-example-c-sockets-linux/
*/

/*
	HTTP Response Creator
*/

void requestTypeString(char* requestString, int httpStatus) {
	switch(httpStatus) {
		case 200:
			sprintf(requestString, "OK");
			break;
		case 400:
			sprintf(requestString, "Bad Request");
		case 404:
			sprintf(requestString, "Not Found");
	}
}

void createHTMLResponse(char *httpResponse, int httpStatus, char* message, int messageLength) {
	//Time stuff may be implemented later.

	// time_t t = time(NULL);
	// struct tm tm = *localtime(&t);
	// printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	char RTString[10];
	requestTypeString(RTString, httpStatus);
	sprintf(httpResponse, "HTTP/1.1 %d %s\nDate: Mon, 27 Jul 2009 12:28:53 GMT\nServer: Apache/2.2.14 (Win32)\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\nContent-Length: %d\nContent-Type: text/html\nConnection: Closed\n\n<html><body><p>%s</p></body><html>", httpStatus, RTString, messageLength + 29, message);
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
	
	// Directory* root = initalizeRoot();
	
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

	// char* hello = "HTTP/1.1 200 OK\nDate: Mon, 27 Jul 2009 12:28:53 GMT\nServer: Apache/2.2.14 (Win32)\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\nContent-Length:88\nContent-Type: text/html\nConnection: Closed\n\n<html><body><h1>Hello, how are you?</h1></body><html>";
	// char* message = "GET / HTTP/1.1\r\n\r\n";

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
				path = strtok(NULL, " ");
				// printf("%d\n", strcmp(path, "/"));
				if(strcmp(requestType, "GET") == 0) { //get request
					//fopen(path, root?)
					createHTMLResponse(HTTPResponse, 200, "Hello Web Server", 18);
					send(client_sock, HTTPResponse, strlen(HTTPResponse), 0);
					// close(client_sock);
				}
				else if(strcmp(requestType, "POST") == 0) { //post request

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

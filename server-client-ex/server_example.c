/*
	C socket server example
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>	//strlen
#include <limits.h> 
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write




int main(int argc , char *argv[])
{
	int server_socket , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[3000];
	int server_backlog = 10;
	int server_port = 10;
	
	//Create socket
	server_socket = socket(AF_INET , SOCK_STREAM , 0);
	if (server_socket == -1)
	{
		printf("Could not create socket");
		exit(1);
	}
	puts("Socket created");
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( server_port );
	
	//Bind
	if( bind(server_socket, (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");
	
	//Listen
	listen(server_socket , server_backlog);

	printf("Listening on port: %d\n", server_port);

	char* hello = "HTTP/1.1 200 OK\nDate: Mon, 27 Jul 2009 12:28:53 GMT\nServer: Apache/2.2.14 (Win32)\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\nContent-Length:88\nContent-Type: text/html\nConnection: Closed\n\n<html><body><h1>Hello, how are you?</h1></body><html>";
	// write(client_sock , hello , strlen(hello));

	char * message = "GET / HTTP/1.1\r\n\r\n";
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	//accept connection from an incoming client
	client_sock = accept(server_socket, (struct sockaddr *)&client, (socklen_t*)&c);
	if (client_sock < 0)
	{
		perror("accept failed");
		exit(1);
	}
	printf("Connection accepted, connection number: %d\n", client_sock);
	char buffer[4096];
	char actualpath[PATH_MAX+1];
	int msgsize = 0;
	size_t bytesread;
	//Receive a message (filepath) from client
	while((bytesread = read(client_sock, buffer+msgsize, sizeof(buffer)-msgsize-1)) > 0)
	{	
		msgsize += bytesread;
		if(msgsize > 4095) break;
		buffer[msgsize-1] = 0;
		// Send the message back to client
		if(msgsize == bytesread) printf("REQUEST IS: %s\n", buffer);
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
		if(msgsize == bytesread) write(client_sock, hello, strlen(hello));
		// send(server_socket, message, strlen(message), 0);
		
		// fclose(fp);
	}

	close(client_sock);

	// if(read_size == 0)
	// {
	// 	puts("Client disconnected");
	// 	fflush(stdout);
	// }
	// else if(read_size == -1)
	// {
	// 	perror("recv failed");
	// }
	
	return 0;
}
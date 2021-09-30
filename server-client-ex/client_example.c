/*
	C ECHO client example using sockets
*/
#include <stdio.h>	//printf
#include <stdlib.h>
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>

int main(int argc , char *argv[])
{
	int sock;
	struct sockaddr_in server;
	int port = atoi(argv[1]);
	char message[1000], server_reply[2000];
	
	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");
	
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	//Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}
	
	puts("Connected\n");
	
	//keep communicating with server
	// char cmdPath[4092];
	// getcwd(cmdPath, 3000);
	// printf("%s\n", cmdPath);

	char request[4092];

	while(1)
	{
		puts("Send an HTTP Request\n");
		// printf("The current directory you are looking in:\n %s", cmdPath);
		fgets(request, 4092, stdin);
		
		printf("Request is: %s\n", request);
		//Send some data
		if(send(sock, request , strlen(request) , 0) < 0)
		{
			puts("Send failed");
			return 1;
		}
		
		//Receive a reply from the server
		if(recv(sock, server_reply , 2000 , 0) < 0)
		{
			puts("recv failed");
			break;
		}
		
		puts("Server reply: ");
		puts(server_reply);
		fflush(stdout);
	}
	
	close(sock);
	return 0;
}
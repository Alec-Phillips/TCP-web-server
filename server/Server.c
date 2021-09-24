// Originally created by Eric Meehan

#include "Server.h"
#include <stdio.h>
#include <stdlib.h>

struct Server server_constructor(int domain, int service, int protocol, 
    u_long interface, int port, int backlog, void(*launch)(struct Server *server)) {

    struct Server server;
    server.domain = domain; 
    server.service = service;
    server.protocol = protocol;
    server.interface = interface;
    server.port = port;
    server.backlog = backlog;
    server.launch = launch;

    server.address.sin_family = domain;
    server.address.sin_port = htons(port); //converts short integer from host byte to network byte order?
    server.address.sin_addr.s_addr = htonl(interface); 

    server.socket = socket(domain, service, protocol);
    if(server.socket == 0) {
        perror("Failed to connect to socket... exiting.\n");
        exit(1);
    }

    if(bind(server.socket, (struct sockaddr *) &server.address, sizeof(server.address)) < 0) {
        perror("Failed to bind socket... exiting.\n");
        exit(1);
    }

    if(listen(server.socket, server.backlog) < 0) {
        perror("Failed to listen... exiting.\n");
        exit(1);
    };



    

    return server;
}
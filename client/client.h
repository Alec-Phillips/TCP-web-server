
// https://github.com/ericomeehan/libeom/blob/main/Networking/Nodes/Client.h

#ifndef Client_h
#define Client_h

#include <sys/socket.h>
#include <netinet/in.h>
// description of 'netinet/in.h' : https://pubs.opengroup.org/onlinepubs/009695399/basedefs/netinet/in.h.html

struct Client {

    // this will hold the socket that will get initialized w/in the client constructor
    // it will be initialized based on the provided domain, service, and protocol
    int socket;

    // these will be provided by the user during construction (the params)
    int domain;
    int service;
    int protocol;
    int port;
    u_long interface;

    char * (*request)(struct Client *client, char *server_ip, void *request, unsigned long size);
};


struct Client client_constructor(int domain, int service, int protocol, int port, u_long interface);

#endif
#include "socketutil.h"

int createTCPIpv4Socket(){return socket(AF_INET, SOCK_STREAM, 0);}

struct sockaddr_in* createIPv4Address(char *ip, int port){
    // create IPv4 socket address
    const struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    // set properties of address
    address->sin_family = AF_INET;
    // port # of the server that is listening HTTP connection
    address->sin_port = htons(port); 

    if(strlen(ip) == 0) //condition if ip pass empty
        address -> sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET, ip, )
    // convert to IP address from text to binary int form 
    inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    return address;
}
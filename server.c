#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "socketutil.h"

int main(){
    int serverSocketFD = createTCPIpv4Socket();
    // UDP port 2000 uses the Datagram Protocol, a communications protocol for the Internet network layer, transport layer, and session layer
    struct sockaddr_in *serverAddress = createIPv4Address("", 2000);

    //To receive the file descriptor of the server, and ask for server addres
    int result = bind(serverSocketFD, serverAddress, sizeof(*serverAddress));
    if(result == 0){
        printf("socket was bound successfully\n");
    }
    //To listen for incoming socket
    int listenResult = listen(serverSocketFD, 10);

    //To return the file descriptor of the connecting client, identical to client side
    struct sockaddr_in clientAddress;
    //in order to pass the pointer of the address size
    int clientAddresssize = sizeof(struct sockaddr_in);
    //clientSocketFD should be identical to client side socketFD
    int clientSocketFD = accept(serverSocketFD, &clientAddress, &clientAddresssize);

    char buffer[1024];
    while(true) {
        //ssize_t is the same as size_t , but is a signed type. 
        //ssize_t is able to represent the number -1 , 
        //which is returned by several system calls and library functions as a way to indicate error.
        ssize_t amountReceived = recv(clientSocketFD, buffer, 1024, 0);
        if (amountReceived>0){
            //clean the old message
            buffer[amountReceived] = 0;
            printf("Respond was %s\n ", buffer);
        }
        
        if (amountReceived == 0){
            break;
        }
    }

    close(clientSocketFD);
    //SHUT_RDWR, further receptions and transmissions will be disallowed.
    shutdown(serverSocketFD, SHUT_RDWR)

    return 0;
}
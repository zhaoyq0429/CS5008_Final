#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "socketutil.h"

int main(){
    int serverSocketFD = createTCPIpv4Socket();
    struct sockaddr_in *serverAddress = createIPv4Address("", 2000);

    //To receive the file descriptor of the server, and ask for server addres
    int result = bind(serverSocketFD, serverAddress, sizeof(*serverAddress));
    if(result == 0){
        printf("socket was bound successfully\n");
    }
    //To listen for incomeing socket
    int listenResult = listen(serverSocketFD, 10);

    //To return the file descriptor of the connecting client
    struct sockaddr_in clientAddress;
    int clientAddresssize = sizeof(struct sockaddr_in);
    int clientSocketFD = accept(serverSocketFD, &clientAddress, &clientAddresssize);



    return 0;
}

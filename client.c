#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>

// To extract mathod and make application more readable
struct sockaddr_in* createIPv4Address(char *ip, int port);
int createTCPIpv4Socket();
struct sockaddr_in* createIPv4Address(char *ip, int port){
    // create IPv4 socket address
    const struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    // set properties of address
    address->sin_family = AF_INET;
    // port # of the server that is listening HTTP connection
    address->sin_port = htons(port); 
    // convert to IP address from text to binary int form 
    inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    return address;
}

int main() {
    // create the socket file descriptor 
    // AF_INET: address family for IP version 4
    // SOCK_STREAM: TCP socket
    // 0 protocal/layers, i.e., using IP layer 
    int socketFD  = createTCPIpv4Socket();
    
    struct sockaddr_in *address = createIPv4Address("142.250.188.46", 80);

    int result = connect(socketFD, address, sizeof(*address));

    if (result == 0) {
        printf("Connection was successful\n");
    }

    char* message;
    // send to Google server using HTTP protocal 
    message = "GET \\ HTTP/1.1\r\nHOST:google.com\r\n\r\n";
    send(socketFD, message, strlen(message), 0);
    
    // To receive response
    char buffer[1024];
    recv(socketFD, buffer, 1024, 0);
    printf("Response was %s\n", buffer);

    
    return 0;
}

int createTCPIpv4Socket(){return socket(AF_INET, SOCK_STREAM, 0);}

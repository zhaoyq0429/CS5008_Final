#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main() {
    // create the socket file descriptor 
    // AF_INET: address family for IP version 4
    // SOCK_STREAM: TCP socket
    // 0 protocal/layers, i.e., using IP layer 
    int socketFD  = socket(AF_INET, SOCK_STREAM, 0);
    // using google's ip as temporary server/listener 
    char* ip = "142.250.188.46";
    // create IPv4 socket address
    const struct sockaddr_in address;
    // set properties of address
    address.sin_family = AF_INET;
    // port # of the server that is listening HTTP connection
    address.sin_port = htons(80); 
    // convert to IP address from text to binary int form 
    inet_pton(AF_INET, ip, &address.sin_addr.s_addr);

    int result = connect(socketFD, &address, sizeof(address));

    if (result == 0) {
        printf("Connection was successful\n");
    }

    char* message;
    // send to Google server using HTTP protocal 
    message = "GET \\ HTTP/1.1\r\nHOST:google.com\r\n\r\n";
    send(socketFD, message, strlen(message), 0);
    return 0;
}
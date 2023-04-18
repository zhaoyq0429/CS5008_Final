#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 5

int clients[MAX_CLIENTS]; // Array to hold socket descriptors of clients
int num_clients = 0; // Number of clients currently connected

void *receive_handler(void *);

int main() {
    int sock = 0, read_size;
    struct sockaddr_in serv_addr;
    char message[2000] = {0};
    
    //Create Mutex Lock for the chatroom
    pthread_t thread_id;

    // Creating socket file descriptor
    // AF_INET: address family for IP version 4 IPv4:32bit, IPv6:128bit address length
    // SOCK_STREAM: TCP socket, TCP is a connection-oriented protocol, whereas UDP is a connectionless protocol
    // 0 protocal/layers, i.e., using IP layer 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connecting to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Creating thread to receive messages from the server
    if( pthread_create( &thread_id, NULL, receive_handler, (void*) &sock) < 0) {
        perror("could not create thread");
        return 1;
    }

    // Sending messages to the server
    while(1) {
        //stdin is an input stream where data is sent to and read by a program in FD
        fgets(message, 2000, stdin);
        send(sock , message , strlen(message) , 0 );
        memset(message, 0, sizeof(message));
    }

    return 0;
}

void *receive_handler(void *socket_desc) {
    // Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char server_message[2000] = {0};

    // Receiving messages from the server
    while( (read_size = recv(sock , server_message , 2000 , 0)) > 0 ) {
        printf("%s", server_message);
        memset(server_message, 0, sizeof(server_message));
    }

    if(read_size == 0) {
        printf("Server disconnected\n");
        fflush(stdout);
    }
    else if(read_size == -1) {
        perror("recv failed");
    }

    return 0;
}


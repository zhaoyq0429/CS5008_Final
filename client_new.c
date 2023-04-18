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

typedef struct { 
    int socket; 
    char name[50]; 
} ClientData;

int clients[MAX_CLIENTS]; // Array to hold socket descriptors of clients
int num_clients = 0; // Number of clients currently connected
int connected = 1; // Flag to check whether the client is still connected 

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


    // Get the client's name and send it to the server 
    ClientData client_data; 
    client_data.socket = sock; 
    printf("Enter your name: "); 
    fgets(client_data.name, 50, stdin); 
    client_data.name[strcspn(client_data.name, "\n")] = 0; 
    send(sock , client_data.name , strlen(client_data.name) , 0 ); 
    // Creating thread to receive messages from the server 
    if( pthread_create( &thread_id, NULL, receive_handler, (void*) &client_data) < 0) { 
        perror("could not create thread"); 
        return 1; 
    } // (Sending messages to the server) 
    
    // Sending messages to the server
    while(1) {
        //stdin is an input stream where data is sent to and read by a program in FD
        fgets(message, 2000, stdin);
        message[strcspn(message, "\n")] = 0; // Quit the chatroom if the client enters "Quit" 
        if (strcmp(message, "Quit") == 0) {
            send(sock , message , strlen(message) ,0);  
            connected = 0; // Set the connected flag to 0 when clinet wants to quit
            close(sock); 
            printf("You have disconnected from the chatroom.\n"); 
            exit(0);
        } 

        send(sock , message , strlen(message) ,0); 
        memset(message, 0, sizeof(message));
    }

    return 0;
}

void *receive_handler(void *client_struct) {

    // Get the socket descriptor and client's name 
    ClientData *client_data = (ClientData *)client_struct; 
    int sock = client_data->socket; 
    char *client_name = client_data->name; 
    int read_size; 
    char server_message[2000] = {0}; 
    char sender[50] = {0}; 
    char message[2000] = {0}; 
    // Receiving messages from the server 
    while(connected && (read_size = recv(sock , server_message , 2000 , 0)) > 0 ) {
         sscanf(server_message, "%[^:]: %[^\n]", sender, message); 
         // Only display the message if it's not from the client 
         if (strcmp(sender, client_name) != 0) { 
            printf("%s", server_message); } 
            memset(server_message, 0, sizeof(server_message)); 
            memset(sender, 0, sizeof(sender)); 
            memset(message, 0, sizeof(message)); 
        } 

    if(read_size == 0) {
        printf("Server disconnected\n");
        fflush(stdout);
    }
    else if(read_size == -1 && connected) { 
        // Only show th error message if the clinet is still connected 
        perror("recv failed");
    }

    return 0;
}


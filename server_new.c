#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 8080 //commonly used port number for web applications
#define MAX_CLIENTS 5

typedef struct {
    int socket;
    char name[50];
} Client;

Client clients[MAX_CLIENTS]; // Array to hold client structs
int num_clients = 0; // Number of clients currently connected
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for clients array

void *handle_client(void *);
void broadcast_message(char *, char *);

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1; 
    int addrlen = sizeof(address);
    pthread_t thread_id;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { //1 for success
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to reuse address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) { //setsucces = 1, reuse open to 1
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; //allows the server to accept connections from any available network interface on the machine
    address.sin_port = htons( PORT ); //convert the port number from host byte order to network byte order.

    // Bind the socket to the specified IP address and port
    // To receive the file descriptor of the server, and ask for server addres
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started listening on port %d\n", PORT);
    
    // Accept incoming connections and handle them in separate threads
    while (1) {
        // accept an incoming connection request on server_fd socket
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
        
        if (num_clients+1 > MAX_CLIENTS) {
        printf("Server is full, rejecting new client\n");
        close(new_socket);
        continue;
        }
        
        printf("New client connected\n");

        // Receive the client's name
        char client_name[50] = {0};
        if (recv(new_socket, client_name, 50, 0) <= 0) {
            perror("Failed to receive client name");
            close(new_socket);
            continue;
        }

        printf("%s has joined the chatroom! \n", client_name);

        // Add new client to the clients array
        pthread_mutex_lock(&clients_mutex);
        clients[num_clients].socket = new_socket;
        strncpy(clients[num_clients].name, client_name, 50);
        num_clients++;
        pthread_mutex_unlock(&clients_mutex);

        // Create thread to handle client's messages
        if( pthread_create( &thread_id, NULL, handle_client, (void*) &clients[num_clients - 1]) < 0) {
            perror("could not create thread");
            return 1;
        }
    }

    return 0;
}

void *handle_client(void *client_struct) {
    Client client = *(Client *)client_struct;
    int sock = client.socket;
    char name[50];
    strncpy(name, client.name, 50);

    int read_size;
    char client_message[2000] = {0};

    // Receive messages from the client
    while ((read_size = recv(sock , client_message , 2000 , 0)) > 0 ) {
        // Check if the client wants to quit 
        if (strcmp(client_message, "Quit") == 0) { 
            printf("Client '%s' disconnected\n", client.name); 
            char quit_message[100] = {0};
            snprintf(quit_message, sizeof(quit_message), "%s has left the chatroom!\n", client.name);
            broadcast_message("Server", quit_message);
            // broadcast_message(name, "has left the chatroom! \n");
            break; 
        }
        // Print the received mesasge on the server
        printf("%s: %s\n", name, client_message);
        // Broadcast the message to all connected clients
        broadcast_message(name, client_message);
        // Initialize client array
        memset(client_message, 0, sizeof(client_message));
    }

    // Remove client from the clients array
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].socket == sock) {
            // broadcast_message(name, "has left the chatroom! \n");
            memmove(clients + i, clients + i + 1, (num_clients - i - 1) * sizeof(int));
            num_clients--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    // Close the socket
    close(sock);

    // Detach the thread to free up resrouces 
    pthread_detach(pthread_self());
    return 0;
}

void broadcast_message(char *sender, char *message) { 
    char full_message[2050] = {0}; 

    snprintf(full_message, sizeof(full_message), "%s: %s\n", sender, message); 
    pthread_mutex_lock(&clients_mutex); 

    for (int i = 0; i < num_clients; i++) { 
        if (send(clients[i].socket, full_message, strlen(full_message), 0) == -1) { 
            perror("send failed"); 
            break; 
        } 
    } 
    pthread_mutex_unlock(&clients_mutex);
}


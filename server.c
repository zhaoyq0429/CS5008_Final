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
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for clients array

void *handle_client(void *);
void broadcast_message(char *);

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    pthread_t thread_id;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to reuse address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Bind the socket to the specified IP address and port
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
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        printf("New client connected\n");

        // Add new client to the clients array
        pthread_mutex_lock(&clients_mutex);
        clients[num_clients++] = new_socket;
        pthread_mutex_unlock(&clients_mutex);

        // Create thread to handle client's messages
        if( pthread_create( &thread_id, NULL, handle_client, (void*) &new_socket) < 0) {
            perror("could not create thread");
            return 1;
        }
    }

    return 0;
}

void *handle_client(void *socket_desc) {
    // Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char client_message[2000] = {0};

    // Receive messages from the client
    while ((read_size = recv(sock , client_message , 2000 , 0)) > 0 ) {
        // Broadcast the message to all connected clients
        broadcast_message(client_message);
        memset(client_message, 0, sizeof(client_message));
    }

    // Remove client from the clients array
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (clients[i] == sock) {
            memmove(clients + i, clients + i + 1, (num_clients - i - 1) * sizeof(int));
            num_clients--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    // Close the socket
    close(sock);

    return 0;
}

void broadcast_message(char *message) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (send(clients[i], message, strlen(message), 0) == -1) {
            perror("send failed");
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}


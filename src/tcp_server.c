#include <stdio.h>
#include <string.h>
#include <unistd.h>      // for close function
#include <netinet/in.h>  // for AF_INET, SOCK_STREAM, etc.
#include <arpa/inet.h>   // for inet_ntoa
#include <pthread.h>     // for POSIX Threads

#include <tcp_server.h>  // Your TCP server interface

#define PORT 1803
#define BUFFER_SIZE 1024

// Global variables
volatile int keep_running = 1;
int global_client_fd = -1;  // Initially no client is connected

// Forward declaration of functions
static void *tcp_server_thread(void *arg);

// Function to send data to the client
void send_to_client(const char* data) {
    if (global_client_fd != -1 && data) {
        send(global_client_fd, data, strlen(data), 0);  // Send the data
    } else {
        // Handle error or no connection case
        fprintf(stderr, "No client connected or invalid data\n");
    }
}

// Function to stop the server
void stop_tcp_server() {
    keep_running = 0;
    if (global_client_fd != -1) {
        close(global_client_fd);  // Close the client socket
        global_client_fd = -1;    // Reset the global client file descriptor
    }
}

// TCP server thread function
static void *tcp_server_thread(void *arg) {
    DataReceivedCallback data_handler = (DataReceivedCallback)arg;
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    int opt = 1;
    char buffer[BUFFER_SIZE];

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Could not create socket");
        return NULL;
    }

    // Set the SO_REUSEADDR socket option
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(server_fd);
        return NULL;
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return NULL;
    }

    // Listen
    listen(server_fd, 3);
    printf("Listening for incoming connections on port %d...\n", PORT);

    // Accept incoming connection
    printf("Waiting for incoming connections...\n");
    socklen_t c = sizeof(struct sockaddr_in);

    // Accept connection from an incoming client
    global_client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &c);
    if (global_client_fd < 0) {
        perror("Accept failed");
        close(server_fd);
        return NULL;
    }

    printf("Connection accepted from %s\n", inet_ntoa(client_addr.sin_addr));

    // Receive messages from client
    while (keep_running) {
        ssize_t bytes_received = recv(global_client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';  // Null-terminate the received data
            if (strcmp(buffer, "WIPADRETRO_CONNECT") == 0) {
                char msg[30] = "CLIENT:";
                strcat(msg, inet_ntoa(client_addr.sin_addr));
                data_handler(msg);
                continue;
            } 
            else {
                data_handler("DISCONNECTED");
                break;
            }
            data_handler(buffer);          // Handle the received data
        } else if (bytes_received == 0) {
            puts("Client disconnected");
            data_handler("DISCONNECTED");
            break;
        } else {
            perror("recv failed");
            data_handler("DISCONNECTED");
            break;
        }
    }

    // Cleanup
    close(global_client_fd);  // Close the client socket
    close(server_fd);         // Close the server socket
    global_client_fd = -1;    // Reset the global client file descriptor
    return NULL;
}

// Function to start the TCP server
void start_tcp_server(DataReceivedCallback callback) {
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, tcp_server_thread, (void *)callback)) {
        fprintf(stderr, "Could not create TCP server thread\n");
    } else {
        pthread_detach(thread_id);
    }
}

void force_disconnect_client() {
    if (global_client_fd != -1) {
        close(global_client_fd);
        printf("Client forcefully disconnected\n");
        global_client_fd = -1; // Reset the global client file descriptor
    }
}

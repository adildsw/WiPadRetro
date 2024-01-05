#include <stdio.h>
#include <string.h>
#include <unistd.h>      
#include <netinet/in.h> 
#include <arpa/inet.h>   
#include <pthread.h>

#include <tcp_server.h>

int global_client_fd = -1;
int global_udp_socket_fd = -1;

pthread_t thread_id;

struct sockaddr_in udp_server_addr;

// Function to initialize UDP socket
void init_udp_sender(const char *server_ip) {
    // Close existing socket if open
    if (global_udp_socket_fd != -1) {
        close(global_udp_socket_fd);
    }

    // Create a socket for UDP
    global_udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (global_udp_socket_fd < 0) {
        perror("Cannot create UDP socket");
        return;
    }

    // Set up the server address structure for sending
    memset(&udp_server_addr, 0, sizeof(udp_server_addr));
    udp_server_addr.sin_family = AF_INET;
    udp_server_addr.sin_port = htons(PORT);
    udp_server_addr.sin_addr.s_addr = inet_addr(server_ip);

    printf("UDP Sender Socket created for %s:%d\n", server_ip, PORT);
}

// TCP server thread function
static void *tcp_server_thread(void *arg) {
    data_received_callback data_handler = (data_received_callback)arg;
    int server_fd = -1;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Could not create socket");
        return NULL;
    }

    // Set the SO_REUSEADDR socket option
    int opt = 1;
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

    init_udp_sender(inet_ntoa(client_addr.sin_addr));
    printf("Connection accepted from %s\n", inet_ntoa(client_addr.sin_addr));

    // Receive messages from client
    while (1) {
        ssize_t bytes_received = recv(global_client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            if (strcmp(buffer, "WIPADRETRO_CONNECT") == 0) {
                char msg[30] = "CLIENT:";
                strcat(msg, inet_ntoa(client_addr.sin_addr));
                data_handler(msg);
                continue;
            } 
            else {
                printf("Invalid client");
                break;
            }
        } else if (bytes_received == 0) {
            printf("Client disconnected");
            break;
        } else {
            printf("recv failed");
            break;
        }
    }

    // Cleanup
    close(global_client_fd);
    close(server_fd);
    server_fd = -1;
    global_client_fd = -1;
    data_handler("DISCONNECTED");
    return NULL;
}

// Function to start the TCP server
void start_tcp_server(data_received_callback callback) {
    if (pthread_create(&thread_id, NULL, tcp_server_thread, (void *)callback)) {
        fprintf(stderr, "Could not create TCP server thread\n");
    } else {
        pthread_detach(thread_id);
    }
}

// Function to send data to the client
void send_to_client(const char* data) {
    if (global_client_fd != -1 && data) {
        send(global_client_fd, data, strlen(data), 0);
    } else {
        fprintf(stderr, "No client connected or invalid data\n");
    }
}

// void udp_send_to_client(const char* data) {
//     if (global_udp_socket_fd < 0) {
//         fprintf(stderr, "UDP socket is not initialized.\n");
//         return;
//     }

//     if (data == NULL) {
//         fprintf(stderr, "Data to send is NULL.\n");
//         return;
//     }

//     ssize_t sent_bytes = sendto(global_udp_socket_fd, data, strlen(data), 0, (struct sockaddr *)&udp_server_addr, sizeof(udp_server_addr));
//     if (sent_bytes < 0) {
//         perror("Failed to send UDP data");
//     } else {
//         printf("Successfully sent %zd bytes\n", sent_bytes);
//     }
// }

void udp_send_to_client(unsigned short data) {
    if (global_udp_socket_fd < 0) {
        fprintf(stderr, "UDP socket is not initialized.\n");
        return;
    }

    unsigned short net_data = htons(data); // Convert to network byte order

    ssize_t sent_bytes = sendto(global_udp_socket_fd, &net_data, sizeof(net_data), 0, 
                                (struct sockaddr *)&udp_server_addr, 
                                sizeof(udp_server_addr));
    if (sent_bytes < 0) {
        perror("Failed to send UDP data");
    } else {
        printf("Successfully sent %zd bytes\n", sent_bytes);
    }
}


// Function to stop the server
void stop_tcp_server() {
    if (global_client_fd != -1) {
        close(global_client_fd);
        global_client_fd = -1;
        pthread_detach(thread_id);
    }
}

// Function to force disconnect the client
void force_disconnect_client() {
    if (global_client_fd != -1) {
        send_to_client("WIPADRETRO_DISCONNECT");
        close(global_client_fd);
        global_client_fd = -1;
        printf("Client forcefully disconnected\n");
    }
}






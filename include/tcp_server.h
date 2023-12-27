#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#define PORT 1803
#define BUFFER_SIZE 1024

typedef void (*data_received_callback)(const char*);

static void *tcp_server_thread(void *arg);
void start_tcp_server(data_received_callback callback);
void send_to_client(const char* data);
void stop_tcp_server();
void force_disconnect_client();

#endif

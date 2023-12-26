#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <netinet/in.h>

typedef void (*DataReceivedCallback)(const char*);

void start_tcp_server(DataReceivedCallback callback);
void send_to_client(const char* data);
void stop_tcp_server();
void force_disconnect_client();

#endif

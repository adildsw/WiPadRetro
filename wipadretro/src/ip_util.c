#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ip_util.h>

char** get_ip_addresses(int* count) {
    struct ifaddrs *ifaddr, *ifa;
    int family, n;
    char** ips = NULL;
    *count = 0;

    if (getifaddrs(&ifaddr) == -1) {
        perror("Error getting IP addresses");
        return NULL;
    }

    for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
        if (ifa->ifa_addr == NULL)
            continue;
        family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) { 
            (*count)++;
        }
    }

    ips = malloc(*count * sizeof(char*));
    if (!ips) {
        perror("Malloc failed for ips");
        freeifaddrs(ifaddr);
        return NULL;
    }

    int i = 0;
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
        family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) { 
            void *tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char* ipstr = malloc(INET_ADDRSTRLEN);
            if (ipstr == NULL) {
                perror("Malloc failed for ipstr");
                continue;
            }
            inet_ntop(AF_INET, tmpAddrPtr, ipstr, INET_ADDRSTRLEN);
            ips[i++] = ipstr;
        }
    }

    freeifaddrs(ifaddr);
    return ips;
}
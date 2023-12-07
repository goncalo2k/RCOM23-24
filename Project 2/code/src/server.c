#include "server.h"
#include <arpa/inet.h>

int fetch_address(char *host, char *port, struct addrinfo **res) {
    
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    return getaddrinfo(host, port, &hints, res);
}

int connector(char *host, char *port) {
    struct addrinfo *res;

    printf("Getting address\n");
    int error = fetch_address(host, port, &res);
    if (error < 0) {
        printf("Could not get address info: %s\n", gai_strerror(error));
        return -1;
    }

    for (struct addrinfo *addr = res; addr != NULL; addr = addr->ai_next) {
        char addr_str[INET6_ADDRSTRLEN];
        inet_ntop(
            addr->ai_family,
            (addr->ai_family == AF_INET6)
                ? (void *)&((struct sockaddr_in6 *)addr->ai_addr)->sin6_addr
                : (void *)&((struct sockaddr_in *)addr->ai_addr)->sin_addr,
            addr_str, INET6_ADDRSTRLEN);

        printf("Trying %s\n", addr_str);
        int fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (fd < 0 || connect(fd, addr->ai_addr, addr->ai_addrlen) < 0) {
            printf("Error connecting to address '%s': %s\n", addr_str,
                strerror(errno));
        } else {
            printf("Connected to address %s\n", addr_str);
            freeaddrinfo(res);
            return fd;
        }
    }

    printf("Could not connect to any address!\n");
    freeaddrinfo(res);
    return -1;
}
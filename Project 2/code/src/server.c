#include "server.h"

unsigned long proccess, fsize = 0;

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

int recdata(int fd, char *buf, int len) {
    int total_bytes_read = 0, read = 0;

    sleep(1);

    while ((read = recv(fd, buf + total_bytes_read,
                              len - total_bytes_read, MSG_DONTWAIT)) > 0) {
        total_bytes_read += read;
        sleep(1);
    }

    if (errno != EAGAIN && errno != EWOULDBLOCK) {
        printf("%s\n", strerror(errno));
        return -1;
    }

    buf[total_bytes_read] = '\0';
    if (total_bytes_read > 0)
        printf("%s", buf);

    return total_bytes_read;
}

int fetch_code(int fd) {
    char buf[2048] = "";
    int curr_code = -1;

    if (recdata(fd, buf, 2048) < 0)
        return -1;

    sscanf(buf, "%d", &curr_code);

    int n = 0;
    unsigned long temp;
    if (sscanf(buf, " %*[^(](%lu bytes)%n", &temp, &n) && n > 0) {
        fsize = temp;
        printf("File size: %lu bytes\n", fsize);
    }

    return curr_code;
}   

int enter_passive_mode(int pfd, char *host, char *port) {
    char cmnd[] = "pasv\n", buf[256];
    if (send(pfd, cmnd, sizeof(cmnd) - 1, 0) < 0) {
        printf("Couldnt send passive command: %s\n", strerror(errno));
        return -1;
    }

    recdata(pfd, buf, 256);
    int curr_code = -1;
    int h1, h2, h3, h4, p1, p2;
    sscanf(buf, "%d %*[^(](%d,%d,%d,%d,%d,%d)\n", &curr_code, &h1, &h2,
           &h3, &h4, &p1, &p2);
    if (curr_code != PASSIVE_MODE) {
        printf("Could not enter passive mode\n");
        return -1;
    }

    sprintf(host, "%d.%d.%d.%d", h1, h2, h3, h4);
    sprintf(port, "%d", p1 * 256 + p2);
    printf("Entering passive mode (%s:%s)\n", host, port);
    return 0;
}

int transfer_file(int fd, const char *path) {
    char buf[512];
    int len;
    int curr_code;

    snprintf(buf, 512, "retr %s\n%n", path, &len);
    if (send(fd, buf, len, 0) < 0) {
        printf("Couldnt send 'retr' command: %s\n", strerror(errno));
        return -1;
    }

    curr_code = fetch_code(fd);
    if (curr_code != TRANSFER_READY && curr_code != TRANSFER_FINISHED) {
        printf("Could not transfer file\n");
        return -1;
    }
    return 0;
}

int recieve_file(int fd, char* local_file_name) {
    int ofd = open(local_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0744);
    
    if (ofd == -1) {
        perror("Error opening or creating file");
        return -1;
    }

    char buf[1024];
    ssize_t bread, bwritten;

    while ((bread = recv(fd, buf, sizeof(buf), 0)) > 0) {
        bwritten = write(ofd, buf, bread);

        if (bwritten == -1) {
            perror("Error writing to file");
            close(ofd);
            return -1;
        }

        // Assuming 'fsize' is a known variable representing the total file size
        // and 'proccess' is a known variable representing the processed bytes
        printf("Written %zd bytes (%lf%%) to file\n", bwritten, (double)(proccess * 100.0 / fsize));
        printf("%.*s\n", (int)bread, buf);
        
        proccess += bwritten;
    }

    close(ofd);

    return 0;
}

int login(int fd, const char *username, const char *password) {
    char buf[1024];
    int len, curr_code;

    curr_code = fetch_code(fd);
    if (curr_code == SUCCESSFUL_LOGIN) {
        printf("Logged in\n");
        return 0;
    }

    snprintf(buf, 512, "user %s\n%n", username, &len);
    if (send(fd, buf, len, 0) < 0) {
        printf("Error sending user command: %s\n", strerror(errno));
        return -1;
    }

    curr_code = fetch_code(fd);
    if (curr_code == SUCCESSFUL_LOGIN) {
        printf("Successfully logged in as '%s'\n", username);
        return 0;
    } else if (curr_code != WAITING_FOR_PASSIVE) {
        printf("Couldnt login as '%s'\n", username);
        return -1;
    }

    snprintf(buf, 512, "pass %s\n%n", password, &len);
    if (send(fd, buf, len, 0) < 0) {
        printf("Error sending passive command: %s\n", strerror(errno));
        return -1;
    }

    curr_code = fetch_code(fd);
    if (curr_code != SUCCESSFUL_LOGIN) {
        printf("Couldnt login as '%s' using password '%s'\n",
              username, password);
        return -1;
    }

    printf("Login successful!\n");
    return 0;
}
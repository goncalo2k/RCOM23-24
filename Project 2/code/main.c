#include "url.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Bad use of command. Usage: ./download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    } 
    
    int cfd, pfd;
    char username[256] = "anonymous", password[256] = "",
    host[256] = "", port[256] = "21", path[256] = "",
    pass_host[INET_ADDRSTRLEN], pass_port[6];

    if (url_parser(argv[1], username, password, host, port, path)) {
        printf("Parsing error. Usage: ./download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }
    
    printf("Username: %s\n", username);
    printf("Password: %s\n", password);
    printf("Host: %s\n", host);
    printf("Port: %s\n", port);
    printf("Path: %s\n", path);
    
    cfd = connector(host, port) ;
    if (cfd < 0)
        exit(-1);

    if (login(cfd, username, password))
        exit(-1);
    printf("Logged in, checking for pass now\n");
    if (enter_passive_mode(cfd, pass_host, pass_port) < 0)
        exit(-1);
    printf("Passive mode entered - connecting now\n");
    printf("Passive host: %s\n", pass_host);
    printf("Passive port: %s\n", pass_port);
    pfd = connector(pass_host, pass_port);

    if (pfd < 0)
        exit (-1);
    
    if(transfer_file(cfd, path) < 0)
        exit(-1);

    close(cfd);
    close(pfd);

    return 0;
}

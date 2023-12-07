#include "url.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Bad use of command. Usage: ./download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    } 

    char username[256] = "anonymous", password[256] = "",
    host[256] = "", port[256] = "21", path[256] = "";

    if (url_parser(argv[1], username, password, host, port, path)) {
        printf("Parsing error. Usage: ./download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }

    printf("Username: %s\n", username);
    printf("Password: %s\n", username);
    printf("Host: %s\n", username);
    printf("Port: %s\n", username);
    printf("Path: %s\n", username);
    
    return 0;
}

#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define WAITING_FOR_PASSIVE     331
#define SUCCESSFUL_LOGIN        230
#define PASSIVE_MODE            227
#define TRANSFER_READY          150
#define TRANSFER_FINISHED       226

int connector(char *host, char *port);

int fetch_address (char *host, char *port, struct addrinfo **res);

int recdata(int fd, char *buf, int len);

int fetch_code(int fd);

int enter_passive_mode(int fd, char *host, char *port);

int transfer_file(int fd, const char *path);

int recieve_file(int fd, char* local_file_name);

int login(int fd, const char *username, const char *password);

#endif
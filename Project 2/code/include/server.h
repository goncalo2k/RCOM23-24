#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int connector(char *host, char *port);

int fetch_address (char *host, char *port, struct addrinfo **res);

#endif
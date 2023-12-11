#ifndef _URL_H_
#define _URL_H_

#include <regex.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef enum {
    START,
    SCHEME,
    USERNAME,
    HOST,
    PORT,
    PATH,
    END
} ParserState;

int url_parser(const char *url, char *username, char *password, char *host,
 char *port, char *path);
 
 #endif
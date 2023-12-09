#include "url.h"

#define AT "@"
#define BAR "/"
#define HOST_REGEX "%*[^/]//%[^/]"
#define HOST_AT_REGEX "%*[^/]//%*[^@]@%[^/]"
#define RESOURCE_REGEX "%*[^/]//%*[^/]/%s"
#define USER_REGEX "%*[^/]//%[^:/]"
#define PASS_REGEX "%*[^/]//%*[^:]:%[^@\n$]"
#define RESPCODE_REGEX "%d"
#define PASSIVE_REGEX "%*[^(](%d,%d,%d,%d,%d,%d)%*[^\n$)]"

int url_parser(const char *input, char *username, char *password, char *host,
               char *port, char *path)
{
    ParserState state = START;

    while (state != END)
    {
        int n =0;
        switch (state)
        {
        case START:
            state  = SCHEME;
            break;
        
        case SCHEME:
            sscanf(input, "ftp://%n", &n);
            input +=n ;
            state = USERNAME;
            break;

        case USERNAME:
            if (sscanf(input, "%255[^:@/]:%255[^:@/]@%n", username, password, &n) && n > 0) 
            {
                input += n;
            } else if (sscanf(input, "%255[^:@/]@%n", username, &n) && n > 0) 
            {
                input += n;
                password[0] = '\0';
            } else {
                username[0] = '\0';
                password[0] = '\0';
            }
            state = HOST;
            break;
        case HOST:
            if (sscanf(input, "%255[^:@/]:%n", host, &n) && n > 0)
                state = PORT;
            else if (sscanf(input, "%255[^:@/]/%n", host, &n) && n > 0)
                state = PATH;
            else if (sscanf(input, "%255[^:@/]%n", host, &n) && n > 0)
                state = END;
            else
                return -1;

            input += n;
            break;

        case PORT:
            if (sscanf(input, "%5[0123456789]/%n", port, &n) && n > 0)
                state = PATH;
            else if (sscanf(input, "%5[0123456789]%n", port, &n) && n > 0)
                state = END;
            else
                return -1;

            input += n;
            break;
        case PATH:
            if (sscanf(input, "%255[^:@]%n", path, &n) == -1)
                return -1;

            input += n;

        default:
            state = END;;
            break;
        }
    }
    
    if (username[0] == '\0')
        strcpy(username, "anonymous");
    if (port[0] == '\0')
        strcpy(port, "21");

    if (strlen(input) > 0) return -1;

    return 0;
};

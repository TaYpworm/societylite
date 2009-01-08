#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
 
#ifndef   NI_MAXHOST
#define   NI_MAXHOST 1025
#endif
 
int main(void)
{
    struct addrinfo * result;
    struct addrinfo * res;
    int error;
 
    result = new addrinfo;
    
    /* resolve the domain name into a list of addresses */
    //error = getaddrinfo("www.google.com", NULL, NULL, &result);
    error = getaddrinfo("localhost", NULL, NULL, &result);
 
    if (0 != error)
    {   
        fprintf(stderr, "error in getaddrinfo: %s\n", gai_strerror(error));
        return 1;
    }   
 
    if (result == NULL)
    {   
        fprintf(stderr, "getaddrinfo found no results\n");
        return 0;
    }   
 
    /* loop over all returned results and do inverse lookup */
    for (res = result; res != NULL; res = res->ai_next)
    {   
        char hostname[NI_MAXHOST] = "";
 
        error = getnameinfo(res->ai_addr, res->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0); 


        if (0 != error)
        {
            fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(error));
            continue;
        }
 
        if (*hostname)
        {
            printf("hostname: %s\n", hostname);
        }
 
    }   
 
    freeaddrinfo(result);
 
    return 0;
}


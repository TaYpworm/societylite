//Jim King
//server_accept.c
//wrapper for socket accept from w richard stevens

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <stddef.h>
#include <time.h>
#include <errno.h>
#include "socket_accept.h"

#define STALE 30 //client's name can't be older than this (in sec)

/**
 * Wait for a client connection to arrive, and accept it.  We also
 * obtain the client's user ID from the pathname that it must
 * bind before calling us.
 * @param listenfd the fd returned be serv_listen
 * @param uidptr memory location where user info is to be put
 * @return a new fd if all OK, < 0 on error
 */
int acceptConnection( int serverSocket, uid_t *uidptr )
{
    int clifd, len;
    time_t staletime;
    struct sockaddr_in unix_addr;
    struct stat statbuf;

    len = sizeof( unix_addr );
    if( (clifd = accept( serverSocket, (struct sockaddr *) &unix_addr, (socklen_t *) &len)) < 0 )
        return -1;

    //get status from client
    if( fstat( clifd, &statbuf) < 0 )
        return -2;

#ifdef S_ISSOCK
    if( S_ISSOCK(statbuf.st_mode) == 0 )
        return -3; //not socket
#endif

    //has time expired?
    /*
    staletime = time(NULL) - STALE;
    if( statbuf.st_atime < staletime ||
        statbuf.st_ctime < staletime ||
        statbuf.st_mtime < staletime )
            return -5;  //too old
*/
    if( uidptr != NULL )
        *uidptr = statbuf.st_uid;

    return clifd;
}


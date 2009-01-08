#include "server_listen.h"
#include <netdb.h>

#include <iostream>

using namespace std;

//Create a server endpoint of a connection

/**
 *  @param name server will use this as the hostname
 *  @return fd if all OK, < 0 on error
 */
int startServer( const int &desiredPort )
{
  int fd, len;
  struct sockaddr_in unix_addr;

  //create unix domain stream socket
  if( (fd = socket( AF_INET, SOCK_STREAM, 0 )) < 0 )
    return -1;

  //clear struct
  memset( &unix_addr, 0, sizeof(unix_addr) );
  unix_addr.sin_family = AF_INET;
  unix_addr.sin_addr.s_addr = htonl( INADDR_ANY );
  
  //passing it 0
  unix_addr.sin_port = htons ( desiredPort );

  //bind name to descriptor
  if( bind( fd, (struct sockaddr * ) &unix_addr, sizeof( unix_addr ) ) < 0 )
    return -2;

  if( listen(fd, 5 ) < 0 ) //tell kernal we're a server
    return -3;

  return fd;
}

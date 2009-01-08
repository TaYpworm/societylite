/* This is something like what the brain would do when it wants to       */
/* listen to an ear                                                      */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "defines.h"
#include "Port.h"
#include "debug.h"
#include "memstat.h"

extern int errno;

/*------------------------------------------------------------------------*/
/* Open connection to hostname:port as FILE *, so that the same fread/fwrite */
/* calls can be used for either file or socket I/O.                          */

FILE *OpenPortFile (char *hostname, int port, char *mode)
{
  FILE *fp;
  int sfd;

  fp = NULL;
  sfd = OpenPort (hostname, port);
  if (sfd > 0)
    fp = fdopen (sfd, mode);
  return (fp);
}

/*------------------------------------------------------------------------*/
/* Open socket connection to hostname:port                                   */

int OpenPort (char *hostname, int port)
{
  struct sockaddr server;
  struct sockaddr_in *dummy;
  struct hostent *host;
  int sfd;
  char *dotaddr;

/* Can specify host as either dotted address or name */

  dotaddr = hostname;

  bzero ((char *) &server, sizeof (struct sockaddr_in));
  dummy = (struct sockaddr_in *) &server;         /* needed for bug in RedHat Linux? */
  dummy->sin_family = AF_INET;

  dummy->sin_addr.s_addr = inet_addr (dotaddr);   /* First assume it's dotted address */
  if (dummy->sin_addr.s_addr == INADDR_NONE)      /* if that didn't work       */
  {
    host = gethostbyname (hostname);              /* look up host name */
    if (host == NULL)
    {
      printf ("Port::OpenPort: gethostbyname error '%s', name = '%s':\n", 
              strerror (errno), hostname);
      Abort (__FILE__, __LINE__, -7);
    }
    dummy->sin_family  = host->h_addrtype;
    bcopy (host->h_addr, (caddr_t) &dummy->sin_addr.s_addr, host->h_length);
//  dotaddr = (char *) inet_ntoa (dummy->sin_addr.s_addr);
  }
//printf ("Port::OpenPort: opening socket to %s, port %d\n", hostname, port);

  dummy->sin_port = htons (port);

  if ((sfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf ("Port::OpenPort: socket error '%s'\n", strerror (errno));
    Abort (__FILE__, __LINE__, -7);
  }

  if (connect (sfd, &server, sizeof (struct sockaddr_in)) < 0)
  {
    printf ("Port::OpenPort: connect error, '%s'\n", strerror (errno));
    Abort (__FILE__, __LINE__, -7);
  }
  return (sfd);
}

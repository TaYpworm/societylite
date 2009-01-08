/* Time-stamp: <2 Nov 2001 11:13:24 james> */
/* This is just a shell, so the parsing routines can be run as a   */
/* stand-alone program for testing                                 */

#include <stdlib.h>
#include <stdio.h>

#include "arrays.h"

#define HOSTLEN  64

ARRAYS *ParseInput (int, char *, int);

int Node, nNodes, IONode;
int FSV;                     /* Frequency, ticks per second */
char host [HOSTLEN], *cwd, *job;

main (argc, argv)
int argc;
char **argv;
{
  ARRAYS *AR;
  char *filename;

/* Do some parsing of command line, and set options */

  if (argc <= 1)
  {
    fprintf (stderr, "%s: no input file specified, quitting\n", argv [0]);
    exit (0);
  }

  filename = argv [1];

  AR = ParseInput (0, filename, 0);
  if (AR != NULL)
  {
    printf ("Parse test finished\n");
  }
}

/*---------------------------------------------------------------------------*/
/* Functions to print errorr message from input parsing.  Since parsing is   */
/* presumed to be identical on all nodes, it only printfs on the root node.  */
/* The functions all uses vararg, with a format string and values as for     */
/* printf.                                                                   */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "input.h"

extern int DB_Node;

/*---------------------------------------------------------------------------*/
/* print message to stdout only.                                             */

void printerr (char *format, ...)
{
  va_list args;
  char str [1024];

  if (DB_Node == 0)
  {
    va_start (args, format);
    vsprintf (str, format, args);

    fputs (str, stdout);
  }
}

/*---------------------------------------------------------------------------*/
/* Prints error message to both stderr and stdout.                           */

void print2err (char *format, ...)
{
  va_list args;
  char str [1024];

  if (DB_Node == 0)
  {
    va_start (args, format);
    vsprintf (str, format, args);

    fputs (str, stderr);
    fputs (str, stdout);
  }
}

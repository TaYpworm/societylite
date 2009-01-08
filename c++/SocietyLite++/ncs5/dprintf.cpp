/*------------------------------------------------------------------
* dprintf: substitute for printf when printing out debugging messages
* automatically adds node (define global int node in main.cpp, and set it)
* to line, and flushes stdout
-------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern int DB_Node;


/*---------------------------------------------------*/

void dprintf (char *format, ...)
{
  va_list args;
  char str [1024];

  va_start (args, format);
  vsprintf (str, format, args);

  printf ("Node %3d: %s", DB_Node, str);
  fflush (stdout);
}

/*---------------------------------------------------*/

void dfprintf (FILE *out, char *format, ...)
{
  va_list args;
  char str [1024];

  va_start (args, format);
  vsprintf (str, format, args);
  fprintf (out, "Node %3d: %s", DB_Node, str);
  fflush (out);
}

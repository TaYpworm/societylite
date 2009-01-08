/* Redefine file open & close functions, to count files as they're opened    */
/* and closed.  This makes it possible to keep a count of how many files are */
/* open at any time, for debugging.  Not needed in production code.          */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "memstat.h"

int OpenFiles = 0;
int MaxOpenFiles;

/*------------------------------------------------------------------------*/

void GetMaxFiles ()
{
  MaxOpenFiles = sysconf (_SC_OPEN_MAX);
}

/*------------------------------------------------------------------------*/

int NumOpenFiles ()
{
  return (OpenFiles);
}

/*------------------------------------------------------------------------*/

FILE *Qfopen (char *name, char *mode, char *file, int line)
{
  FILE *fp;

  fp = NULL;
  if (OpenFiles >= MaxOpenFiles)
    printf ("%s:%d: Error number of open file already at max (%d)\n", file, line, MaxOpenFiles);
  else
  {
    fp = fopen (name, mode);
    if (fp == NULL)
      fprintf (stderr, "%s:%d: Error opening file %s\n", file, line, name);
    else 
      OpenFiles++;
  }
  return (fp);
}

/*------------------------------------------------------------------------*/

FILE *Qfdopen (char *name, char *mode, char *file, int line)
{
  FILE *fp;

  fp = NULL;
  if (OpenFiles >= MaxOpenFiles)
    printf ("%s:%d: Error number of open file already at max (%d)\n", file, line, MaxOpenFiles);
  else
  {
    fp = fopen (name, mode);
    if (fp == NULL)
      fprintf (stderr, "%s:%d: Error opening file %s\n", file, line, name);
    else 
      OpenFiles++;
  }
  return (fp);
}

/*------------------------------------------------------------------------*/
/* Open socket connection to hostname:port                                   */

int Qfclose (FILE *fp, char *file, int line)
{
  if (fclose (fp) == 0)
    OpenFiles--;
}

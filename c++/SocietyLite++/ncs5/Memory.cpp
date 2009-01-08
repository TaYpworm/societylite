/* Some functons related to memory errors and such.                          */
/* running parts of the program.                                             */

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#include "defines.h"
#include "Memory.h"
#include "NodeInfo.h"
#include "debug.h"

extern NodeInfo *NI;

void MemError (char *where)
{
  int kBytes;

  printf ("MemError: allocation failed in %s (out of memory?)\n", where);
  fprintf (stderr, "MemError: allocation failed in %s (out of memory?)\n", where);

  kBytes = GetMemoryUsed ();

  printf ("MemError: Total memory allocated to this node = %d KBytes\n", kBytes);
  fprintf (stderr, "MemError: Total memory allocated to this node = %d KBytes\n", kBytes);

/* Figure out some way to kill other nodes? */

  Abort (__FILE__, __LINE__, -1);
}

/*---------------------------------------------------------------------------*/
/* Get memory used from /proc/{pid}/statm.                                   */

int GetMemoryUsed ()
{
  FILE *proc;
  int npages, nKb;
  char name [1024];

  sprintf (name, "/proc/%d/statm", NI->pid);
  proc = fopen (name, "r");
  if (proc == NULL)
  {
    nKb = -1;
  }
  else
  {
    fscanf (proc, "%d", &npages);
    nKb = 4 * npages;

    fclose (proc);
  }
  return (nKb);
}

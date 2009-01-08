/* General purpose abort functions.  All exit/MPI_Abort calls funnel through */
/* this, so file/line can be easily printed.                                 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

#include "debug.h"

/*------------------------------------------------------------------------*/

void Abort (char *file, int line, int code)
{
  printf ("%s:%d: An error at this point caused the program to be aborted.  Error code = %d\n", 
          file, line, code);
  fprintf (stderr, "%s:%d: An error at this point caused the program to be aborted.  Error code = %d\n",
          file, line, code);

  MPI_Abort (MPI_COMM_WORLD, code);
}

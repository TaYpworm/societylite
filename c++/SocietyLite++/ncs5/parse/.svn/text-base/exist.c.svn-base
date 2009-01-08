/* Check whether file exists and has length > 0.  If so, open for reading and */
/* return FILE *.  Otherwise return NULL                                      */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

FILE *exist (char *filename)
{
  struct stat fs;
  FILE *fp;

  fp = NULL;
  stat (filename, &fs);

  if (fs.st_size > 0)
    fp = fopen (filename, "r");
  return (fp);
}

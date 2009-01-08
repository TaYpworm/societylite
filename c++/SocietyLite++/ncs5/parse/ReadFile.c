/* Use MPI to read & distribute input file.  Node 0 does actual reading,     */
/* others get data via MPI.  Returns pointer to buffer containing file       */
/* contents, or NULL if error.  Number of bytes read in argument.            */

/* Note that the parser requires the buffer to have two empty bytes after    */
/* the data, which it will fill with its YY_END_OF_BUFFER_CHAR.  This is why */
/* buffers are allocated as (nbytes+2).                                      */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mpi.h>

#define TRUE 1
#define FALSE 0

#define BLKSIZE 10240

char *ReadFile (char *name, int node, int *bytes)
{
  FILE *in;
  MPI_Status status;
  struct stat fs;
  size_t nbytes, nread;
  int i, nnodes, nblk;
  char *data, *send, *recv, *end;

  if (node == 0)
  {
    stat (name, &fs);
    nbytes = fs.st_size;
  
    in = fopen (name, "r");
  
    if (in == NULL)
    {
      print2err ("Error: Could not open input file '%s'.\n", name);
      *bytes = -1;
    }
    else
    {
      data = (char *) malloc (nbytes + 2);
    
      nread = fread (data, 1, nbytes, in);
      fclose (in);
  
      *bytes = (int) nbytes;      /* in case size_t != int */
    }
  }

  MPI_Bcast (bytes, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (*bytes > 0)
  {
    if (node == 0)
    {
      MPI_Comm_size (MPI_COMM_WORLD, &nnodes);      /* Hack for now */

      end = data + *bytes;
      send = data;
      while (send < end)
      {
        nblk = end - send;
        if (nblk > BLKSIZE) nblk = BLKSIZE;
        for (i = 1; i < nnodes; i++)
        {
          MPI_Send (send, nblk, MPI_BYTE, i, 0, MPI_COMM_WORLD);
        }
        send += nblk;
      }
    }
    else
    {
      data = (char *) malloc (*bytes + 2);
      recv = data;
      end = data + *bytes;
      while (recv < end)
      {
        MPI_Recv (recv, *bytes, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
        recv += BLKSIZE;
      }
    }

    data [*bytes] = '\0';       /* make sure it's NULL-terminated, as */
                                /* calling code may treat as string   */
  }
  else
    data = NULL;

  return (data);
}


/*----------------------------------------------------------------------------*/
/* This reads a file of real numbers and returns array of doubles.  If the   */
/* reverse flag is set, then the array will be returned reversed, as used by */
/* PSGFile.  Returns NULL if node 0 could not open file, or other error.     */

double *ReadFPNFile (char *name, int node, int *nvals, int reverse)
{
  FILE *in;
  MPI_Status status;
  struct stat fs;
  size_t nbytes, nread;
  double f, *fpn, *offset;
  int i, j, n, ws, nnodes, doublesPerBlock;
  char *data, *s, *end;

  if (node == 0)
  {
    stat (name, &fs);
    nbytes = fs.st_size;

    in = fopen (name, "r");
    if (in == NULL)
    {
      print2err ("Error: Could not open input file '%s'.\n", name);
      *nvals = -1;
    }
    else
    {
      data = (char *) malloc (nbytes + 2);
    
      nread = fread (data, 1, nbytes, in);
      fclose (in);
    
      data [nbytes] = '\0';       
    
/* Count the number of numbers in the file */
    
      s = data;
      end = data + nread;
      n = 0;
      ws = TRUE;
      while (s < end)
      {
        if (isspace (*s))
        {
          ws = TRUE;
        }
        else
        {
          if (ws)
            n++;
          ws = FALSE;
        }
        s++;
      }
      *nvals = n;
    
    /* Allocate space and read them in */
    
      fpn = (double *) calloc (*nvals, sizeof (double));
    
      s = data;
      end = data + nread;
      if (reverse)
      {
        n = *nvals - 1;
        while (s < data + nread && n>=0 )
        {
          f = strtod (s, &end);
          if (s == end)
          {
            s = data + nread;
          }
          else
          {
            fpn [n] = f;
            s = end;
            n--;
          }
        }
      }
      else
      {
        n = 0;
        while (s < data + nread && n<*nvals)
        {
          f = strtod (s, &end);
          if (s == end)
          {
            s = data + nread;
          }
          else
          {
            fpn [n] = f;
            s = end;
            n++;
          }
        }
      }
      free (data);
    }
  }

  MPI_Bcast (nvals, 1, MPI_INT, 0, MPI_COMM_WORLD);
  doublesPerBlock = floor(BLKSIZE/sizeof(double));

  //if there are many values, need to send in smaller blocks
  // to keep MPI from breaking
  if (*nvals > 0)
  {
    if (node == 0)
    {
      MPI_Comm_size (MPI_COMM_WORLD, &nnodes);
      offset = fpn;
      for( i=0; i<*nvals; i+=doublesPerBlock )
      {
        for (j = 1; j < nnodes; j++)
        {
          if( i+doublesPerBlock > *nvals )
            MPI_Send (offset, (*nvals-i), MPI_DOUBLE, j, 0, MPI_COMM_WORLD);
          else
            MPI_Send (offset, doublesPerBlock, MPI_DOUBLE, j, 0, MPI_COMM_WORLD);
        }
        offset += doublesPerBlock;
      }
    }
    else
    {
      fpn = (double *) calloc (*nvals, sizeof (double));
      offset = fpn;

      for( i=0; i<*nvals; i+=doublesPerBlock )
      {
        if( i+doublesPerBlock > *nvals )
          MPI_Recv (offset, (*nvals-i), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
        else
          MPI_Recv (offset, doublesPerBlock, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
        offset += doublesPerBlock;
      }
    }
  }
  else
    fpn = NULL;

  return (fpn);
}

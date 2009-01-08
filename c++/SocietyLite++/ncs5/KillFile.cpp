/* Creates the "kill file", which contains a list of the nodes the program   */
/* is running on, and the process id on each node.  It is used by the 'kf'   */
/* script to kill a misbehaving program.  (Not an easy task, otherwise.)     */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <mpi.h>

#include "KillFile.h"
#include "Managers.h"
#include "debug.h"
#include "defines.h"
#include "memstat.h"

#define MEM_KEY KEY_OTHER

#define FILELEN (HOSTLEN + 8)

static char *KillFileName;


void MakeKillFile ()
{
  MPI_Status status;
  FILE *kf;
  int i, len;
  char name [FILELEN];

  sprintf (name, "%s %d", NI->host, NI->pids [NI->Node]);

  if (NI->Node == 0)
  {
    len = strlen (NI->cwd) + 16;
    KillFileName = (char *) calloc (len, sizeof (char));

    sprintf (KillFileName, "%s/%d.kf", NI->cwd, NI->pids [NI->Node]);

    kf = fopen (KillFileName, "w");
    fprintf (kf, "%s 0\n", name);
    for (i = 1; i < NI->nNodes; i++)
    {
      MPI_Recv (name, FILELEN, MPI_CHAR, MPI_ANY_SOURCE, TAG_PIDINFO, MPI_COMM_WORLD, &status);
      fprintf (kf, "%s %d\n", name, status.MPI_SOURCE);
    }
    fclose (kf);
  }
  else
  {
    MPI_Send (name, strlen (name) + 1, MPI_CHAR, 0, TAG_PIDINFO, MPI_COMM_WORLD);
  }
}

/*---------------------------------------------------------------------------*/
/* On the root node, wait for DONE message from the other nodes              */

double AllDone (double TimeOut)
{
  struct timeval tv;
  MPI_Status status;
  double tstart, t0, t1, et, dtprint;
  int usec, count, num, flag, tmp [3];
  bool timedout;

  dtprint = 1.0;
  et = 0.0;
  if (NI->Node == 0)
  {
    count = 0;
    num = (NI->nNodes - 1);        /* Because root does its own */
    usec = (int) (0.025 * 1000000.0);
    tstart = t0 = MPI_Wtime ();
    timedout = false;

    printf ("Root node has finished, waiting for %d others\n", num);
    while ((count < num) && !timedout)
    {
      MPI_Iprobe (MPI_ANY_SOURCE, TAG_DONE, MPI_COMM_WORLD, &flag, &status);
      if (flag)
      {
        MPI_Recv (tmp, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
        count++;
      }
      else
      {
        t1 = MPI_Wtime ();
        et = t1 - tstart;
        if (et > TimeOut)
          timedout = true;
        else
        {
          tv.tv_sec = 0;             /* Wait a bit before probing again */
          tv.tv_usec = usec;
//        select (0, NULL, NULL, NULL, &tv);
        }
      }
      if ((t1 - t0) >= dtprint)
      {
        printf ("Wait %5.1f of %5.1f, %d of %d nodes have finished\n", 
                et, TimeOut, count + 1, NI->nNodes);
        t0 = t1;
      }
    }
    if (timedout)
    {
      fprintf (stderr, "Timed out waiting for done messages from other nodes (%f sec)\n", et);
      fprintf (stderr, "Use the kill file if you suspect a problem...\n");
    }
    else
    {
      printf ("Root got TAG_DONE messages from all nodes (et = %f sec)\n", et);
      printf ("Program completed successfully!\n");
      if (NI->nSleep == 0) EraseKillFile ();
    }
  }
  else
  {
    MPI_Send (tmp, 1, MPI_INT, 0, TAG_DONE, MPI_COMM_WORLD);    /* Send signal that this node is done */
  }
  et = MPI_Wtime () - NI->tstart;
  MPI_Finalize ();
  return (et);
}

/*---------------------------------------------------------------------------*/
/* Erases the kill file if the job completes successfully                    */

void EraseKillFile ()
{
  unlink (KillFileName);
}

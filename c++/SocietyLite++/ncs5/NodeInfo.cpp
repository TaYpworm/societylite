/* Creates an array of structures that holds info about all the nodes        */
/* running parts of the program.                                             */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>

#include "NodeInfo.h"
#include "defines.h"
#include "debug.h"
#include "memstat.h"

#define MEM_KEY KEY_OTHER


NodeInfo::NodeInfo (int node, int nodes, int argc, char *argv [])
{
  FILE *proc;
  double bmip;
  int i;
  char *arg, *s, *stop, line [132];
  bool found;

  Node   = node;
  nNodes = nodes;
  check  = ConnectRpt = SpikeRpt = false;


  tstart = MPI_Wtime ();

/* Do some parsing of command line, and set options */

  if (argc <= 1)
  {
    if (Node == 0)
    {
      printf ("%s: no arguments on command line, quitting\n", argv [0]);
      fprintf (stderr, "%s: no arguments on command line, quitting\n", argv [0]);
    }
    Abort (__FILE__, __LINE__, -4);
  }

  job = host = "";
  input = cwd = NULL;
  nSleep = 0;

  i = 0;
  while (i < argc)
  {
    arg = argv [i];
//  if (Node == 0) printf ("arg %d = '%s'\n", i, arg);
    if (*arg == '-')
    {
      switch (*(arg+1))
      {
        case 'c': check = true;
            break;

        case 'd': i++;
                  cwd = argv [i];
            break;

        case 's': i++;
                  nSleep = atoi (argv [i]);
//                if (Node == 0) printf ("NodeInfo: read nSleep = %d\n", nSleep);
            break;
      }
    }
    else
      input = arg;
    i++;
  }

  if (cwd == NULL) cwd = getenv ("PWD");
  if (Node == 0) printf ("NodeInfo::NodeInfo: cwd = '%s'\n", cwd);

  if (input == NULL)
  {
    if (Node == 0)
    {
      printf ("%s: no input file given, quitting\n", argv [0]);
      fprintf (stderr, "%s: no input file given, quitting\n", argv [0]);
    }
    Abort (__FILE__, __LINE__, -5);
  }

/* Set the IONode */

//IONode = (nNodes - 1);   /* Make last node handle the I/O */
//IONode = 0;              /* Root node handles */

  IONode = nNodes / 2;     /* Middle node handles */
  if (Node == 0) printf ("IONode is %d of %d\n", IONode, nNodes);

  CellCount = SynapseCount = 0;

/* Allocate storage space for info about other nodes */

  buffer = (char *)  calloc ((nNodes + 1) * HOSTLEN, sizeof (char));
  host   = buffer + (nNodes * HOSTLEN);
  names  = (char **) calloc (nNodes, sizeof (char *));

  pids      = (int *) calloc (5 * nNodes, sizeof (int));
  nClusters = pids + nNodes;
  nCells    = pids + 2 * nNodes;
  firstCell = pids + 3 * nNodes;
  nSynapses = pids + 4 * nNodes;

  Bmips  = (double *) calloc (3 * nNodes, sizeof (double));
  weight  = Bmips  + nNodes;
  Aweight = weight + nNodes;


/* Now get host name and distribute it to all nodes */

  gethostname (host, HOSTLEN-1);

  //fprintf( stderr, "Host: %s\n", host );

  MPI_Allgather (host, HOSTLEN, MPI_CHAR, buffer, HOSTLEN, MPI_CHAR, MPI_COMM_WORLD);

  s = buffer;
  for (i = 0; i < nNodes; i++)
  {
    names [i] = s;
    s += HOSTLEN;
  }

/* Do the same for the process IDs */

  pid = getpid ();
  MPI_Allgather (&pid, 1, MPI_INT, pids, 1, MPI_INT, MPI_COMM_WORLD);

/* Get the node's bogomips rating and distribute it */

  bmip = 5000.0; //default value

  proc = fopen ("/proc/cpuinfo", "r");  //attempt to open file
  if( proc )
  {
    found = false;
    while (!found && (fgets (line, 255, proc) != NULL))
    {
      if (strncmp (line, "bogomips", 8) == 0)
      {
        s    = line + 9;
        stop = line + 90;
        while ((*s != ':') && (s < stop))
          s++;
        s++;
        sscanf (s, "%lf", &bmip);
        found = true;
      }
    }
    fclose (proc);
  }

  MPI_Allgather (&bmip, 1, MPI_DOUBLE, Bmips, 1, MPI_DOUBLE, MPI_COMM_WORLD);

  TotalBmips = 0.0;
  for (i = 0; i < nNodes; i++)
  {
//  if (Node == 0)
//    printf ("NodeInfo::NodeInfo: %3d: %-12s  %5d  %7.2f\n",
//            i, names [i], pids [i], Bmips [i]);
    TotalBmips += Bmips [i];
  }
  if (Node == 0)
    printf ("NodeInfo::NodeInfo: Total Bmips = %7.2f\n", TotalBmips);

/* Allocate the sending to and receiving from lists */

  nSend = nRecv = 0;
  Send = (int *) calloc (2 * nNodes, sizeof (int));
  Recv = Send + nNodes;

  MaxSynDelay = MAX_SYNDELAY;
}

NodeInfo::~NodeInfo ()
{
  free (buffer);
  free (names);
  free (pids);
  free (Bmips);
}

/*---------------------------------------------------------------------------*/
/* Print information on number of cells, synapses, etc.                      */

void NodeInfo::Print ()
{
  int i;

  if (Node == 0)
  {
    for (i = 0; i < nNodes; i++)
    {
      printf ("%4d %-16s %6.1f %8d %8d\n", i, names [i], Bmips [i], nCells [i],
              nSynapses [i]);
    }
    printf ("Total Cells = %10d, total synapses = %10d\n", CellCount, SynapseCount);
  }
}

/*---------------------------------------------------------------------------*/
//Save Load functions

int NodeInfo::Save( FILE *out )
{
  int nbytes = 0;
  int iHolder[5], i;

  //only write out cell/synapse counts
  //nSend/nRecv for each node

  i=0;
  iHolder[i++] = CellCount;
  iHolder[i++] = SynapseCount;
  iHolder[i++] = nSend;
  iHolder[i++] = nRecv;
  iHolder[i++] = MaxSynDelay;
  fwrite( iHolder, i, sizeof( int ), out );
    nbytes += i*sizeof( int );

  //send and recv are really on one array, so I only need 1 write
  fwrite( Send, sizeof( int ), nNodes*2, out );
    nbytes += sizeof( int ) * 2 * nNodes;

  return nbytes;
}

/*---------------------------------------------------------------------------*/

void NodeInfo::Load( FILE *in )
{
  const int iSize = 5;
  int iHolder[iSize], i;

  i=0;
  fread( iHolder, sizeof(int), iSize, in );
  CellCount = iHolder[i++];
  SynapseCount = iHolder[i++];
  nSend = iHolder[i++];
  nRecv = iHolder[i++];
  MaxSynDelay = iHolder[i++];

  //Send and Recv should already be allocated when the program first started
  fread( Send, sizeof( int ), nNodes*2, in );
}


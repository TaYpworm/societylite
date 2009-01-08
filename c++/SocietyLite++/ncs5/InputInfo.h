#ifndef INPUTINFO_H
#define INPUTINFO_H

/* Defines a struct used to keep track of input-related infos.  It hides the */
/* details of input from the Stimulus object, which just contains a pointer  */
/* to it.                                                                    */


#include <stdio.h>
#include <mpi.h>

typedef struct _InputInfo /* One of these on each node for each report */
{
  bool InComm;          /* True if report gets called on this node */

  int IONode;           /* rank in Comm of the node that handles I/O for this report */
  MPI_Comm Comm;        /* Communicator for nodes having parts of this report */
  MPI_Group Group;             
  FILE *in;
  int port;             /* Socket file descriptor, if > 0, output to port as produced */

  int nInGroup;         /* number of nodes in the group: either nWithData or nWithData + 1 */
  int nWithData;        /* number of nodes that have data for the report */
  int nBytes;           /* Total bytes of data read per input timestep */
  int *nodes;           /* list of nodes that will get data */
  int *Used;            /* flag, if > 0, node will get data */
} INPUTINFO;

#endif

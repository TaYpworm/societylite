/* This handles gathering input from some source, and delivering it to the   */
/* Stimulus objects on various nodes.  It is more or less the inverse of the */
/* Transmit/Report module.                                                   */

/* On startup, it creates a separate communicator over which control         */
/* messages will pass, to ensure that they won't interfere with other brain  */
/* messages.) To reduce ethernet contention., one node (IONode) is           */
/* designated to read the data from the outside world and distribute it over */
/* MPI/Myrinet to the nodes.  7/18/02 - JF                                   */

#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "Input.h"
#include "Port.h"
#include "Managers.h"
#include "debug.h"
#include "memstat.h"

#define MEM_KEY  KEY_INPUT

Input::Input ()
{
  MEMADDOBJ (MEM_KEY);

/* Create a separate communicator for Input messages */

  MPI_Comm_dup (MPI_COMM_WORLD, &InputComm);
  MPI_Comm_group (MPI_COMM_WORLD, &Group);
}

/*------------------------------------------------------------------------*/

Input::~Input ()
{
  MEMFREEOBJ (MEM_KEY);
}

/*---------------------------------------------------------------------------*/
/* Initialize an input.  Every node must call this function (from the        */
/* Stimulus constructor) when a new input Stimulus is added, whether or not  */
/* the stimulus will be received by cells on that node.  The routine gathers */
/* information about whether an input is received on each node, and builds a */
/* communicator containing those nodes, plus the IONode if it's not already  */
/* included.  It then opens the source file or port.                         */

void *Input::MakeInput (char *name, int port, int nbytes)
{
  INPUTINFO *II;
  double tstart;
  int i;
  bool ioInList;

  printf ("Input::MakeInput: nbytes = %d\n", nbytes);

  tstart = MPI_Wtime ();

  II = (INPUTINFO *) malloc (sizeof (INPUTINFO));
  II->nodes = (int *) calloc (2 * NI->nNodes, sizeof (int));
  II->Used  = II->nodes + NI->nNodes;
  II->in    = NULL;
  printf ("Input::MakeInput: nNodes = %d, nbytes = %d\n", NI->nNodes, nbytes);

//printf ("Input::MakeInput: calling allgather\n");
  MPI_Allgather (&nbytes, 1, MPI_INT, II->Used, 1, MPI_INT, InputComm);
//printf ("Input::MakeInput: allgather done\n");

  II->nInGroup = II->nWithData = II->nBytes = 0;

  printf ("Input::MakeInput: IONode = %d\n", NI->IONode);
  ioInList = false;
  for (i = 0; i < NI->nNodes; i++)
  {
//  printf ("Input::MakeInput: loop, node = %d, used = %d\n", i, II->Used [i]);
    if (II->Used [i] > 0)
    {
      II->nodes [II->nWithData] = i;
      if (II->Used [i] > II->nBytes)  II->nBytes = II->Used [i];

      if (i == NI->IONode)
      {
        II->IONode = II->nWithData;
        ioInList = true;
      }
      II->nWithData++;
    }
  }

  II->nInGroup = II->nWithData;

  if (!ioInList)
  {
    II->nodes [II->nInGroup] = NI->IONode;
    II->nInGroup++;
  }

//printf ("Input::MakeInput: II->nInGroup = %d\n", II->nInGroup);
//for (i = 0; i < II->nInGroup; i++)
//  printf ("Input::MakeInput: node %d = %d\n", i, II->nodes [i]);


  MPI_Group_incl (Group, II->nInGroup, II->nodes, &(II->Group));

  MPI_Comm_create (InputComm, II->Group, &(II->Comm));

  if (II->Comm == MPI_COMM_NULL)      /* This node is not in communicator */
  {
    II->InComm = false;
    free (II->nodes);
    II->nodes  = NULL;
    free (II);
    II = NULL;
  }
  else
  {
    II->InComm = true;
  
    if (NI->Node == NI->IONode)
    {
      printf ("Input::MakeInput: nInGroup = %d, nWithData = %d\n", II->nInGroup, II->nWithData);
      for (i = 0; i < II->nWithData; i++)
        printf ("Input::MakeInput:   %d: node %d, used = %d\n", i, II->nodes [i], II->Used [i]);
    }
  
    if (NI->Node == NI->IONode)
    {
      if (port > 0)
        II->in = OpenPortFile (name, port, "r");
      else if( port == -1 )
        II->in = getAutoPort( name );
      else
        II->in = fopen (name, "r");
      printf ("Input::MakeInput: Opened input source '%s:%d'\n", name, port);
    }
  }

  return (II);
}

/*---------------------------------------------------------------------------*/
/* Free a report block                                                       */

void Input::FreeInput (void *ptr)
{
  INPUTINFO *II;

  II = (INPUTINFO *) ptr;
  if (II->in != NULL) fclose (II->in);
  if (II->nodes != NULL) free (II->nodes);
  if (II->Comm  != MPI_COMM_NULL)  MPI_Comm_free (&(II->Comm));
  if (II->Group != MPI_GROUP_NULL) MPI_Group_free (&(II->Group));
  free (II);
}

/*---------------------------------------------------------------------------*/
/* The IONode reads a block of data from the input source, and distributes   */
/* it to all the nodes that use it.  Unlike reporting, all the data is sent  */
/* to every node - there isn't that much of it at any timestep, and it makes */
/* the code simpler.                                                         */

/* It's assumed that the necessary data is waiting to be read.  If not, the  */
/* read should return immediately, and fill the input buffer with zeros.     */
/* This matches the real-world, where "nothing" is a valid sensory input.    */


void Input::Read (void *ptr, void *data)
{
  INPUTINFO *II;
  float *fp;
  int i, nread;

  II = (INPUTINFO *) ptr;
  if (NI->Node == NI->IONode)
  {
    nread = fread (data, 1, II->nBytes, II->in);
  }
  if (nread == 0)
  {
    fp = (float *) data;
    for (i = 0; i < II->nBytes / sizeof (float); i++)
      fp [i] = 0.0;
  }

  MPI_Bcast (data, II->nBytes, MPI_BYTE, II->IONode, II->Comm);
}

//---------------------------------------------------------------------

FILE *Input::getAutoPort( char *name )
{
  if( !TheBrain->HostName ) //make sure a server name exists
    return NULL;

  int sd = OpenPort( TheBrain->HostName, TheBrain->HostPort );
  char *label = NULL;
  int length;

  if( sd<= 0 ) //error
    return NULL;

  //send keyword
  write( sd, "request\n", 8 );

  //does this change name?
  char *baseName = NULL;
  if( (baseName = strstr( name, NI->job )) )
  {
    baseName += strlen( NI->job ) + 1;
  }
  else
    baseName = name;

  //build name from Brain->L.name, Brain->job, Stim->L.name
  length = strlen( NI->AR->Brain->L.name ) + strlen( NI->job ) + strlen( baseName )+1;
  length += strlen( "read" ) +1;
  length += strlen( "binary" ) +1;

  label = new char[ length+1 ];
  sprintf( label, "read\nbinary\n%s%s%s\n", NI->AR->Brain->L.name, NI->job, baseName );

  //send name
  write( sd, label, length );

  if( label ) delete [] label;

  return fdopen( sd, "r" );
}


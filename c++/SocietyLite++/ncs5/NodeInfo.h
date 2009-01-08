#ifndef NODEINFO_H
#define NODEINFO_H

//forward declaration
struct ARRAYS;

//savefunctions for the ARRAYS struct
#include "SaveStruct.h"
#include "LoadStruct.h"

//@Include: SaveStruct.h
//@Include: LoadStruct.h
//@Include: InitStruct.h

/**
 * This holds a collection of information mostly related to the capabilities
 * of the various nodes the program is running on, used for load balancing,
 * program control, etc.  Since some of this needs to be available to almost
 * every routine, various other things that have no obvious home have been
 * put in here too.
 */
class NodeInfo
{
  public:
          ///If this NCS simulation communicates with a central server, this is the hostname
  char *Server;
          ///The control port of the central server if this NCS simulation has contact to one
  int Port;
          ///MPI rank in the parallel job
  int Node;
          ///Number of nodes in the job
  int nNodes;
          ///Node that handles I/O for all the nodes
  int IONode;
          ///The process id for this node
  int pid;
          ///Total number of cells in the brain (not just this node)
  int CellCount;
          ///Total number of synapses in the brain (not just this node)
  int SynapseCount;     
          ///Total compute power of all the nodes
  double TotalBmips;

/* These have one entry for every node */
          ///Name of each node (value of host on node)
  char *buffer;
          ///Name of each node (value of host on node)
  char **names;
          ///Process id from each node
  int *pids;
          ///Number of clusters on each node
  int *nClusters;
          ///Number of cells on each node
  int *nCells;
          ///Index into GCList, etc of first cell on node
  int *firstCell;
          ///Number of Synapses (receiving side) on each node
  int *nSynapses;
          ///Compute power of each node
  double *Bmips;
          ///Share of weight that should be assigned to each node
  double *weight;
          ///Actual weight assigned to each node after distribution
  double *Aweight;
          ///Number of nodes that this node send messages to
  int nSend;
          ///Number of nodes that this node receives messages from
  int nRecv;
          ///The number of synapses sending to each node
  int *Send;
          ///The number of synapses receiving from each node
  int *Recv;
          ///The maximum synaptic delay on this node
  int MaxSynDelay;

/* This is miscellaneous stuff put here for neatness */

          ///Name of this node
  char *host;
          ///Working directory - aquired from command-line arguments
  char *cwd;
          ///Job name, taken from input
  char *job;
          ///Frequency, ticks per second
  int FSV;
          ///Pointer to initial objects created after parsing input file
  ARRAYS *AR;
          ///Pointer to input file name (one of argv)
  char *input;
          ///For debug use
  int nSleep;
          ///Time the program was started (from MPI_Wtime)
  double tstart;

/* debug & trace flags */

          ///if true, run distribution check only
  bool check;
          ///if true, print report of connections
  bool ConnectRpt;
          ///if true, print report of spikes, mem usage, &c at end of each timestep
  bool SpikeRpt;

  //-------------------------------------------------------------

  /**
   * Constructor. Parses arguments and allocates memory 
   */
  NodeInfo (int, int, int, char *[]);
  ~NodeInfo ();
  void Print ();

  /**
   * Save important information to a savefile.  This data is node specific, so each Node must
   * save its own copy
   * @param out File Pointer to write data to
   * @return number of bytes written
   */
  int Save( FILE *out );

  /**
   * Aquire important data from a file.  Not all data was saved, since new data will be 
   * filled in when a new run is started.
   * @param in File Pointer to obtain data from
   */
  void Load( FILE *in );

};
#endif 


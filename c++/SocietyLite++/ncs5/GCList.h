#ifndef GCLIST_H
#define GCLIST_H

/* These structures define the Global Cluster list and the connection        */
/* descriptors.  A CLUSTER struct has information about which node each      */
/* cluster is on, what cells are in it, and what other clusters it connects  */
/* to.  The GCList has a CLUSTER struc for each cluster of cells in the      */
/* brain.  There is an identical copy of the GCList on each node.            */

/* It also holds a list of connection descriptors (CONNECT struct), each of  */
/* which holds info about a connection between two clusters.  Each CONNECT   */
/* appears in the lists twice, once at the send end, and once at the         */
/* receiving end.                                                            */

/* The GCList is created and owned by CellManager.cpp.  Other routines will  */
/* access it (generally through CellManager functions) to obtain lists of    */
/* cells meeting specified criteria, etc.                                    */


#include "InitStruct.h"

/**
 * Structure containing information regarding a specific group of cells used
 * during simulation creation.
 */
struct CLUSTER
{
        ///Index to the Column definition in the ARRAYS object
  int iCol;
        ///Index to the Layer definition in the ARRAYS object
  int iLay;
        ///ndex to the Cell definition in the ARRAYS objects
  int iCel;
        ///ID of first cell in cluster - index in node's cell list
  int idx;
        ///Number of cells in cluster
  int Qty;
        ///Number of outgoing synapses for cluster
  int nSendTo;
        ///number of incoming synapses for cluster
  int nSyn;
        ///Node this cell cluster is assigned to
  int Node;
        ///Total compute weight for cluster
  double weight;
        ///Cluster position: x coordinate
  double x;
        ///Cluster position: y coordinate
  double y;
        ///Cluster position: z coordinate
  double z;
        ///Index of this cluster in cluster list.  Needed because
  int iSelf;
        ///List of connections made using this cluster
  struct CONNECT **CDList;
};
typedef struct CLUSTER CLUSTER;

#include "InitStruct.h"

/**
 * This structure is the Connection Descriptor, which holds info for a
 * connection between two clusters.
 */
struct CONNECT
{
        ///Cluster where synapses start
  CLUSTER *From;
        ///Cluster where synapses finish (will own the synapse)
  CLUSTER *To;
        ///Number of connections to make
  int nCon;
        ///Seed to use when generating connections
  int Seed;
        ///Compartment within source cell to use during connections
  int FromCmp;
        ///Compartment within destination cell to create the synapse on
  int ToCmp;
        ///Pointer to decriptor to use in making connections */
  T_CONNECT *iCon;
};
typedef struct CONNECT CONNECT;

#endif

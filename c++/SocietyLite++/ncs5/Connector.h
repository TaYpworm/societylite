#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mpi.h>

#include <vector>

#include "Managers.h"
#include "RandomManager.h"
#include "parse/arrays.h"
#include "Memory.h"

/**@name Connector
 * Functions for creating synapses between cells.
 */
//@{

/**
 * To establish each connection, the destination cell must send four pieces
 * of data to the FROM node: The ids of the FROM cell and cmp, the memory address
 * of the Synapse's ReceiveMsg function on the TO node, and the time delay (in timesteps).
 */
struct CONNECTMSG //typedef struct _ConnectMsg
{
        ///Index into the source node's global cell list
  int FromCell;
        ///Index into the source cell's compartment list
  int FromCmp;
        ///Memory address of the synapse on the destination node
  void *Addr;
        ///Number of timesteps that must elapse before the synapse firing can arrive
  int Delay;
};
typedef struct CONNECTMSG CONNECTMSG;

/**
 * This struct temporarily holds the information for each cell.  The use of
 * vectors allow collection of an arbitrary number of connections: when all
 * the connections have been determined, the vectors will be converted to
 * plain arrays for speed & space efficiency.                                
 */
struct TMPCMP
{
        ///Object on receiving side
  vector <Synapse *> Syn;
        ///Receiving node
  vector <int> Node;
        ///Memory Address on sending side
  vector <void *> Addr;
        ///delay on sending side
  vector <int> Delay;
};
typedef struct TMPCMP TMPCMP;

/**
 * Temporary container object to hold an array of temporary compartment information.
 */
struct TMPCELL
{
        ///Pointer to create a dynamic array of temporary Compartments
  TMPCMP *Cmp;
};
typedef struct TMPCELL TMPCELL;

//function prototypes and documentation

/**
 * This function is used when the probability of connection stays constant
 * (that is, it does not decay with distance).  This will make connections
 * so that the final number of connections made matches the estimate. For 
 * example, if 10 cells are connected to 10 cells, but only with 0.8
 * probability, then the final count should be 80 synapses.
 *
 * @param CD Connection Description contains information regarding source and destination
 * @param Map Matrix of connections to be filled in as connections are made. 1 means connection, 0 means no.
 * @param nMade Number of Connections made when all probabilities have been thrown.
 * @param CMBuf Data about the connections will be stored here, so that later it can be communicated to the Sending Node
 * @param CelIDs Destination Cell IDs that get connected will be stored here for quick lookup
 * @param CmpIDs Destination Compartment Ids that get connected will be stored here for quick lookup (although, shouldn't it always be the same?)
 */
static void MakeConnects (CONNECT *CD, char *Map, int *nMade, CONNECTMSG *CMBuf,
                   int *CelIDs, int *CmpIDs);

/**
 * After synapse connections are determined, the sending node (i.e. the "From" node)
 * acquires infomation from the receiving node (i.e. the "To" node).  This includes telling
 * the sender which of its cells are to become sources (Cell and Cmp),
 * the memory addresses to send spikes to on the receiving node, and the time delay
 * (because more time delay means the spike can be sent via MPI at a more convenient time)
 *
 * @param num Number of connections made between the two cell clusters
 * @param node ID of the Sending Node
 * @param SynIdx not used
 * @param CMBuf Pointer to array of Connection Messages which contain the memory address and time delay of the synapses
 */
static void AddToSendTo (int num, int node, int SynIdx, CONNECTMSG *CMBuf);

/**
 * This function is for connections that depend on distances.  It handles two
 * sub-types of connection probabilities: Decaying Probabilities and normal
 * Probabilities.  In the case of decaying probabilities, as the distance between
 * two cells increase, the probability of connection decreases.  In the case of
 * normal probabilities, the probability stays constant.  Both cases also use
 * distance to modify synaptic delay based on the speed of the synapse type.
 * 
 * @param CD Connection Description contains information regarding source and destination
 * @param Map Matrix of connections to be filled in as connections are made. 1=connected, 0=no.
 * @param nMade Number of Connections made when all probabilities have been thrown.
 * @param CMBuf Data about the connections will be stored here, so that later it can be communicated to the Sending Node
 * @param CelIDs Cell IDs that get connected will be stored here for quick lookup
 * @param CmpIDs Compartment Ids that get connected will be stored here for quick lookup (although, shouldn't it always be the same?)
 * @param src Pointer to the global Cell coordinates of the destination Node. If that Node is different from the source Node, the coordinate data must have been received previously.
 * @param nSrcCells Total nnumber of Global Cells defined on the destination Node.
 * @param distanceMap Matrix which will contain the final result of distance caluculation modified by the synapse speed. Rename delayMap?
 */
void MakeConnects (CONNECT *CD, char *Map, int *nMade, CONNECTMSG *CMBuf, 
                   int *CelIDs, int *CmpIDs, double *src, int nSrcCells,
                   double *dest, int nDestCells, double *distanceMap, int transpose=0 );

/**
 * Find the x,y,z coordinates of a source and destination cell and calculate
 * the distance between them.
 *
 * @param sIndex The source cell's index
 * @param source Pointer to array of cell coordinates. All x,y,z coordinates are in one array
 * @param qty1 The number of global cells on the node where the source cluster origniated from. Used to navigate the corrdinate array.
 * @param dIndex The destination cell's index
 * @param destination Pointer to array of cell coordinates. All x,y,z coordinates are in one array
 * @param qty2 The number of global cells on the node where the destination cluster origniated from. Used to navigate the corrdinate array.
 */
inline double distance( int sIndex, double *source, int qty1, int dIndex, double *destination, int qty2 );

/**
 * After connections have been made, the connected cell IDs (and compartment IDs)
 * will be stored continuously into one array for quick reference
 *
 * @param CD Connection Description contains information regarding source and destination
 * @param Map Matrix of connections: 1 = cell i&j are connected, 0 mean no
 * @param CMBuf Data about the connections will be stored here, so that later it can be communicated to the Sending Node
 * @param CelIDs Destination Cell IDs that get connected will be stored here for quick lookup
 * @param CmpIDs Destination Compartment Ids that get connected will be stored here for quick lookup (although, shouldn't it always be the same Compartment ID?)
 */
void gatherConnectedIDs( CONNECT *CD, char *Map, CONNECTMSG *CMBuf, int *CelIDs, int *CmpIDs );

/**
 * Output info about a single connection map.  Will output names of
 * clusters involved (column, layer, cell, compartment), synapse, and a
 * matrix of the connections with the source cluster forming the matrix
 * columns and the destination cluster forming the matrix rows.
 *
 * @param CD Connection Description contains information regarding source and destination
 * @param Map Represents present connections with a '1' and absent connections with a '0'
 * @param fout Destination file pointer
 */
void outputConnectMap( CONNECT *CD, char *Map, FILE *fout, int reversed=0 );

/**
 * For recurrent connections: When the original connection (cluster A to cluster B)
 * is made, the reverse connections (cluster B to cluster A) are made at the same time.
 * This allows for the connection maps to be coordinated on one node.  If only one node
 * is used, it can procede to create the synapses in function createSynapses.  If
 * two nodes are used, after the maps have been coordinated, node A will send the second
 * map to node B so that it can create the synapses.
 *
 * @param CD Connection Description contains information regarding source and destination
 * @param Map original connection map connecting cluster a to cluster b
 * @param nMade Number of connections made
 * @param CMBuf Data about the connections will be stored here
 * @param CellIDs Destination Cell IDs that get connected will be stored here if only one node
 * @param CmpIDs Destination Compartment Ids that get connected will be stored here for quick lookup (although, shouldn't it always be the same Compartment ID?)
 * @param src Pointer to the global Cell coordinates of the destination Node. If that Node is different from the source Node, the coordinate data must have been received previously.
 * @param nSrcCells Total nnumber of Global Cells defined on the destination Node.
 * @param distanceMap Matrix which will contain the final result of distance caluculation modified by the synapse speed.
 * @param tCel Storage location of connection info for each Cell if one node
 * @param ConComm MPI Communication object
 * @param status MPI Status is stored here after a receive
 * @param MsgType Special Message Type created for the transmission of CONNECTMSG objects
 */
void MakeRecurrentConnects( CONNECT *CD, char *Map, int nMade, CONNECTMSG *CMBuf,
                            int *CellIDs, int *CmpIDs, double *src, int nSrcCells,
                            double *dest, int nDestCells, double *distanceMap, TMPCELL *tCel,
                            MPI_Comm &ConComm, MPI_Status &status, MPI_Datatype &MsgType,
                            FILE *fout );

/**
 * For recurrent connections: This function allows for one Connection Descriptor
 * to handle two different connections by exchanging information.  In a recurrent
 * connection, one T_CONNECT stores infomation about both connections, while the
 * other T_CONNECT was disabled.  This ensures that the connections are made
 * simultaneously.  This function merely swaps the information so that the same
 * functions can be used when connecting up cluster B to cluster A as were used
 * when connecting up cluster A to cluster B.
 *
 * @param CD Connection Description contains information regarding source and destination
 */
void swapInfo( CONNECT *CD );

/**
 * For recurrent connections: This function is called by the node B so that it
 * may receive the secondary connection map created by node A.  This function is
 * only called if the clusters are on two different nodes.  Once the map has been
 * received, the synapses can be created in the createSynapses function.
 *
 * @param CD Connection Description contains information regarding source and destination
 * @param map Space to store the Matrix of connections that will be received
 * @param distanceMap Space to store Matrix of distances between connected cells if option is turned on
 * @param tCel Storage location of connection info for each Cell
 * @param ConComm MPI Communication object
 * @param status MPI Status is stored here after a receive
 * @param MsgType Special Message Type created for the transmission of CONNECTMSG objects
 */
void acceptRecurrentConnection( CONNECT *CD, char *map, double* distanceMap, TMPCELL *tCel,
                                MPI_Comm &ConComm, MPI_Status &status, MPI_Datatype &MsgType, FILE *fout );

/**
 * For recurrent connections: general purpose synapse creator for when clusters
 * are on one or two nodes.  Node B calls this function to create the synapses.
 * If node A and node B are the same node, the newly  generated synapse info will
 * be directly added; otherwise, it will be communicated to the correct node.
 *
 * @param CD Connection Description contains information regarding source and destination
 * @param map Matrix of connections that will be created
 * @param distanceMap Matrix of distances between connected cells
 * @param nMade Number of connections made between clusters
 * @param tCel Storage location of connection info for each Cell
 * @param ConComm MPI Communication object
 * @param status MPI Status is stored here after a receive
 * @param MsgType Special Message Type created for the transmission of CONNECTMSG objects
 */
void createSynapses( CONNECT *CD, char *map, double *distanceMap, int nMade, TMPCELL *tCel,
                     MPI_Comm &ConComm, MPI_Status &status, MPI_Datatype &MsgType );

/**
 * For recurrent connections: If the clusters were on different nodes, node A
 * calls this function to inform node B about the connection scheme.  That is,
 * it sends the connection map (and distance map if applicable) so that node B
 * can use that to generate synapses on the appropriate compartments.
 * 
 * @param CD Connection Description contains information regarding source and destination
 * @param MapBtoA Matrix of connections that will be created
 * @param distanceMap Matrix of distances between connected cells
 * @param ConComm MPI Communication object
 * @param status MPI Status is stored here after a receive
 */
void sendRecurrentInfo( CONNECT *CD, char *MapBtoA, double* distanceMap, MPI_Comm &ConComm, MPI_Status &status );

/**
 * For recurrent connections: If the clusters were on different nodes, node A
 * calls this function to retrieve information about the synapses created on node B.
 * 
 * @param CD Connection Description contains information regarding source and destination
 * @param nMade The number of connections made when connection cluster B to cluster A
 * @param CMBuf Data about the connections will be stored here as it is read from node B
 * @param ConComm MPI Communication object
 * @param status MPI Status is stored here after a receive
 * @param MsgType Special Message Type created for the transmission of CONNECTMSG objects
 */
void acceptDestinationInfo( CONNECT* CD, int nMade, CONNECTMSG *CMBuf,
                            MPI_Comm &ConComm, MPI_Status &status, MPI_Datatype &MsgType );

/**
 * For recurrent connections: After a list (vector) of candidate connections have
 * been created, they are shuffled so that they can be pulled iteratively from
 * the list, while maintaining randomness.  This version is for a group that
 * is recurrent with itself, so it will use one list to create new connections,
 * as well as steal from.
 * 
 * @param nRecurrent The number of remaining recurrent connections that must be made
 * @param target Collection of potential connections to create or steal
 */
void shuffleConnections( int nRecurrent, vector<int> &target );

/**
 * For recurrent connections: After lists (vectors) of candidate connections have
 * been created, they are shuffled so that they can be pulled iteratively from
 * the list, while maintaining randomness.  This version is two unique cell groups
 * becoming recurrent with each other, so one list is for potatial connections to 
 * form, while the other list is poential connections to steal
 *
 * @param nRecurrent The number of remaining recurrent connections that must be made
 * @param target Collection of potential connections to create
 * @param steal  Collection of potential connections to steal
 */
void shuffleConnections( int nRecurrent, vector<int> &target, vector<int> &steal );

/**
 * For recurrent connections: after the maps have been coordinated, the second map must be
 * trasposed so the source cells form the columns and destination cells form the rows
 *
 * @param nMap The number of elements in the matrix
 * @param nTo  The number of destination cells
 * @param nFrom The number of source cells
 * @param map  The connection map
 * @param distanceMap The distances between connected cells
 */
void transposeMap( int nMap, int nTo, int nFrom, char *map, double *distanceMap );

/**
 * For a cell group recurrent with itself, all operations work within the
 * same matrix.
 *
 * @param CD Connection Description contains information regarding source and destination
 * @param Map Original randomly generated connections, will be modified 
 * @param nMade The number of connections made
 * @param CMBuf Data about the connections will be updated after modification
 * @param CellIDs Destination Cell IDs that get connected will be stored here
 * @param CmpIDs Destination Compartment Ids that get connected will be stored here
 * @param distanceMap Matrix for the final result of distance caluculation modified by the synapse speed.
 */
void MakeSelfRecurrence( CONNECT *CD, char *Map, int nMade, CONNECTMSG *CMBuf,
                            int *CellIDs, int *CmpIDs, double *distanceMap );

/**
 * For recurrent connections: coordinate two connections maps to adjust the number of recurrent connections.
 * The MapBtoA will have new connections created and existing connections removed until the percentage of
 * recurrent connections meets the value set in CD.  This function will be called twice, with the Map parameters
 * swapped on the second calling.
 *
 * @param CD Connection Description contains information regarding source and destination
 * @param MapAtoB Connections map for synapses going one-way
 * @param MapBtoA Connections map for synapses going the reverse direction
 * @param nMade The number of synapses made in MapAtoB (a percentage of these will be made recurrent)
 * @param distanceMapAtoB Distances of synapses going one-way (modified by synapse speed)
 * @param distanceMapAtoB Distances of synapses going the reverse direction (modified by synapse speed)
 * @param direction Should be zero on first call, one on second call - determines which recurrence percentage to use in CD
 */
void CoordinateConnectionsOneWay( CONNECT *CD, char *MapAtoB, char *MapBtoA, int nMade,
                                  double *distanceMapAtoB, double *distanceMapBtoA, int direction );

//@}


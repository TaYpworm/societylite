#ifndef MESSAGEBUS_H
#define MESSAGEBUS_H

#include <mpi.h>

#include "Message.h"
#include "SendList.h"
#include "Packet.h"

/**
 * This defines the organization of the message sending buffers.  There is
 * an array of buffer pointers, with an entry for each node.  Only the nodes
 * this node sends to actually point to a real buffer.  The others just have
 * the array entry set to NULL.  This is so that selecting the proper buffer
 * for a message means just using the destination node as an array index.
 *
 * The SendMsg routine will put its message into the buffer the array
 * currently points to.  If that fills it, the buffer is sent with
 * MPI_Isend, the buffer pointer moved to the busy buffer list (ring
 * buffer?) and a new buffer gotten from the free buffer list.  If no free
 * ones are left, MPI_Wait is called on the requests in the busy buffer
 * list, to clear any pending sends and thus free the buffers.
 *
 * The buffer encapsulates both the information to be sent (in PACKET), and
 * information about the status of the send.  Note trickiness with request:
 * MPI says it's an opaque type that can be assigned & compared.  Requests
 * array has to be in order of buffers for MPI_Waitsome and send/busy/free
 * scheme to work...
 */
class MessageBus
{ 
 public:

/* SendList and BusyList are arrays of PACKET pointers.  There is a pointer  */
/* for each node (including this node) so messages can easily be indexed by  */
/* node, but if this node doesn't send to or receive from a particular node, */
/* the corresponding pointer will be NULL.                                   */

        ///File pointer for sending logged information
  FILE *msgFP;
        ///Logged data: Number of spikes sent
  int nSpikes;

        ///Number of nodes used by the simulation
  int nNodes;
  
        ///Not used
  int nBusy;
        ///Not used
  int maxBusy;
        ///Not used
  PACKET **BusyList;
        ///Not used - Array of request structs for msgs
  MPI_Request *BusyReq;
        ///Not used - Array of status structs for waiting on busy msgs
  MPI_Status  *BusyStat;
        ///Not used - For MPI_Waitsome to return indices in
  int *iDone;

/* For testing, counts of messages & packets sent & received */

        ///Count of messages sent: for testing
  int msgSent;
        ///Count of messages received: for testing
  int msgRecv;
        ///Count of packets sent: for testing
  int pktSent;
        ///Count of packets received: for testing
  int pktRecv;


/* SendTo and RecvFrom are integer lists of the nodes that are actually      */
/* being sent to and received from, so that nodes this node doesn't          */
/* communicate with can be skipped.                                          */

        ///Number of other nodes this MessageBus sends data to
  int nSend;
        ///Array of pointers to packets.  Only nodes communicated with will have actual packets
  PACKET **SendList;
        ///Array containing the actual ids of nodes sent to
  int *SendTo;
        ///Array containing MPI_Requests (set to MPI_REQUEST_NULL)
  MPI_Request *SendReq;

        ///Number of other nodes this MessageBus receives data from
  int nRecv;
        ///Array containing the actual ids of nodes received from
  int *RecvFrom;


/* Variables for incoming message buffer */

          ///Current slot in incoming message ring buffer that the brain will take the messages from
  int SlotNow;
          ///Number of slots in the incoming message ring buffer (twice the maximum synaptic delay)
  int nSlots;
          ///Array of Buffers where each slot is an upcoming timestep, so that all messages for a timestep form a list
  Message **MsgList;
          ///Storage area for newly arrived packets.  Packets are sorted based on the time of the latest message
  PACKET **PendList;

/* Variables for the synchronization flags */

  int *NodeTime;
  
/* Functions defined for module */
  
    /**
     * Constructor builds and initializes structures used for sending and receiving.
     */
  MessageBus (); 
    /**
     * Destructor
     */
  ~MessageBus (); 

    /**
     * Called by Compartment to send spike messages.  It gets a SENDITEM list 
     * and count, to avoid overhead of repeated calls. 
     * @param TimeStep Current Timestep of the simulation
     * @param delay The number of timesteps between creating of the spike and actual delivery time
     * @param nTo The number of Compartments that will receive this spike message
     * @param ToList Information about the destination compartments
     */
  void SendSpikeMsgs (int TimeStep, int delay, int nTo, SENDITEM *ToList );
  
    /**
     * Called by Stimulus routines.  It sends messages only to this node
     * @param TimeStep Current TimeStep of the simulation
     * @param nTo Number of compartents that receive this stimulus
     * @param MsgType Whether the message is Current (I) or Voltage Clamp (V)
     @ @param Value The amount of current (I) or voltage (V)
     */
  void SendStimMsg (int TimeStep, int nTo, char **ToList, int MsgType, double Value);
  
    /**
     * Called by Send*Msg and FlushMsgs - Calls MPI to send packet to other
     * node, or if same node, links packet into PendList 
     * @param Pkt Pointer to Packet to be sent
     * @param ToNode ID of target node
     * @param TimeStep The current TimeStep of the simulation
     */
  void SendPacket (PACKET *Pkt, int ToNode, int TimeStep);
  
    /**
     * This routine receives any MPI messages that may be waiting It uses either
     * MPI_Probe or MPI_Iprobe (depending on wait argument), so it can either
     * return immediately if nothing is pending, or block until something is
     * ready to be received (to force synchronization).
     * @param timestep The current timestep of the simulation
     * @param wait Indicates if the message bus should block until it receives messages
     * @return boolean to indicate if any messages are present (true)
     */
  bool ReceiveMsgs (int timestep, bool wait);
  
    /**
     * Alfter receiving a packet filled with messages, the individual messages are inspected
     * for their delivery time and placed in the appropriate slot of the MsgList
     * @param Pkt Packet filled with freshly delivered messages
     * @param TimeStep Current TimeStep of the simulation
     */
  void SlotMsgs (PACKET *Pkt, int TimeStep);
  
    /**
     * Called by Brain to distribute the current timestep's messages to their
     * destinations.  First, any messages still in need of sending off-node are
     * sent, then messages that local structures need at this timestep are given
     * to the appropriate location (Stimulus and Spike messages given to compartments).
     * @param TimeStep Current TimeStep of the simulations
     */
  void DeliverMsgs (int TimeStep);
  
    /**
     * This is called (from Brain::DoThink) after cells have done their
     * processing, but before reporting.  It sends any remaining messages
     * from the current timestep, or sends an empty packet to each node that
     * receives messages from this node.  This packet has the synchronization
     * flag set.
     * @param TimeStep The simulation's current Timestep
     */
  void FlushMsgs (int TimeStep);
  
    /**
     * This is called from Brain::DoThink at the end of each timestep.  It
     * checks to see if all nodes that this node receives from have also
     * finished this timestep.  If not, it checks for incoming messages until
     * they have.  Also, returns empty packets to the Message Manager
     * @param TimeStep Current TimeStep of the simulation
     */
  void Sync (int TimeStep);
  
    /**
     * Helper function for Sync, this function verifies that each other node
     * this receives from has advanced far enough to allow this node to continue.
     * @param TimeStep Current TimeStep of the simulation
     */
  bool Synched (int TimeStep );
  
    ///Not Used
  void IncreaseBusy ();

    ///Prints information about a packet and other packets stored in its linked list
  void PrintPktList (PACKET *);
    ///Prints information about a message and other messages stored in its linked list
  void PrintMsgList (Message *);
};

#endif 

/* This module handles message passing & synchronization.  See comments at   */
/* end of file.                                                              */

#include <stdlib.h>
#include <stdio.h>

#include "MessageBus.h"
#include "Managers.h"

#include "debug.h"
#include "memstat.h"
#include "QQ.h"

#define MEM_KEY  KEY_MESSAGEBUS

int QQSync, QQmsgSent, QQpktSent, QQmsgRecv, QQpktRecv;

extern int *gdbptr;

MessageBus::MessageBus ()
{
  PACKET *Pkt;
  int i;
  char buf [128];
                                       
//printf ("MessageBus::MessageBus\n");
//QQDeliverMsgs  = QQAddState ("MessageBus::DeliverMsgs");
//QQFlushMsgs    = QQAddState ("MessageBus::FlushMsgs");
//QQSync         = QQAddState ("MessageBus::Sync");

/* For testing, counts of messages & packets sent & received */

  msgSent = msgRecv = pktSent = pktRecv = nSpikes = 0;
  if (NI->Node == 0)
  {
    sprintf (buf, "msg.%3.3d", NI->Node);
//  msgFP = fopen (buf, "w");
  }

/* Data structures for sending */

  nSend    = nRecv = 0;
  SendList = (PACKET **) calloc (NI->nNodes, sizeof (PACKET *));
  SendTo   = (int *)     calloc (NI->nNodes, sizeof (int));
  SendReq  = (MPI_Request *) calloc (NI->nNodes, sizeof (MPI_Request));

/* Data structures for receiving & synchronization */

  RecvFrom = (int *) calloc (NI->nNodes, sizeof (int));
  NodeTime = (int *) calloc (NI->nNodes, sizeof (int));

/* Initialise the send & receive lists.  */

  for (i = 0; i < NI->nNodes; i++)
  {
    if ((NI->Send [i] > 0) || (NI->Recv [i] > 0) || (i == NI->Node))
    {
      Pkt = MsgMgr->GetPacket ();          /* if SendTo node, get empty packet */
      Pkt->TimeSent = Pkt->LastTime = 0;
      SendList [i] = Pkt;
      SendTo [nSend++] = i;
      NodeTime [i] = 0;
    }
    else
      SendList [i] = NULL;

    if (NI->Recv [i] > 0)
    {
      if (i != NI->Node) RecvFrom [nRecv++] = i;
    }
    SendReq [i] = MPI_REQUEST_NULL;
  }


/* Set up ring buffers to handle received packets & messages.  There are two */
/* buffers.  PendList handles the packets.  A packet is put in the slot      */
/* determined by its LastTime field, and becomes free after that slot is     */
/* current.  MsgList handles message pointers.  The actual messages are      */
/* still in their packets, they are just linked to the slot to be processed. */

  SlotNow  = 0;                          /* Slot currently being processed */
  nSlots   = 2 * NI->MaxSynDelay;        /* The longest time a message can wait */

  MsgList  = (Message **) calloc (nSlots, sizeof (Message *));
  PendList = (PACKET **)  calloc (nSlots, sizeof (PACKET *));

  for (i = 0; i < nSlots; i++)
  {
    PendList [i] = NULL;
    MsgList  [i] = NULL;
  }

}

/*------------------------------------------------------------------------*/

MessageBus::~MessageBus ()
{
//MEMFREEOBJ (MEM_KEY);

//if (NI->Node == 0)
//  fclose (msgFP);
}

/*---------------------------------------------------------------------------*/
/* Called by Compartment to send spike messages.  It gets a SENDITEM list    */
/* and count, to avoid overhead of repeated calls.                           */

void MessageBus::SendSpikeMsgs (int TimeStep, int Delay, int nTo, SENDITEM *ToList)
{
  PACKET *Pkt;
  Message *Msg;
  int i, t, delay;

  delay = TimeStep + Delay;

//nSpikes += nTo;
  for (i = 0; i < nTo; i++)
  {
    Pkt = SendList [ToList [i].Node];                  /* Select destination packet */
    Msg = &(Pkt->Msgs [Pkt->nMsgs++]);

    Msg->Spike.Type = MSG_SPIKE;
    t = ToList [i].Delay + delay;
    Msg->Spike.Time = t;
    if (t >= Pkt->LastTime) Pkt->LastTime = t;
    Msg->Spike.Addr = ToList [i].Addr;
    msgSent++;

    if (Pkt->nMsgs >= MSGS_PER_PACKET)
    {
      SendPacket (Pkt, ToList [i].Node, TimeStep);
      Pkt = MsgMgr->GetPacket ();
      Pkt->LastTime = TimeStep;
      SendList [ToList [i].Node] = Pkt;
    }
  }
}

/*---------------------------------------------------------------------------*/
/* Called by Stimulus routines.  It sends messages only to this node.        */

void MessageBus::SendStimMsg (int TimeStep, int nTo, char **ToList,
                              int MsgType, double Value)
{
  PACKET *Pkt;
  Message *Msg;
  int i;

  for (i = 0; i < nTo; i++)
  {
    Pkt = SendList [NI->Node];
    Msg = &(Pkt->Msgs [Pkt->nMsgs++]);
  
    Msg->Stim.Type  = MsgType;
    Msg->Stim.Time  = TimeStep;
    if (TimeStep > Pkt->LastTime) Pkt->LastTime = TimeStep;
    Msg->Stim.Addr  = (void *) ToList [i];
    Msg->Stim.Value = Value;
    msgSent++;
  
    if (Pkt->nMsgs >= MSGS_PER_PACKET)
    {
      SendPacket (Pkt, NI->Node, TimeStep);
      Pkt = MsgMgr->GetPacket ();
      Pkt->TimeSent = Pkt->LastTime = TimeStep;
      SendList [NI->Node] =  Pkt;
    }
  }
}

/*---------------------------------------------------------------------------*/
/* Called by Send*Msg and FlushMsgs - Calls MPI to send packet to other      */
/* node, or if same node, links packet into PendList                         */

void MessageBus::SendPacket (PACKET *Pkt, int ToNode, int TimeStep)
{
  int bytes, islot;
  char *data;
  bool haveMsg;

  Pkt->TimeSent = TimeStep;
  Pkt->Node     = NI->Node;
  if (ToNode == NI->Node)                /* Packet goes to this node */
  {
    Pkt->nMsgs &= ~SYNC_MASK;
    islot = Pkt->LastTime - TimeStep;
    if (islot < 0)
    {
      printf ("MessageBus::SendPacket : Error, islot < 0 (%d)\n", islot);
      Abort (__FILE__, __LINE__, -8);
    }
    islot = (islot + SlotNow) % nSlots;
    Pkt->link = PendList [islot];
    PendList [islot] = Pkt;

    SlotMsgs (Pkt, TimeStep);
  }
  else
  {
    data  = ((char *) Pkt) + PKTLNKSIZE;
    bytes = PKTHDRSIZE + (Pkt->nMsgs & ~SYNC_MASK) * sizeof (Message);
  
    MPI_Send (data, bytes, MPI_BYTE, ToNode, TAG_PACKET, MPI_COMM_WORLD);

    MsgMgr->FreePacket (Pkt);

    /* Now call the receiver to check for any incoming messages */
  
    haveMsg = true;
    while (haveMsg)
      haveMsg = ReceiveMsgs (TimeStep, false);
  }
  pktSent++;
}

/*---------------------------------------------------------------------------*/
/* This routine receives any MPI messages that may be waiting It uses either */
/* MPI_Probe or MPI_Iprobe (depending on wait argument), so it can either    */
/* return immediately if nothing is pending, or block until something is     */
/* ready to be received (to force synchronization).                          */

bool MessageBus::ReceiveMsgs (int TimeStep, bool wait)
{
  MPI_Status status;
  PACKET *Pkt;                /* The area into which MPI places data */
  int islot, bytes, flag;
  char *data;
  bool haveMsg;

  haveMsg = false;
  if (wait)
  {
    MPI_Probe (MPI_ANY_SOURCE, TAG_PACKET, MPI_COMM_WORLD, &status);
    flag = 1;
  }
  else
  {
    flag = 0;
    MPI_Iprobe (MPI_ANY_SOURCE, TAG_PACKET, MPI_COMM_WORLD, &flag, &status);
  }

  if (flag != 0)
  {
    haveMsg = true;
    Pkt = MsgMgr->GetPacket ();
    data  = ((char *) Pkt) + PKTLNKSIZE;
    bytes = sizeof (PACKET) - PKTLNKSIZE;
    MPI_Recv (data, bytes, MPI_BYTE, MPI_ANY_SOURCE, TAG_PACKET, MPI_COMM_WORLD, &status);
    pktRecv++;
  
    if (Pkt->nMsgs & SYNC_MASK)
    {
      NodeTime [Pkt->Node] = Pkt->TimeSent;
      Pkt->nMsgs &= ~SYNC_MASK;
    }

    if (Pkt->nMsgs == 0)
    {
      MsgMgr->FreePacket (Pkt);
    }
    else
    {
      pktRecv += Pkt->nMsgs;
      islot = Pkt->LastTime - TimeStep;
      islot = (islot + SlotNow) % nSlots;
      Pkt->link = PendList [islot];
      PendList [islot] = Pkt;
    
      SlotMsgs (Pkt, TimeStep);
    }
  }
  return (haveMsg);
}

/*---------------------------------------------------------------------------*/
/* This walks through all messages in a packet, and links them to the        */
/* correct incoming time slot.                                               */

void MessageBus::SlotMsgs (PACKET *Pkt, int TimeStep)
{
  Message *Msg;
  int i, islot;

  Msg = Pkt->Msgs;
  for (i = 0; i < Pkt->nMsgs; i++)
  {
    islot = Msg->Any.Time - TimeStep;
    islot = (islot + SlotNow) % nSlots;
    Msg->Any.link = (ANY_MSG *) MsgList [islot];
    MsgList [islot] = Msg;

    Msg++;
  }
  Pkt->nMsgs = 0;
}

/*---------------------------------------------------------------------------*/
/* Called by Brain to distribute the current timestep's messages to their    */
/* destinations                                                              */

void MessageBus::DeliverMsgs (int TimeStep)
{
  PACKET *Pkt;
  Message *Msg;

//QQStateOn (QQDeliverMsgs);

/* First send any pending local messages */


  Pkt = SendList [NI->Node];
  if (Pkt->nMsgs > 0)
  {
    SendPacket (Pkt, NI->Node, TimeStep);
    Pkt  = MsgMgr->GetPacket ();
    Pkt->TimeSent = Pkt->LastTime = TimeStep;
    SendList [NI->Node] = Pkt;
  }

  Msg = MsgList [SlotNow];

  while (Msg != NULL)
  {
    switch (Msg->Any.Type)
    {
      case MSG_SPIKE: ((Synapse *) Msg->Any.Addr)->ReceiveMsg (TimeStep);
          break;

      case MSG_STIMV: ((Compartment *) Msg->Any.Addr)->ReceiveStimV (Msg);
          break;

      case MSG_STIMC: ((Compartment *) Msg->Any.Addr)->ReceiveStimC (Msg);
          break;
    }
    Msg = (Message *) Msg->Any.link;
  }
  MsgList [SlotNow] = NULL;
//QQStateOff (QQDeliverMsgs);
}

/*---------------------------------------------------------------------------*/
/* This is called (from Brain::DoThink) after cells have done their          */
/* processing, but before reporting &c.  It sends any remaining messages     */
/* from the current timestep, or sends an empty packet to each node that     */
/* receives messages from this node.  This packet has the synchronization    */
/* flag set.                                                                 */

void MessageBus::FlushMsgs (int TimeStep)
{
  PACKET *Pkt;
  int i, node;

//QQStateOn (QQFlushMsgs);
  for (i = 0; i < nSend; i++)
  {
    node = SendTo [i];
    Pkt = SendList [node];
    Pkt->nMsgs |= SYNC_MASK;
    SendPacket (Pkt, node, TimeStep);
    Pkt = MsgMgr->GetPacket ();
    Pkt->LastTime = TimeStep + 1;
    SendList [node] = Pkt;
  }
//QQStateOff (QQFlushMsgs);
}

/*---------------------------------------------------------------------------*/
/* This is called (from Brain::DoThink?) at the end of each timestep.  It    */
/* checks to see if all nodes that this node receives from have also         */
/* finished this timestep.  If not, it checks for incoming messages until    */
/* they have.                                                                */

void MessageBus::Sync (int TimeStep)
{
  PACKET *Pkt;
  int totalSpikes;

//QQStateOn (QQSync);

/* First free any packets in PendList for this timestep, since their         */
/* messages have all been processed.                                         */

//printf ("MessageBus::Sync: SlotNow = %d\n", SlotNow);
//PrintPktList (PendList [SlotNow], numPend [SlotNow]);
  Pkt = PendList [SlotNow];
  while (Pkt != NULL)
  {
    PendList [SlotNow] = Pkt->link;
    MsgMgr->FreePacket (Pkt);
    Pkt = PendList [SlotNow];
  }

  while (!Synched (TimeStep))
  {
    ReceiveMsgs (TimeStep, true);
  }
  MPI_Barrier (MPI_COMM_WORLD);
  SlotNow = ++SlotNow % nSlots;

//MPI_Reduce (&nSpikes, &totalSpikes, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
//if (NI->Node == 0)
//{
//  fprintf (msgFP, "%5d: %10d\n", TimeStep, totalSpikes);
//}
#ifdef MSG_STATS
  MsgMgr->Write (nSpikes);
#endif  
  nSpikes = 0;

  msgSent = msgRecv = pktSent = pktRecv == 0;

//QQStateOff (QQSync);
}

/*---------------------------------------------------------------------------*/
/* Checks the NodeTime array to see if all the RecvFrom from nodes have      */
/* reached a specified timestep.  Returns true if not receiving from anyone. */

bool MessageBus::Synched (int TimeStep)
{
  int i, idx;
  bool synched;

  synched = true;
//printf ("MessageBus::Synched: TimeStep = %d, nRecv = %d\n", TimeStep, nRecv);
  for (i = 0; i < nRecv; i++)
  {
    idx = RecvFrom [i];
//  printf ("MessageBus::Synched: NodeTimes [%d] = %d\n", idx, NodeTime [idx]);
    if (NodeTime [idx] < TimeStep)
    {
      synched = false;
      break;
    }
  }
  return (synched);
}

/*---------------------------------------------------------------------------*/

void MessageBus::PrintMsgList (Message *Msg)
{
  int i;

  printf ("MessageBus::PrintMsgList: Msg = %x\n", Msg);
  i = 0;
  while (Msg != NULL)
  {
    printf ("%3d: Msg %8x (pkt %3d) -> %8x\n",
            i, Msg, -1, Msg->Any.link);
    Msg = (Message *) Msg->Any.link;
    i++;
  }
}

/*---------------------------------------------------------------------------*/

void MessageBus::PrintPktList (PACKET *Pkt)
{
  int i;

  printf ("MessageBus::PrintPktList:\n");
  i = 0;
  while (Pkt != NULL)
  {
    printf ("%3d: Pkt %3d (%8x) -> %8x, nMsgs = %d\n",
            i, Pkt->ID, Pkt, Pkt->link, Pkt->nMsgs);
    Pkt = Pkt->link;
    i++;
  }
}

/*---------------------------------------------------------------------------

Message passing & synchronization.  

There are three kinds of nodes relative to this node, ones it sends
packets to (SendTo nodes), ones it gets packets from (RecvFrom nodes),
and ones that it doesn't communicate with at all.  The code maintains
lists of these nodes.  For efficient indexing, they're mostly arrays with
elements for each node.  Those nodes that this node doesn't talk to are
set to NULL.  All nodes that this one talks to are in the SendTo list,
since they all must at least be sent a SYNC packet at each timestep.  Only
the nodes that send this node actual synapse messages will be in the
RecvFrom list, though.

Sending:

Messages are sent in packets, as defined in Packet.h.  These packets
contain some header info, and space for MSGS_PER_PACKET messages.  The
SendList array maintains a pointer to an active packet for each node
being sent to.

To send messages, the program makes calls to the Send*Msg functions in
this module.  These functions place the information to be sent in the
packet for the destination node, and maintain the packet header.  When a
packet is full, it is sent.  After cell processing for each timestep is
complete, FlushMsgs should be called to send partly filled packets.

To understand the synchronization, consider that there is a maximum
"distance" (really a propagation time delay - NI->MaxSynDelay), between
any two nodes.  For this node to proceed beyond the end of a timestep,
all SendTo nodes must be less than this distance beyond the current
timestep, and this node must be less than this distance beyond the
RecvFrom nodes.  The NodeTime array keeps track of what timestep each
other node is at.

Synchronization, timestep, and message count info is contained in the
packet header.  Only enough bytes for the header and messages are sent:
the receive has allocated space for the largest possible send, and reads
the packet into that space.


Receiving:

Packets are received by the ReceiveMsgs function.  The MPI code does not
need to know which node a packet is to be received from, so the code
simply checks for incoming packets.  (Note: local packets bypass MPI, and
are put directly in PendList.)

The incoming data is placed in an empty packet.  This now contains the
sending node & time.  If the sync flag in the packet is sent, the NodeTime
entry for the node is updated.  If the packet contains messages, it is passed
to SlotMsgs.

The code maintains a ring buffer of pending packet lists (PendList).  A
packet may contain messages to be delivered at different timesteps (up to
NI->MaxSynDelay from now).  The sending code maintains the LastTime field
in the packet header, and the received packet is placed in this slot in
the ring.  At the end of a timestep, the packets in the current slot will
be empty, and so can be returned to the free list.

There is also a parallel ring buffer of Message pointers (MsgList).  As
each packet is scanned, it is linked into the list corresponding to its
delivery time (using the empty link field in the packet space, to avoid
having to copy everything).  At each timestep, the DeliverMsgs function
takes the current list, and links the message pointers into the receiving
compartments lists.

Note that since Message pointers just point to locations within packets,
Messages never need to be freed.  (And indeed, trying to do so would
probably cause a seg fault...) The intent is to eliminate need for
multiple copy operations on messages, and reduce the granularity of
allocating & freeing message space.  In ddition, memory should stay more
coherent, and thus allow for more caching.


Synchronization

This now becomes fairly simple.  At the end of each timestep, the
NodeTimes fields are checked.  If this node is within the minimum disttance
from the sending & receiving nodes, it can proceed; if not, it
must wait for more packets to be received, and check again.

---------------------------------------------------------------------------*/

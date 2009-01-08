#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include "Message.h"
#include "Packet.h"

/* Try allocating in chunks that are one 80x86 memory page */

#define MSG_CHUNK (4096 / sizeof (Message))
//#define MSG_CHUNK (4)

/**
 * This is a re-implementation of the MessageManager using packets.  Message
 * no longer exist as independent objects: they are instead contained in
 * packets of many messages.  Allocation is per-packet.
 *
 * The manager contains a pointer to a list (stack) of free packets, and a
 * count of the number free.  Code that needs a packet calls GetPacket,
 * which returns a pointer to the next free packet.  If there are no free
 * packets, more are allocated and placed on the list.
 *
 * When the using code is finished with the packet, it calls FreePacket,
 * which places the packet back on the stack again.  There is no mechanism
 * for actually deleting packets, since it's expected that the number will
 * converge to some steady-state value as a run progresses.  Any freed
 * messages would thus have to be reallocated.
 */
class MessageManager
{
public:
        ///The total number of packets created by MessageManager
  int nTotal;
        ///The number of available packets
  int nFree;
        ///Unique id given to a packet
  int serial;
        ///Pointer to linked list of available packets
  PACKET *Free;

/* For debug */

#ifdef MSG_STATS
  int nAlloc;
  int nFreed;
  int nGot;
  int nRet;
  char fname [256];
  FILE *out;

  void Write (int);
#endif

        /**
         * Constructor initializes variables
         */
  MessageManager (int);
        /**
         * Destructor
         */
  ~MessageManager ();

        /**
         * Retrieves an available packet from list, calling AllocPkts if it has run out.
         * The packet is cleared before being returned to ensure that it is ready for data
         * to be added.
         * @return Pointer to the retrieved packet
         */
  PACKET *GetPacket ();
  
        /**
         * Restores packet to the available list
         * @param pkt Pointer to the packet
         */
  void FreePacket (PACKET *pkt);

        /**
         * Whenever new packets are needed, a batch is created and added to the list
         */
  void AllocPkts ();
  
        /**
         * Provides information on the total number of packets created and packets available
         * @param total Integer address where total packet count can be stored.
         * @param nfree Integer address where number of free pckets can be stored.
         */
  void GetMsgCounts (int *total, int *nfree);
};

#endif 

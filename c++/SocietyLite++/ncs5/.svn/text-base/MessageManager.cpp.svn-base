/* This is a re-implementation of the MessageManager using packets.  Message */
/* no longer exist as independent objects: they are instead contained in     */
/* packets of many messages.  Allocation is per-packet.                      */

/* The manager contains a pointer to a list (stack) of free packets, and a   */
/* count of the number free.  Code that needs a packet calls GetPacket,      */
/* which returns a pointer to the next free packet.  If there are no free    */
/* packets, more are allocated and placed on the list.                       */

/* When the using code is finished with the packet, it calls FreePacket,     */
/* which places the packet back on the stack again.  There is no mechanism   */
/* for actually deleting packets, since it's expected that the number will   */
/* converge to some steady-state value as a run progresses.  Any freed       */
/* messages would thus have to be reallocated.                               */

#include <stdio.h>
#include "MessageManager.h"
#include "Memory.h"

#include "debug.h"
#include "memstat.h"

#define MEM_KEY  KEY_MESSAGEMGR

MessageManager::MessageManager (int Node)
{
  MEMADDOBJ (MEM_KEY);

  printf ("MessageManager::MessageManager: initialized\n");
  nTotal = nFree = 0;
  serial = 0;
  Free = NULL;

#ifdef MSG_STATS

  nAlloc = nFreed = nGot = nRet = 0;
  sprintf (fname, "msgStats.%3.3d", Node);
  out = fopen (fname, "w");

#endif
}

/*------------------------------------------------------------------------*/

MessageManager::~MessageManager ()
{
  MEMFREEOBJ (MEM_KEY);
}

/*------------------------------------------------------------------------*/

PACKET *MessageManager::GetPacket ()
{
  PACKET *pkt;
 
  if (nFree == 0) AllocPkts ();

  pkt = Free;
  Free = Free->link;
  nFree--;
//printf ("MessageManager::GetPacket: returning pkt %d\n", pkt->ID);

#ifdef MSG_STATS
  nAlloc++;
  nGot++;
#endif

  pkt->nMsgs = 0;
  pkt->link  = NULL;

  return (pkt);
}

/*------------------------------------------------------------------------*/

void MessageManager::FreePacket (PACKET *pkt)
{
  pkt->link = Free;
  Free = pkt;
  nFree++;

#ifdef MSG_STATS
  nFreed++;
  nRet++;
#endif
}

/*------------------------------------------------------------------------*/
             
void MessageManager::AllocPkts ()
{
  PACKET *pkt;
  int i;

  pkt = (PACKET *) calloc (MSG_CHUNK, sizeof (PACKET));
  if (pkt == NULL) MemError ("MessageManager::AllocPkts");

  for (i = 0; i < MSG_CHUNK; i++)
  {
    pkt->link = Free;
    pkt->ID   = serial++;
    Free = pkt;
    pkt++;
  }
  nTotal += MSG_CHUNK;
  nFree  += MSG_CHUNK;
}

/*------------------------------------------------------------------------*/

void MessageManager::GetMsgCounts (int *total, int *nfree)
{
  *total = nTotal;
  *nfree = nFree;
}

/*------------------------------------------------------------------------*/

#ifdef MSG_STATS

void MessageManager::Write (int nSpikes)
{
  int buf [4];

  buf [0] = nTotal;
  buf [1] = nFree;
  buf [2] = nAlloc;
  buf [3] = nFreed;
//fwrite (buf, sizeof (int), 4, out);
  fprintf (out, "%10d %10d %10d %10d %10d %10d %10d\n", 
           nTotal, nFree, nAlloc, nFreed, nGot, nRet);

  nAlloc = nFreed = 0;
}
#endif

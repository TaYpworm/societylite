/* Maintains a pool of ActiveSyn structs, similiar to the Message Pool.  Eac */
/* compartment has a list, which has one of these for each synapse firing    */
/* that is being processed.  It contains a pointer to the synapse that       */
/* fired, and a count of the number of timesteps it should remain active.    */
/* This pool should minimize the overhead of allocation, while using less    */
/* memory than giving each synapse its own array..                           */

#include <stdlib.h>
#include <stdio.h>

#include "ActiveSyn.h"
#include "Memory.h"

#include "debug.h"
#include "memstat.h"

#define MEM_KEY  KEY_PSGPOOL

ActiveSynPool::ActiveSynPool (int Node)
{
  MEMADDOBJ (MEM_KEY);

  nTotal = nFree = 0;
  Free = NULL;

#ifdef ACTIVE_STATS

  nAlloc = nFreed = 0;
  sprintf (fname, "active.%d", Node);
  out = fopen (fname, "w");

#endif
}

/*------------------------------------------------------------------------*/

ActiveSynPool::~ActiveSynPool ()
{
  MEMFREEOBJ (MEM_KEY);
}

/*------------------------------------------------------------------------*/

ACTIVESYN *ActiveSynPool::GetActiveSyn ()
{
  ACTIVESYN *ap;
 
  if (nFree == 0) AllocActiveSyn ();

  ap = Free;
  Free = Free->link;
  nFree--;

#ifdef ACTIVE_STATS
  nAlloc++;
#endif

  return (ap);
}

/*------------------------------------------------------------------------*/

void ActiveSynPool::FreeActiveSyn (ACTIVESYN *ap)
{
  ap->link = Free;
  Free = ap;
  nFree++;

#ifdef ACTIVE_STATS
  nFreed++;
#endif
}

/*------------------------------------------------------------------------*/
             
void ActiveSynPool::AllocActiveSyn ()
{
  ACTIVESYN *ap;
  int i;

  ap = (ACTIVESYN *) calloc (AS_CHUNK, sizeof (ACTIVESYN));
  if (ap == NULL) MemError ("ActiveSynPool::AllocActiveSyn");

  for (i = 0; i < AS_CHUNK; i++)
  {
    ap->link = Free;
    Free = ap;
    ap++;
  }
  nTotal += AS_CHUNK;
  nFree  += AS_CHUNK;
//printf ("ActiveSynPool::AllocActiveSyn: added %d, nFree = %d, nTotal = %d\n",
//        AS_CHUNK, nFree, nTotal);
}

/*------------------------------------------------------------------------*/

void ActiveSynPool::GetActiveSynCounts (int *total, int *nfree)
{
  *total = nTotal;
  *nfree = nFree;
}

/*------------------------------------------------------------------------*/

#ifdef ACTIVE_STATS

void ActiveSynPool::Write ()
{
  int buf [4];

  buf [0] = nTotal;
  buf [1] = nFree;
  buf [2] = nAlloc;
  buf [3] = nFreed;
  fwrite (buf, sizeof (int), 4, out);
  nAlloc = nFreed = 0;
}
#endif

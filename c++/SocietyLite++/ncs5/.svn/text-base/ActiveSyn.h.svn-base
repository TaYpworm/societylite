#ifndef ACTIVESYN_H
#define ACTIVESYN_H

#include "Synapse.h"
#include "ActiveSynPtr.h"

class Synapse;

/* Allocation unit: in multiples of a 80x86 memory page (4 KB)               */

#define AS_CHUNK ((8 * 4096) / sizeof (ACTIVESYN))

class ActiveSynPool
{
public:
  int nTotal;
  int nFree;
  ACTIVESYN *Free;

/* For debug */

#ifdef ACTIVE_STATS
  int nAlloc;
  int nFreed;
  char fname [256];
  FILE *out;

  void Write ();
#endif

  ActiveSynPool (int);
  ~ActiveSynPool ();

  ACTIVESYN *GetActiveSyn ();
  void FreeActiveSyn (ACTIVESYN *);

  void AllocActiveSyn ();
  void GetActiveSynCounts (int *, int *);
};

#endif 

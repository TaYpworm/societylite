#ifndef ACTIVESYNPTR_H
#define ACTIVESYNPTR_H

#ifndef SAME_PSC
  #include "Synapse.h"

  class Synapse;
#endif

/* This is the basic struct.  It holds synapse & link pointers, and a count  */

typedef struct _ActiveSyn
{
  _ActiveSyn *link;      
  int Count;             
  float Value;
#ifdef SAME_PSC
  float Reversal;
  double *PSG;
#else
  Synapse *Syn;
#endif
} ACTIVESYN;

#endif 

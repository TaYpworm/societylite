/* This code creates an individual random number generator for each type of  */
/* brain element defined in the input file.  That is, there's a RNG for each */
/* definition, not one for each object that may wind up being created from   */
/* the definition.                                                           */

/* The RNG objects are created as arrays, then the seed for each is passed   */
/* to it in a loop.  This is probably creating some inefficiency, since      */
/* there's a good chance that any particular brain element definition will   */
/* not be used on some nodes (or at all, since definitions can exist in      */
/* input without being used!).  However, it does make lookup easier...       */


#include <stdio.h>
#include "parse/arrays.h"

#include "debug.h"
#include "memstat.h"

#define MEM_KEY  KEY_RANDOMMGR
 
#include "RandomManager.h"
 
RandomManager::RandomManager (ARRAYS *AR)
{
  int i;

  MEMADDOBJ (MEM_KEY);

  nCmp      = AR->nCmp;
  nChan     = AR->nChannel;
  nStim     = AR->nStimulus;
  nReport   = AR->nReports;
  nSynapse  = AR->nSynapse;
  nSynLearn = AR->nSynLearn;
  nSynRSE   = AR->nSynFD;

  Brain   = new Random ();
  Brain->SeedIt (AR->Brain->Seed);
  Connect = new Random ();
  Connect->SeedIt (AR->Brain->Seed);

  Cmp = new Random [nCmp] ();
  for (i = 0; i < nCmp; i++)
    Cmp [i].SeedIt (AR->Cmp [i]->Seed);
    
  Chan = new Random [nChan] ();
  for (i = 0; i < nChan; i++)
    Chan [i].SeedIt (AR->Channel [i]->Seed);
    
  Stim = new Random [nStim] ();
  for (i = 0; i < nStim; i++)
    Stim [i].SeedIt (AR->Stimulus [i]->Seed);
    
  Report = new Random [nReport] ();
  for (i = 0; i < nReport; i++)
    Report [i].SeedIt (AR->Reports [i]->Seed);
    
  Synapse = new Random [nSynapse] ();
  for (i = 0; i < nSynapse; i++)
    Synapse [i].SeedIt (AR->Synapse [i]->Seed);
    
  SynLearn = new Random [nSynLearn] ();
  for (i = 0; i < nSynLearn; i++)
    SynLearn [i].SeedIt (AR->SynLearn [i]->Seed);
    
  SynRSE = new Random [nSynRSE] ();
  for (i = 0; i < nSynRSE; i++)
    SynRSE [i].SeedIt (AR->SynFD [i]->Seed);
}

RandomManager::~RandomManager ()
{
  MEMFREEOBJ (MEM_KEY);

  delete Brain;
  delete Connect;
  delete []Cmp;
  delete []Chan;
  delete []Stim;
  delete []Report;
  delete []Synapse;
  delete []SynLearn;
  delete []SynRSE;
}

/*------------------------------------------------------------------------*/

Random *RandomManager::GetCmpRand (int idx)
{
  return (&Cmp [idx]);
}

/*------------------------------------------------------------------------*/

Random *RandomManager::GetChannelRand (int idx)
{
  return (&Chan [idx]);
}

/*------------------------------------------------------------------------*/

Random *RandomManager::GetStimRand (int idx)
{
  return (&Stim [idx]);
}

/*------------------------------------------------------------------------*/

Random *RandomManager::GetReportRand (int idx)
{
  return (&Report [idx]);
}

/*------------------------------------------------------------------------*/

Random *RandomManager::GetSynapseRand (int idx)
{
  return (&Synapse [idx]);
}

/*------------------------------------------------------------------------*/

Random *RandomManager::GetSynLearnRand (int idx)
{
  return (&SynLearn [idx]);
}

/*------------------------------------------------------------------------*/

Random *RandomManager::GetSynRSERand (int idx)
{
  return (&(SynRSE [idx]));
}

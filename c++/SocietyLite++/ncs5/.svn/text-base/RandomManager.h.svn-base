#ifndef RANDOMMANAGER_H
#define RANDOMMANAGER_H

#include <time.h>
#include "Random.h"
#include "parse/arrays.h"
#include "defines.h"

class Random;

class RandomManager
{
 public:
  Random *Connect;       // only have one of these
  Random *Brain;

  int nCmp;
  Random *Cmp;
  int nChan;
  Random *Chan;
  int nStim;
  Random *Stim;
  int nReport;
  Random *Report;
  int nSynapse;
  Random *Synapse;
  int nSynLearn;
  Random *SynLearn;
  int nSynRSE;
  Random *SynRSE;


  RandomManager (ARRAYS *);
  ~RandomManager ();

  Random *GetCmpRand (int);
  Random *GetChannelRand (int);
  Random *GetStimRand (int);
  Random *GetReportRand (int);
  Random *GetSynapseRand (int);
  Random *GetSynLearnRand (int);
  Random *GetSynRSERand (int);
};

#endif

/* This module handles processing of Stimulus objects.  See documentation at */
/* end of file.                                                              */

#include "defines.h"
#include <stdio.h>
#include <math.h>

#include "DoStim.h"
#include "Stimulus.h"
#include "Managers.h"
#include "parse/ReadFile.h"

#include "debug.h"
#include "memstat.h"

#define MEM_KEY KEY_DOSTIM

#define CHUNK 256          /* chunk size for allocation */
#define FIREWIND 0.0025    /* duration of stimulus firing window, sec */

static void Error1 (T_STIMULUS *);
static void Error2 (T_STIMULUS *);
static void Error3 (T_STIMULUS *);
static void Error4 (T_STIMULUS *);

/*---------------------------------------------------------------------------*/
/* Cycle through the Brain's stimulus list, doing doing each stimulus if the */
/* time & such are correct.                                                  */

void DoStimuli (int TimeStep)
{
  Stimulus *S, *Snext;
  bool StimDone;

  S = TheBrain->StimList;
  while (S != NULL)
  {
    Snext = S->next;
    StimDone = S->DoStimulus (TimeStep);
    
    //If the stim is finished, and the user will not dynamically add new start/stop time, delete it    
    if (StimDone && !S->Dynamic)
        delete S;         /* Stimulus finished, so delete from list */
      
    S = Snext;
  }
}

/*---------------------------------------------------------------------------*/
/* Makes a Stimulus object from the STIMULUS and STIM_INJECT inputs, or, if  */
/* the specified Stimulus has already been created, add the STIM_INJECT's    */
/* cells to its list.                                                        */

void AddStimulus (T_STINJECT *iStInj, T_STIMULUS *iStim, RandomManager *RM)
{
  Stimulus *S;
  Random *RN;
//int n, ncells, *CellList;
 
//printf ("DoStim::AddStimulus: nStimulus = %d\n", TheBrain->nStimulus);
  RN = RM->GetStimRand (iStInj->STIM_TYPE);
  if (RN == NULL)
  {
    fprintf (stderr, "MakeStimulus: Random Number Generator is NULL, exiting\n");
    Abort (__FILE__, __LINE__, -3);
  }

/* If the stimulus is file-based, then read data if it hasn't yet been read */
  if (iStim->PATTERN == FILEBASED)
  {
    if (iStim->SpikeProb == NULL)         
      ReadFileStim (iStim);

    S = new Stimulus (iStim, iStInj, RN);
//  printf ("DoStim::AddStimulus: made FILEBASED, id = %d\n", S->id);
    if (S->id < 0) delete S;
  }
  else 
  {                    
    S = new Stimulus (iStim, iStInj, RN);
    if (S->id < 0) delete S;
  } 
//printf ("DoStim::AddStimulus: done\n");
}

/*---------------------------------------------------------------------------*/
/* Reads stimulus data from file, and does preprocessing on it.  Normalize   */
/* the decibel input data and converts each value to a frequency value.  Als */
/* creates the spike probability matrix.                                     */

/* The data file is currently organized 90 degrees off.  It has nFreqs,    */
/* and the number of columns is determined by the number of timesteps it     */
/* covers.  This is also wasting memory, since a Stimulus object will be     */
/* created only if at least some the group of cells receiving the stimulus   */
/* are on this node.  Should be fixed later...                               */

void ReadFileStim (T_STIMULUS *iStim)
{
  double dmax, dmin, drange, admin, adrange, tratio, cf, *data;
  int i, nread;
//char *inbuf, *s;

//printf ("DoStim::ReadFileStim: '%s'\n", iStim->FileName);
  if (iStim->nFreqs <= 0) Error1 (iStim);     /* Can't read if this is 0 ? */

  data = ReadFPNFile (iStim->FileName, NI->Node, &nread, 0);

  if (data == NULL) Error2 (iStim);
  if (nread == 0) Error3 (iStim);

/* Find min/max valuse for scaling */

  dmax = -INFINITY;
  dmin =  INFINITY;

  for (i = 0; i < nread; i++)
  {
    if (data [i] > dmax) dmax = data [i];
    if (data [i] < dmin) dmin = data [i];
  }

  if (dmax == dmin) Error4 (iStim);

  iStim->TimeIntervals = nread / iStim->nFreqs;

/* Now normalize the data, convert to frequency matrix and determine         */
/* probability.  This is assigned to the SpikeProb array, and the input      */
/* buffer can be freed.                                                      */

  iStim->nSpikeProb = nread;
  iStim->SpikeProb = (float *) calloc (nread, sizeof (float));

  drange  = dmax - dmin;
  admin   = dmin - drange * (iStim->DynRange [0] / (iStim->DynRange [1] - iStim->DynRange [0]));
  adrange = dmax - admin;

  tratio = FIREWIND / iStim->Time_Freq_Incr [0];
  cf     = (tratio / adrange) * (iStim->DynRange [1] * iStim->Time_Freq_Incr [0]);

//printf ("DoStim::ReadFileStim: nfreqs = %d, ntimes = %d\n", iStim->nFreqs, iStim->TimeIntervals);
  for (i = 0; i < nread; i++)
  {
    iStim->SpikeProb [i] = (float) (cf * (data [i] - admin));
  }

//for (i = 0; i < 25; i++)
//  printf ("%2d: %f\n", i, iStim->SpikeProb [i]);

  free (data);
//printf ("DoStim::ReadFileStim: done\n");
}

/*---------------------------------------------------------------------------*/
/* Cycles through the input, creating specified stimulus objects.  Note that */
/* Stimulus object is created from T_STINJECT!                               */

void MakeStimsFromInput (ARRAYS *AR, RandomManager *RM)
{
  T_BRAIN *iBrain;
  T_STIMULUS *iStim;
  T_STINJECT *iStInj;
  int i;

  iBrain = AR->Brain;

  for (i = 0; i < iBrain->nStInject; i++)            
  {
    iStInj = AR->StInject [iBrain->StInject [i]]; 
    iStim  = AR->Stimulus [iStInj->STIM_TYPE];    
    AddStimulus (iStInj, iStim, RM);
  }
  printf ("DoStim::MakeStimsFromInput: done\n");
}

/*---------------------------------------------------------------------------*/
/* Error functions that need to write to both stdout & stderr                */

static void Error1 (T_STIMULUS *iStim)
{
  char msg [1024];

  sprintf (msg, "DoStim::ReadFileStim: Error, FREQ_ROWS value <= 0\n");
  printf ("%s", msg);
  fprintf (stderr, "%s", msg);
  Abort (__FILE__, __LINE__, -3);
}

/*---------------------------------------------------------------------------*/

static void Error2 (T_STIMULUS *iStim)
{
  char msg [1024];

  sprintf (msg, "DoStim::ReadFileStim: Error, can't read from stimulus input file '%s' (%s:%d)\n",
             iStim->FileName, iStim->L.file, iStim->L.line);
  printf ("%s", msg);
  fprintf (stderr, "%s", msg);
  Abort (__FILE__, __LINE__, -3);
}

/*---------------------------------------------------------------------------*/

static void Error3 (T_STIMULUS *iStim)
{
  char msg [1024];

  sprintf (msg, "DoStim::ReadFileStim: Error, read 0 values from stimulus file '%s'\n",
                iStim->FileName);
  printf ("%s", msg);
  fprintf (stderr, "%s", msg);
  Abort (__FILE__, __LINE__, -3);
}

/*---------------------------------------------------------------------------*/

static void Error4 (T_STIMULUS *iStim)
{
  char msg [1024];

  sprintf (msg, "DoStim::ReadFileStim: Error, Max and Min from stimulus file '%s' are equal\n",
           iStim->FileName);
  printf ("%s", msg);
  fprintf (stderr, "%s", msg);
  Abort (__FILE__, __LINE__, -3);
}

/*-----------------------------------------------------------------------------

This module handles adding, deleting and calling Stimulus objects.  They can 
(potentially) be created either at startup, from info in the input or 
dynamically at run time.  (This last isn't inplemented yet, but code is being 
re-written from the old DoStim.cpp version to allow it.)

Stimulus.cpp does the actual creation of the object: This module is a layer on 
top of it, taking care of when and how the objects are added and removed.  A 
linked list of Stimulus objects kept in the Brain, so that new ones can be 
added, and old ones deleted when they have finished.

A stimulus object is created from two parts: a T_STIMULUS input defines the 
stimulus; a T_STINJECT says which cells are to receive it.  Many stim injects 
may reference the same definition, each creating a new stimulus object (or set 
of objects, for file-based), so the number of Stimulus objects created depends 
on the number of STIM_INJECTs, not STIMULUS.

This code should be very similar to CreateReport.cpp, just going in the opposite 
direction.  7/9/02 - JF

-----------------------------------------------------------------------------*/

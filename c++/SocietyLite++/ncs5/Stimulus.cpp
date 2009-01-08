/* This module handles external input to the Brain.  See description at end  */
/* of file.                                                                  */

#include <stdio.h>
#include "Stimulus.h"
#include "Managers.h"

#include "debug.h"
#include "memstat.h"

#define MEM_KEY  KEY_STIMULUS

#include <iostream>

using namespace std;

Stimulus::Stimulus (T_STIMULUS *iStim, T_STINJECT *iStInj, Random *RNG)
{
  int i;
  Port = iStim->Port;

  MEMADDOBJ (KEY_STIMULUS);
  id = TheBrain->nStimulus;
  RN = RNG;
  
  idx = iStim->L.idx;  //different from id - will be used for dynamic stim

  MsgType = (iStim->MODE == MODE_CURRENT) ? MSG_STIMC : MSG_STIMV;
  PATTERN = iStim->PATTERN;
  TIMING  = iStim->TIMING;
  
//printf ("Stimulus::Stimulus: pattern = %s, id = %d\n", NameDefine (PATTERN), id);

  AMP_Start  = iStim->AMP_Start;
  AMP_End    = iStim->AMP_End;

  iTime      = 0;                       /* Set up start & stop times */
  nTimes     = iStim->nTimes;
  TStart     = iStim->Time_Start;
  TStop      = iStim->Time_Stop;
  
  //A dynamic stimulus will have start/stop times add via port commands
  //This flag will prevent DoStim from deleting the stimulus due to being out of times
  Dynamic = 0;
  if (nTimes == 0 )
    Dynamic = 1;
  else if( TStart[0] < 0 && TStop[0] < 0 ) //user has made first time pair = -1
    Dynamic = 1;

  VERT_TRANS = iStim->VERT_TRANS;

  FREQ_Start = iStim->FREQ_Start;
  FREQ_End   = iStim->FREQ_End;
  nDataRows  = iStim->TimeIntervals;
  Data       = iStim->SpikeProb;

  nThings    = 0;
  ThingPtrs  = WillFire = NULL;
  DataIdx    = NULL;
                  
  SpikeTicks = (int) (NI->FSV * iStim->Time_Freq_Incr [0]);       /* Ticks per Data item */
  FireTicks  = (int) (NI->FSV * FIREWIND);               /* Ticks in firing window */
  nFireWin   = (int) (iStim->Time_Freq_Incr [0] / FIREWIND);  /* Number of firing windows */
  PulseTicks = (int) (NI->FSV * (float) iStim->WidthSec);

  nFreqs       = iStim->nFreqs;
  CellsPerFreq = iStim->CellsPerFreq;
  Func = &Stimulus::EmptyFunc;

  switch (PATTERN)
  {
    case INPUTSTIM: GetInputCells (iStInj);
                    II = In->MakeInput (iStim->FileName, iStim->Port, iStim->nFreqs * sizeof (float));
                    if (II == NULL)
                      id = -1;        /* Flag so caller will delete */
                    else
                    {
                      FREQ_Start = FREQ_End = iStim->DynRange [0];
                      
                      //use DynRange for scaling values
                      inputScalar = FIREWIND*(iStim->DynRange[1]-iStim->DynRange[0]);
                      inputTranslation = FIREWIND*iStim->DynRange[0];

                      nDataRows = 1;
                      Data  = (float *) calloc (nFreqs, sizeof (float));
                      Func = &Stimulus::InputStim;
                    }
        break;
    case FILEBASED: GetInputCells (iStInj);
                    if (nThings == 0)
                      id = -1;        /* Flag so caller will delete */
                    else
                    {
                      FREQ_Start = FREQ_End = iStim->DynRange [1];
                      Func = &Stimulus::FileStim;
                    }
        break;
    case PULSE:     if (GetCells (iStInj) > 0)
                    {
                      SpikeTicks = PulseTicks;
                      PulseTicks = (FREQ_Start == 0.0) ? 0 : (int) ((1.0 / FREQ_Start) * NI->FSV);
                      InPulse = true;
                      PulseAmp = AMP_Start;
                      Func = &Stimulus::PulseStimulus;      
                    }
        break;
    case LINEAR:    if (GetCells (iStInj) > 0)
                    {
                      PulseAmp = AMP_Start;
                      Func = &Stimulus::LinearStimulus;
                    }
        break;
    case SINE:      if (GetCells (iStInj) > 0)
                    {
                      Phase = iStim->PHASE * PI / 180.0;
                      Theta = Phase;

                      if (FREQ_Start == 0.0) FREQ_Start = 1.0;  // Default to 1.0

                      if (NI->FSV == 0) //treat FSV as 1
                        dTheta = FREQ_Start * 2.0 * PI;
                      else
                        dTheta = FREQ_Start * 2.0 * PI / (double) NI->FSV;

                      // number of table entries = FSV?

                      SineTable = (double *) calloc (NI->FSV, sizeof (double));
                      for (i = 0; i < NI->FSV; i++)
                      {
                        SineTable [i] = sin (Theta);
                        Theta += dTheta;
                        if (Theta >= 2 * PI) Theta -= 2.0 * PI;
                      }

                      // Cells receiving this stim get a random position in the table

                      CellTablePosition = (int *) calloc (nThings, sizeof (int));
                      for (i = 0; i < nThings; i++)
                        CellTablePosition [i] = (int) (RN->Rand () * NI->FSV);

                      Func = &Stimulus::SineStimulus;
                    }
        break;
    case ONGOING:   if (GetCells (iStInj) > 0)
                      Func = &Stimulus::OngoingStimulus;
        break;
    case NOISE:     if (GetCells (iStInj) > 0)
                      Func = &Stimulus::NoiseStimulus;
        break;
    default:        printf ("Stimulus::Stimulus: Unknown stimulus type\n");
                    Func = &Stimulus::EmptyFunc;
  }

  ResetStim ();

  CalculateAMPIncrement (); 

  if (TStart [0] == 0)   // do we start our stimulus right away, or delay it a bit?
    Active = true; 
  else
    Active = false;

/* The object inserts itself into the Brain's list... */

//printf ("Stimulus::Stimulus: %d inserting self into list\n", id);
  this->next = TheBrain->StimList;
  TheBrain->StimList = this;
  if (this->next != NULL) this->next->prev = this;
  this->prev = NULL;
  TheBrain->nStimulus++;
}

/*---------------------------------------------------------------------------*/
/* Note: DON'T free TStart, or data for type FILEBASED!  They are owned by   */
/* the T_STIMULUS instead of the T_STINJ.  so other stimulus objects might   */
/* be pointing to the same data                                              */

Stimulus::~Stimulus ()
{
  MEMFREEOBJ (KEY_STIMULUS);

//printf ("Stimulus::~Stimulus: deleting self\n");

  if ((Data != NULL) && (PATTERN != FILEBASED)) free (Data);

  if (WillFire   != NULL) free (WillFire);
  if (ThingPtrs  != NULL) free (ThingPtrs);
  if (DataIdx    != NULL) free (DataIdx);

  if (this == TheBrain->StimList)    /* Stimulus is first in list */
    TheBrain->StimList = this->next;

  if (this->next != NULL) this->next->prev = this->prev;
  if (this->prev != NULL) this->prev->next = this->next;

  TheBrain->nStimulus--;
//printf ("Stimulus::~Stimulus: deleted\n");
}

/*---------------------------------------------------------------------------*/
/* Get a list of the cmps on this node that will receive the stimulus, and   */
/* set up the data areas to point to them.  Probabilities &c are handled by  */
/* the caller.                                                               */

int Stimulus::GetInputCells (T_STINJECT *iStInj)
{
  Cell *Cel;
  int i, j, idx, ncells, nsize, *CellArray;

//printf ("Stimulus::GetInputCells\n");

  CellArray = CellMgr->ListCells (iStInj->Column, iStInj->Layer, iStInj->Cell, &ncells, &nsize);
//printf ("Stimulus::GetInputCells: ListCells returns ncells = %d, nsize = %d\n", ncells, nsize);

  if (CellArray == NULL)
    Error1 (iStInj);
  else if (ncells < nFreqs * CellsPerFreq)
    Error2 (iStInj, ncells);
  else if (nsize > 0)
  {
    ThingPtrs = (char **) calloc (nsize, sizeof (char *));
    WillFire  = (char **) calloc (nsize, sizeof (char *));
    DataIdx   = (int *)   calloc (nsize, sizeof (int));

    for (i = idx = 0; i < nFreqs; i++)
    {
      for (j = 0; j < CellsPerFreq; j++, idx++)
      {
        if (CellArray [idx] >= 0)
        {
          Cel = TheBrain->Cells [CellArray [idx]];
          ThingPtrs [nThings] = (char *) Cel->Compartments [iStInj->Cmp];
          DataIdx   [nThings] = i;
          if (nThings < nsize) nThings++;
        }
      }
    }
  }
  return (nThings);
}

/*---------------------------------------------------------------------------*/
/* Like GetInputCells, this gets a list of cmps that receive a stimulus.     */
/* This version is used by the internally-generated stimulus functions, and  */
/* handles receiving probabilities and such.                                 */

int Stimulus::GetCells (T_STINJECT *iStInj)
{
  Cell *Cel;
  double prob;
  int i, j, *CellIDs;

//printf ("Stimulus::GetCells: nThings =  %d \n", nThings);

  CellIDs = CellMgr->ListCellsOnNode (iStInj->Column, iStInj->Layer, iStInj->Cell, &nThings);

  if (nThings > 0)
  {
    if (iStInj->Prob < 1.0)          /* pick cells to stimulate by probability */
    {
      for (i = j = 0; i < nThings; i++)     
      {                                    
        prob = RN->Rand ();
        if (prob <= iStInj->Prob)          /* remove cell if prob > specified */
          CellIDs [j++] = CellIDs [i];
      }  
      nThings = j;
    }

/* Make the ThingList and fill it with pointers to cmps that will get the stim */

    if (nThings > 0)
    {
      ThingPtrs = (char **) malloc (nThings * sizeof (char *));

      for (i = 0; i < nThings; i++)
      {
        Cel = TheBrain->Cells [CellIDs [i]];
        ThingPtrs [i] = (char *) Cel->Compartments [iStInj->Cmp];
      }
    }
  }
  else
    id = -1;

  if (CellIDs != NULL) free (CellIDs);
  return (nThings);
}

/*---------------------------------------------------------------------------*/
/* Set internal stimulus values to initial conditions.  This is used when    */
/* the stimulus is to be repeated (there is more than one time interval)     */

void Stimulus::ResetStim ()
{
  iSpTick  = SpikeTicks;
  iFwTick  = FireTicks;
  iPTicks  = PulseTicks;
  iFireWin = nFireWin;
  iRow     = 0;

  Time_Elapsed = 0;
  Theta        = Phase;
}

/*---------------------------------------------------------------------------*/
/* If the current timestep is within one of the active time periods, call    */
/* the appropriate routine to produce the specified kind of simulus.         */

bool Stimulus::DoStimulus (int TimeStep)
{
  bool done;

  done = false;
//printf ("Stimulus::DoStimulus: pattern = %s, t = %d\n", NameDefine (PATTERN), TimeStep);
  if ((TimeStep >= TStart [iTime]) && (TimeStep <= TStop [iTime]))
  {
    (this->*Func) (TimeStep);

    Time_Elapsed++;

    if (TimeStep == TStop [iTime]) 
    {
      iTime++;

/* If the number of intervals is used up, return the done flag so the        */
/* stimulus will delete itself.  Otherwise, reset everything to replay the   */
/* stim from start at the next interval.                                     */

      if (iTime >= nTimes) 
        done = true;
      else
        ResetStim ();
    }
  }
  return (done);
}

/*---------------------------------------------------------------------------*/
/* This routine is rather complicated.  The Data array contains              */
/* probabilities, each of which applies over a user-specified spike window   */
/* of N = (iStim->Time_Freq_Incr [0] * Brain->FSV) ticks.  Within that       */
/* window, there are some number of biologically- determined firing windows. */

/* At the start of each firing window, the probability value is applied to   */
/* each cell in the list to determine whether it will fire.  If it fires, it */
/* produces a pulse with amplitude AMP_START, lasting for WidthSec * FSV     */
/* ticks                                                                     */

void Stimulus::FileStim (int TimeStep)
{
  double r;
  int i, idata;
 
//printf ("Stimulus::FileStim: t = %4d: iRow = %d/%d, FT = %d/%d, FW = %d/%d. P = %d/%d, Sp = %d/%d\n",
//        TimeStep, iRow, nDataRows, iFwTick, FireTicks, iFireWin, nFireWin, iPTicks, PulseTicks, iSpTick, SpikeTicks);

  if (iFwTick == FireTicks)            /* Start of firing window? */
  {
    nFire = 0;
    iPTicks = PulseTicks;
    if (iFireWin > 0)                     /* Windows left in spike period? */
    {
      idata = iRow * nFreqs;
      for (i = 0; i < nThings; i++)           /* Determine which cells   */
      {                                         /* will receive this pulse */
        r = RN->Rand ();
        if (r < Data [idata + DataIdx [i]])
          WillFire [nFire++] = ThingPtrs [i];
      }
    }
  }

  if (nFire > 0)               
  {
    MsgBus->SendStimMsg (TimeStep, nFire, WillFire, MsgType, AMP_Start);
    iPTicks--;
    if (iPTicks == 0) nFire = 0;
  }

  iFwTick--;
  if (iFwTick == 0)         /* if end of firing window, and haven't */
  {                         /* finished all windows, start another  */
    iFireWin--;
    iFwTick = FireTicks;
  }

  iSpTick--;
  if (iSpTick == 0)         /* if end of spike window, and not end */
  {                         /* of data, move to next spike */
    iRow++;
    if (iRow >= nDataRows) 
      ResetStim ();
    else
    {
      iSpTick  = SpikeTicks;
      iFwTick  = FireTicks;
      iFireWin = nFireWin;
    }
  }
} 

/*---------------------------------------------------------------------------*/
/* This routine is similar to FileStim, the main difference being that it    */
/* gets a new line of data from input for every spike window, rather than    */
/* having all the data been read in as a block when the stim is created.  It */
/* handles all frequency bands (CellsPerFreq), rather than having a separate */
/* object for each band, since that's much simpler than having the Input     */
/* module try to scatter the data to the correct nodes.                      */

void Stimulus::InputStim (int TimeStep)
{
  double r;
  int i;
 
  //Stims such as file based will immediately start at t=TIME_START even though
  //iSpTick is at the maximum and has not counted down yet
  //How can Input Stim be done so that it starts immediately despite
  //iSpTick being at the maximum timer after ResetStim()? have it count up instead
  if( iSpTick == SpikeTicks )
    iSpTick = 0;
 
//printf ("Stimulus::InputStim: t = %d, iSpTick = %d\n", TimeStep, iSpTick);

  if (iSpTick == 0)         /* Time to get next data from input? */
  {
    //get data
    In->Read (II, Data);
    
    //transform data based on Dynamic Range using pre-computed values
    for( int i=0; i<nFreqs; i++ )
        Data[i] = Data[i]*inputScalar + inputTranslation;

    //iSpTick  = SpikeTicks;
    iFwTick  = FireTicks;
    iFireWin = nFireWin;
  }

  if (nThings > 0)        /* because IONode may not have cells */
  {
    if (iFwTick == FireTicks)            /* Start of firing window? */
    {
      nFire   = 0;
      iPTicks = PulseTicks;
      if (iFireWin > 0)                     /* Windows left in spike period? */
      {
        for (i = 0; i < nThings; i++)        /* Determine which cells   */
        {                                        /* will receive this pulse */
          r = RN->Rand ();
          if (r < Data [DataIdx [i]])
            WillFire [nFire++] = ThingPtrs [i];
        }
      }
    }
  
    if (nFire > 0)               
    {
      MsgBus->SendStimMsg (TimeStep, nFire, WillFire, MsgType, AMP_Start);
      iPTicks--;
      if (iPTicks == 0) nFire = 0;
    }
  
    iFwTick--;
    if (iFwTick == 0)         /* if end of firing window, and haven't */
    {                         /* finished all windows, start another  */
      iFireWin--;
      iFwTick = FireTicks;
    }
  }

  //iSpTick--;
  iSpTick++;
}

/*------------------------------------------------------------------------*/
/* Sends a pulse of Stimulus messages to specified cells.  The pulse lasts   */
/* for Width timesteps, and is sent FREQ_Start times per second.  The pulse  */
/* amplitude may change, but this is not tested.                             */

void Stimulus::PulseStimulus (int TimeStep)
{
  if (InPulse)
  {
    if (PulseAmp != 0.0)     /* calculate pulse value based on timestep and AMP */
      MsgBus->SendStimMsg (TimeStep, nThings, ThingPtrs, MsgType, PulseAmp);
    
    PulseAmp += AMP_Increment;
    iSpTick--;
    if (iSpTick == 0) InPulse = false;
  }

  iPTicks--;
  if (iPTicks <= 0)
  {
    InPulse  = true;
    PulseAmp = AMP_Start;
    iSpTick  = SpikeTicks;
    iPTicks  = PulseTicks;
  }
}

/*----------------------------------------------------------------------------*/
/* Send a stimulus that changes at a constant rate with time.                 */

void Stimulus::LinearStimulus (int TimeStep)
{
  MsgBus->SendStimMsg (TimeStep, nThings, ThingPtrs, MsgType, PulseAmp);
  PulseAmp += AMP_Increment;
}

/*---------------------------------------------------------------------------*/
/* Send a stimulus varies with a sine function.                              */

void Stimulus::SineStimulus (int TimeStep)
{
  double Value;
  int i;
  
  for (i = 0; i < nThings; i++)
  {
    Value = VERT_TRANS + AMP_Start * SineTable [CellTablePosition [i]++]; //look up cell and advance it to next position
    MsgBus->SendStimMsg (TimeStep, 1, &(ThingPtrs [i]), MsgType, Value);

    // check to see if the cell has reached end of sine lookup table
    if (CellTablePosition [i] >= NI->FSV) CellTablePosition [i] -= NI->FSV;

  }
}

/*------------------------------------------------------------------------*/

void Stimulus::OngoingStimulus (int TimeStep)
{

}

/*------------------------------------------------------------------------*/

void Stimulus::NoiseStimulus (int TimeStep)
{

}

/*------------------------------------------------------------------------*/

void Stimulus::CalculateAMPIncrement ()
{
  double NumPulses, AMPDiff, AmpDuration, FinalEnd;
  int i, temp;

  if ((PulseTicks == 0) || (AMP_Start == AMP_End))
  {                
    AMP_Increment = 0.0;
  }
  else
  {
    i = nTimes - 1;             // this is index of last element
    FinalEnd = TStop [i];       // very last TIME_End value
    NumPulses = AMPDiff = AmpDuration = 0.0;
  
  // num_spaces is calculated as per a one second interval, as we use the frequency as cycles per one second.
  // everything is adjusted back from there
  
    AMPDiff = fabs (AMP_End - AMP_Start);
  
  // amp duration is over the whole cycle, from very first TIME_Start to very last TIME_End
  
    if (TStart == 0)
    {
      printf ("Stimulus::CalculateAMPIncrement: WARNING - TStart vector for STIMULUS %d is empty.\n", id);
      AmpDuration = 0;
    }
    else
      AmpDuration = FinalEnd - TStart [0];
  
    NumPulses = FREQ_Start * AmpDuration;
    temp = (int) NumPulses - 1;
  
    if (NumPulses != 0)
      AMP_Increment = (double) (AMPDiff / temp);
    else
      AMP_Increment = 0.0;
  }
}

/*------------------------------------------------------------------------*/

void Stimulus::PrintCellList ()
{
//ADDRESS *A;
//int i;

//printf ("Stimulus::PrintCellList: %d cells in list\n", nCells);
//for (i = 0; i < nCells; i++)
//{
//  A = &(Cells [i]);
//  printf ("  %4d: (%3d, %3d, %d)\n", i, A->Node, A->Cell, A->Cmp);
//}
}

/*---------------------------------------------------------------------------*/

void Stimulus::Error1 (T_STINJECT *iStInj)
{
  char msg [1024];

  sprintf (msg, "Stimulus:: Error, no cells found for STIMULUS_INJECT '%s'\n",
           iStInj->L.name);

  printf ("%s", msg);
  fprintf (stderr, "%s", msg);
}

/*---------------------------------------------------------------------------*/

void Stimulus::Error2 (T_STINJECT *iStInj, int ncells)
{
  int n;
  char msg [1024];

  n = sprintf (msg, "Stimulus:: Error, number of cells found for STIMULUS_INJECT '%s' (%d)\n",
               iStInj->L.name, ncells);
  n = sprintf (msg + n, "    is less than FREQ_ROWS (%d) * CELLS_PER_FREQ (%d)\n",
               nFreqs, CellsPerFreq);

  printf ("%s", msg);
  fprintf (stderr, "%s", msg);
}

/*---------------------------------------------------------------------------*/

void Stimulus::EmptyFunc (int TimeStep)
{
  if( !(TheBrain->flag & WARNINGS_OFF_FLAG) )
    printf ( "Warning: Stimulus %s: Empty Function called, stimulus not applied\n",
             NI->AR->Stimulus[idx]->L.name );
}

/*-----------------------------------------------------------------------------

This module handles external input to the brain.  This can be supplied in a 
number of ways: generated internally (as random noise, or in several 
distributions), read from a data file (FILEBASED type), or taken in real time 
from a socket (INPUTSTIM).

Regardless of the source of the input, the delivery is handled in the same way. 
Each node is responsible for delivering stimulus messages for the cells that are 
on it, and only those cells.  The stimulus module delivers the message directly 
to the destination cell/cmp's incoming message queue, bypassing the MessageBus 
module.  This is because the stimulus must generally be delivered "now" - that 
is, at the same timestep in which it was generated or received - while the 
MessageBus is designed around the premise that a message will need to be 
delivered no sooner than the next timestep.


6/13/02-JF: Added file-based stimulus that works in parallel.  A possible 
problem is that because of the parallel assignment of stimulus to cells (see 
MakeObject), the output voltages may not be reported in the same order as the 
frequency bands in the stimulus input file.

7/15/02-JF: Modified to allow dynamic creation & deletion of Stimulus objects. 
Each object is a member of the Brain's StimList list.  The object adds itself 
when created, and deletes itself when finished.  (DoStimulus returns a code to 
the caller to tell it when to call the destructor.)

There is a possible problem related to the FILEBASED stimulus.  The T_STIMULUS 
input structure holds a pointer to the data that is read from the file.  Since 
multiple Stimulus objects can be created from the same T_STIMULUS, the question 
is when or if the allocated memory should be freed.

-----------------------------------------------------------------------------*/

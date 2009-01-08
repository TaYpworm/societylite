#ifndef STIMULUS_H
#define STIMULUS_H

/* See description in .cpp file */

#include <math.h>

#include "InitStruct.h"
#include "Message.h"
#include "Random.h"
#include "defines.h"

using namespace std;

class Random;

/**
 * The Stimulus class represents external input to the brain.  This can be supplied in a 
 * number of ways: generated internally (as random noise, or in several 
 * distributions), read from a data file (FILEBASED type), or taken in real time 
 * from a socket (INPUTSTIM).
 * 
 * Regardless of the source of the input, the delivery is handled in the same way. 
 * Each node is responsible for delivering stimulus messages for the cells that are 
 * on it, and only those cells.  The stimulus module delivers the message directly 
 * to the destination cell/cmp's incoming message queue, bypassing the MessageBus 
 * module.  This is because the stimulus must generally be delivered "now" - that 
 * is, at the same timestep in which it was generated or received - while the 
 * MessageBus is designed around the premise that a message will need to be 
 * delivered no sooner than the next timestep.
 */
class Stimulus
{
 public:
  int id;       /* For debugging */
  int Port; //Jim - hopefully temporary
            ///Flag to indicate that this stimulus will have start/end time dynamically added
  int Dynamic;
          ///identifier to be able to tell which T_STIMULUS object this is derived from (if any)
  int idx;
          ///Linked list pointer to the next stimulus object
  Stimulus *next;
          ///Linked list pointer to the previous stimulus object
  Stimulus *prev;
          ///Information used by input stims to collect its data
  void *II;
          ///Function pointer to the appropriate stimulus type
  void (Stimulus::*Func) (int);
          ///Random Number generator
  Random *RN;

/* Defines a 2D array of cells which may potentially receive an input        */
/* stimulus.  Each row corresponds to CellsPerFreq cells, and has a          */
/* corresponding entry in the 1D nCells and Input arrays.                    */
          ///Number of frequencies (items) in input data
  int nFreqs;
          ///Number of cells assigned to each frequency
  int CellsPerFreq;
          ///Number of rows in data - 1 for INPUT
  int nDataRows;
          ///Pointer to stimulus data
  float *Data;
          ///Number of receiving objects (Compartments, usually)
  int nThings;
          ///Pointers to the objects that can receive stimulus
  char **ThingPtrs;
          ///Each object destined to receive stimulus will access the data array at a particular index
  int *DataIdx;            /* Index of data corresponding to each object */

// all from input file
          ///MODE determines if stimulus is applied as a current (MSG_STIMC) or as a voltage clamp (MSG_STIMV)
  int MsgType;
          ///Options include LINEAR, SINE, ONGOING, PULSE, NOISE
  int PATTERN;
          ///Options include EXACT, URAND, POISSON
  int TIMING;
          ///A stimulus uses this value for initial strength
  double AMP_Start;
          ///If a stimulus decays over time, the final current amp
  double AMP_End;
          ///The number of time intervals when this stimulus is applied
  int nTimes;
          ///The time interval this stimulus is applying
  int iTime;
          ///Array of start time values
  int *TStart;
          ///Array of stop time values
  int *TStop;

/* Used for file-based only */
          ///Number of cells from list that will get stim in the active firing window
  int nFire;
          ///List of cells that will fire within firing window
  char **WillFire;
          ///Number of timesteps within a SpikeProb window
  int SpikeTicks;
          ///Number of timesteps within a single firing window of a spike window
  int FireTicks;
          ///Number of firing windows in a spike window
  int nFireWin;
          ///Number of timesteps a pulse will fire in the window
  int PulseTicks;
  
          ///Row of data being used for current window
  int iRow;
          ///Current tick within firing window
  int iFwTick;
          ///Current tick within spike window
  int iSpTick;
          ///Current tick within pulse width
  int iPTicks;
          ///Firing window counter
  int iFireWin;
          ///Used for some stimuli as frequency of the stimulus pulse, but is really used in only initialization - so it can probably be removed as a class variable
  double FREQ_Start;
          ///This isn't used anywhere from what I can tell
  double FREQ_End;
          ///Used by sine stimulus to adjust the vertical translation of the stimulus waveform
  double VERT_TRANS;
  
          ///Used by INPUT stimulus to scale the data read according to the Dynamic Range
  double inputScalar;
          ///Used by INPUT stimulus to translate the data read according to the Dynamic Range's Minimum value
  double inputTranslation;

// now all variables that will be calculated as functions of the FSV and DURATION for the brain

  bool Active;              // true if TimeStep is within one of the specified time intervals
                            
  //pulse

  bool InPulse;
  int Time_Elapsed;            // how far into the stimulus are we? used to determin width and values
  double PulseAmp;             // currently used AMP value, starts at AMP_Start
  double AMP_Increment;        // for variable amplitudes, only, otehrwise zero

          ///Used by sine stimulus to keep track of the eleapsed angle
  double Theta;
          ///Used by sine stimulus to give a starting angle (rather than always starting at zero )
  double Phase;
          ///Increment added to the angle at each time step
  double dTheta;
          ///A look up table for SINE values to reduce number of computations
  double *SineTable;
          ///Each cell will advance through the SINE table from different starting points
  int *CellTablePosition;

// functions

  Stimulus (T_STIMULUS *, T_STINJECT *, Random *);
 ~Stimulus ();                                      

  int GetInputCells (T_STINJECT *);
  int GetCells (T_STINJECT *);
  void ResetStim ();
  bool DoStimulus (int);

  void FileStim (int);
  void InputStim (int);
  void OngoingStimulus (int);
  void NoiseStimulus (int);
  void PulseStimulus (int);
  void LinearStimulus (int);
  void SineStimulus (int);

  void CalculateAMPIncrement ();

  void EmptyFunc (int);
  void Error1 (T_STINJECT *);
  void Error2 (T_STINJECT *, int);
  void PrintCellList ();
};

#endif

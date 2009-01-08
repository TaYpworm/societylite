#ifndef COMPARTMENT_H
#define COMPARTMENT_H

#include "Brain.h"
#include "Channel.h"
#include "Synapse.h"
#include "Cell.h"
#include "Message.h"
#include "SendList.h"
#include "ActiveSynPtr.h"
#include "RandomManager.h"
#include "defines.h"
#include "parse/arrays.h"
#include "CircleList.h"
#include <vector>
#include <queue>
#include <deque>

using namespace std;

class Cell;
class Channel;
class Synapse;

//Doc++ include statements
//@Include: Synapse.h
//@Include: Channel.h
//@Include: SendList.h
//@Include: CircleList.h

/**
 * The compartment is where the actual computation of cell activity takes    
 * place.  It computes the internal membrane potential, and when this        
 * reaches a critical level, generates an action potential (AP).  The        
 * compartment may contain channels, the internal computations of which are  
 * part of the dynamics.                                                     

 * In addition to the channels, the compartment contains two other lists of  
 * objects.  The SendTo list contains the addresses of the cells which       
 * receive a message whenever the cell generates its AP.  The SynapseList is 
 * a list of Synapse objects, each of which represents a connection to       
 * another cell from which this cell can receive an AP.                      

 * Cells may potentially include more than one compartment.  If & when this  
 * is implemented, the compartments within a cell will need to exchange      
 * information.  This should NOT be done via the MessageBus, for efficiency. 
 */
class Compartment
{
 public:
          ///For debug, the input structure which defines cmp
  //T_CMP *source; 
          ///ID of the cell that owns this cmp
  int CellID;
          ///ID of this cmp within the parent cell
  int CmpID;

          ///Pointer to base of linked list
  Message *IncomingMsgs;

// Voltage Current data
          ///the resting membrane voltage of the compartment
  double VoltageMembraneRest;
          ///the membrane voltage Vm
  double VoltageMembrane;

//for the channel to use...
          ///Concentration of Calcium particle internal to the compartment
  double CalciumInternal;
          ///Concentration of Calcium particle external to the compartment
  double CalciumExternal;
          ///Change in Calcium due to spike
  double CaSpikeInc;
          ///Calcium particle resistance to change
  double Ca_Persistence;
          ///Calcium time constant
  double Calcium_Tau;

          ///used?
  int maxSendTo;
          ///The number of compartments ~this~ send to
  int nSendTo;
          ///Linked list of information concerning compartments this one sends to
  SENDITEM *SendTo;

// spike info

          ///number of voltages in spike
  int nSpikeShape;
          ///Array of voltage values that make up spike shape
  double *SpikeShape;

// Channel Objects - list of channels owned by this cmp

          ///Number of channels inside this
  int nChannels;
          ///Array of pointers to channel objects
  Channel **vChannels;

/* This is a list of all synapses owned by the cmp.  Only a fraction will be */
/* active (in a PSG waveform) at any time.  For efficiency, those synapses   */
/* that do positive learning come first in the list, so the program only has */
/* to loop through those when learning is done, not test evey one.           */

          ///Total synapses
  int nSynapse;
          ///Number that do positive learning
  int nPosLearnSynapse;
          ///Starting index of other synapses
  int iOtherSynapse;
          ///Linked list of pointers to synapse objects
  Synapse **SynapseList;   

/* This is a list of the synapses that are in the process of delivering a    */
/* PSG waveform.  Any particular synapse may be in the list several times,   */
/* at different points of the waveform.                                      */

#ifdef SAME_PSC
  int nActiveIn;
  ACTIVESYN *ASP;             /* operations faster */
#endif
  ACTIVESYN *ActiveList;
        
// Firing Data - TimeStep
          ///Number of timesteps since last calc
  int nDT;
          ///time of the most recent Action Potential
  int LastFireTime;
          ///Compartment designed to integrate and fire. i.e. has spike shape, threshold
  bool Active;
          ///Countdown from SpikeSize value to 0, when < 0 the refractory period is over
  int RefractoryTime;
          ///dt/C where dt = 1/FSV and C = Tau_Membrane_mean/R_Membrane_mean
  double dt_C;
          ///Calculated Capacitance of compartment - farads
  double Capacitance;

  //these next four summed together form NetCurrent

          ///i_leak in the integration function
  double Current_Leak;
          ///Sum of the current for all the channels
  double TotalChannelCurrent;
          ///Sum of the current for all the synapses
  double TotalSynapseCurrent;
          ///Sum of the current for all the stimulus values at this time step
  double TotalStimulusCurrent;
          ///Current coming in from adjacent compartments within this cell
  double TotalAdjacentCompartmentCurrent;
          ///netI - total of the four previous summed together, subtracting Current_Leak
  double NetCurrent;
          ///When true, it signals that we are to use a different equation for the membrane voltage
  bool VoltageClamp;
          ///Membrance voltage to use when in a voltage clamp
  double StimulusVoltage;

  // Bio-Membrane Data (Ca, Na, K)
  
  double Tau_Membrane;
  double R_Membrane;
          ///When compartment reaches threshold, a spike is generated and transmitted along all synapses out
  double Threshold;
 
  double Leak_Reversal;
  double SPIKE_HALFWIDTH;
          ///Conductance of current (I) leak
  double Leak_G;
          ///Each timestep 1/FSV
  double dt;
          ///Deacy factor for the membrane voltage
  double Persistance;
          ///The delay of synapse transmission after being converted to ticks from seconds
  int CompartmentDelay;

  /**
   * Data Structure to keep track of a compartment's previous states.
   * Currently only tracks compartment voltages.  The size of the buffer is
   * determined by how great of a connection delay exists between this
   * compartment and all other compartments connected to it.
   */
  CircleList stateBuffer;

          ///Number of compartment connections that use this compartment as a source
  int nForward;
          ///Number of compartment connections that use this compartment as a destination
  int nReverse;
          ///Array of pointers to the forward connections (stored in the original input)
  T_CMPCONNECT **forwardConn;
          ///Array of pointers to the reverse connections (stored in the original input)
  T_CMPCONNECT **reverseConn;

          ///Spike shape index into ARRAYS structure
  int SpikeIndex;

//********************functions*****************************

          ///default constructor
  Compartment ();
  /**
   * Primary constructor used: called within BuildBrain.cpp
   * @param iCmp Intermediate object generated from input containing compartment template
   * @param CELLID unique value identifying this compartment's parent cell
   * @param CMPID unique value identifying this compartment among parent cell's compartment list
   * @param kdx index of iCmp in parsed data AR
   * @param RM Manager of random number generators
   * @param AR User input data in parsed and structured format
   */
  Compartment (T_CMP *iCmp, int CELLID, int CMPID, int kdx, RandomManager *RM, ARRAYS *AR);
          ///Destructor must free channels, synapses, SendTo
  ~Compartment (); 
  /**
   * Main compartment processing function.  Calls all synapses and channels of this compartment
   * @param TimeStep iteration of Brain::DoThink
   */
  void DoProcessCompartment (int TimeStep);
  /**
   * Handles processing of a spike message from another cell.  It adds the synapse
   * to the list of active synapses by getting an ACTIVESYN pointer from the pool,
   * filling it with pointers to the Synapse and PSC templates, and setting the count.
   * At each subsequent timestep, the value of template [count] will be added to the
   * total, and the count decremented.  When it reaches zero, the PSC is over, and the
   * synapse becomes inactive again.
   * Note that any particular synapse may be in the active list several times, 
   * since a new spike may start (spike msg is received) before a prior one   
   * has reached the end of the template.  Thus there may be several ongoing  
   * PSCs at different points in the template.
   * @param Syn spiking synapse
   * @param Value Synaptic value
   */
  void AddSynapseToActive (Synapse *Syn, double Value);
  /**
   * Interface provided for MessageBus to give this compartment Stimulus Current (I)
   * @param Msg Message contain current in field StimC.Current
   */
  void ReceiveStimC (Message *Msg);
  /**
   * Interface provided for MessageBus to give this compartment Stimulus Voltage (V)
   * @param Msg Message containing voltage in field StimV.Voltage
   */
  void ReceiveStimV (Message *);
  /**
   * Sums contributions from all active synapses to give a total synapse current.
   * Checks each synapse to see if it has reached the end of the template, and
   * if so, removes it from the active list.
   * @return final calculated synapse current (I)
   */
  double GetSynapseCurrent (void);
  /**
   * Check to see if the threshold has been reached, and if so fire an action
   * potential with the designated spike pattern for this compartment
   * @param TimeStep iteration of Brain::DoThink
   */
  void Fire (int);
  /**
   * Go through designated synapses and check for conditions that allow for Hebbian
   * learning.
   * @param TimeStep iteration of Brain::DoThink
   */
  void DoPositiveLearning (int TimeStep);
  /**
   * @return compartment's membrane voltage
   */
  double GetMembraneVoltage ();
  /**
   * @return concentration of Calcium internal to this compartment
   */
  double GetCalciumInternal ();
  /**
   * @return concentration of Calcium external to this compartment
   */
  double GetCalciumExternal ();
  /**
   * @return 1/FSV
   */
  double GetDT ();

// channel interface functions

  double UpdateChannelsAndGetCurrentValue (int); 
  void UpdateCalciumInternal ();
  void UpdateCalciumWithSpike ();

  // Report Functions -----------------------------------------------------
  /**
   * Not defined?
   */
  void DoReport (Cell *, int);

  /**
   * Number of items reported on for a particular channel family
   * @param family Integer code of family type
   * @return total number of items 
   */
  int TotalItems (int family);

  /**
   * Count up total number of channels in this compartment belonging to a particular family
   * @param family Integer code of family type to count
   * @return number of channels matching specified family code
   */
  int NumFamily (int family);

  /**
   * Populate data buffer with specified channel's information for reporting
   * @param family Integer code of family type to report on
   * @param buf data buffer to store information on
   * @param nitems address where final number of data items written into buffer, is dereferenced in function
   */
  void ReportChannel (int family, float *buf, int *nitems);

  /**
   * Write compartment data to a file stream. This is only the first half of the
   * compartment save. It saves data for this individual compartment and not any
   * connection information (synapses, SendTo). That must be saved later for
   * purposes of future loading.
   * @param out the output stream
   * @return the number of bytes written
   */
  int Save (FILE *out );

  /**
   * Finish writing compartment data to a file.  This is the second half of the
   * compartment save. It is called after all individual cell and compartments have 
   * been written.  This is important to the loading of a brain state since memory
   * addresses will need to be assigned and that cannot be done until all memory has
   * been allocated.
   * @param out File pointer to destination file.
   * @return number of bytes written
   */
  int SaveConnections( vector<Synapse *> &collection, int targetNode );//FILE *out );

  /**
   * Read compartment data from a file stream
   * @param in the input stream
   * @param nothing dummy string in case pointers need something to point to. Will hopefully be phased out.
   */
  void Load (FILE *in, char *nothing);

  /**
   * Finish loading compartment data from a file.  This loads the second half of 
   * compartment data after all cells and compartments have been loaded.  This
   * is important because connection data requires that all memory be allocated
   * so that pointers to synapstic sources and destinations can be configured.
   * This function goes through its SendTo list. If any entries are for the target Node,
   * it will take the first address of the queue.
   * @param collection synapse memory addresses
   * @param targetNode Node ID where the memory addresses are from.
   */
  void LoadConnections( deque<Synapse*> &collection, int targetNode );
  void CmpReset (FILE *, char *);
  
  /**
   * Set variables for Forward Connections (i.e. this compartment is the source )
   * @param nConnect the number of connections
   * @param connections pointer to array of pointers, each of which points to a connection
   */
  void SetForwardConnections( int nConnect, T_CMPCONNECT **connections );
  
  /**
   * Set variables for Reverse Connections (i.e. this compartment is the destination )
   * @param nConnect the number of connections
   * @param connections pointer to array of pointers, each of which points to a connection
   */
  void SetReverseConnections( int nConnect, T_CMPCONNECT **connections );

  /**
   * Calculate the current adjustment in this compartment due to connections
   * with adjacent compartments
   * @param TimeStep the current TimeStep of the simulation
   */
  void ProcessAdjacentCompartments( int TimeStep );

  /**
   * Allocate the minimum number of elements for the stateBuffer.
   * @param size the number of previous compartment states that must be retained
   */
  void InitBuffer( int size );

  /**
   * Insert the latest Compartment state into the stateBuffer, overwriting 
   * the oldest state if the buffer is full
   */
  void UpdateBuffer();

  /**
   * Helper function to set the TotalStimulusCurrent value to zero after 
   * the Brain has finished reporting in the DoThink loop.  This allows
   * the MessageBus to directly add any new Stimulus Current to the
   * Compartment so that it accumulates over potentially several stimuli.
   */
  void PrepareStimulusCurrent();
};

#endif

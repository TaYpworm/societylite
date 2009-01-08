/* Redone for better learning, etc.  See comments at end.  8/03 - JF         */

#include <stdlib.h>
#include <stdio.h>

#include "Compartment.h"
#include "Managers.h"
#include "debug.h"
#include "memstat.h"

#define MEM_KEY  KEY_CMP

Compartment::Compartment ()
{
  vChannels   = NULL;
  nChannels   = 0;
  SynapseList = NULL;
  nSynapse    = 0;
  SendTo      = NULL;
}

/* Note: kdx in the arg list is index of iCmp in AR.  It needs to be passed  */
/* to get the Random object, but should go away when Random handling is      */
/* fixed - JF                                                                */

Compartment::Compartment (T_CMP *iCmp, int CELLID, int CMPID, int kdx, 
                          RandomManager *RM, ARRAYS *AR)
{
  T_SPIKE *iSpike;
  T_CHANNEL *iChan;
  Random *RN;
  int i, idx;

  MEMADDOBJ (MEM_KEY);

//source = iCmp;            /* Debugging info */
  CellID = CELLID;
  CmpID  = CMPID;
  RN     = RM->GetCmpRand (kdx);

  IncomingMsgs = NULL;

//printf ("Compartment::Compartment: making '%s'\n", AR->Compart [idx].name);
  LastFireTime   = -9999; 
  RefractoryTime = -1;

  VoltageMembrane = VoltageMembraneRest = RN->GaussNum (iCmp->VMREST);

  //printf ("Cell %d: VoltageMembrane = %f\n", CellID, VoltageMembrane);

/* a compartment is active only if it has a spike shape, halfwidth and       */
/* threshold.  Other wise it is considered a passive compartment that merely */
/* passes along information instead of doing the integrate and fire          */
/* calculations.  (Relevant in new model? - JF)                              */

  SpikeIndex = -1;
  if (iCmp->Active)
  {
    Active          = true;
    SPIKE_HALFWIDTH = RN->GaussNum (iCmp->Spike_HW);
    Threshold       = RN->GaussNum (iCmp->Threshold);

    iSpike          = AR->Spike [iCmp->Spike];
    SpikeIndex      = iCmp->Spike;
    nSpikeShape     = iSpike->nVoltages;
    SpikeShape      = iSpike->Voltages;
  }
  else 
  {
    Active          = false;
    SPIKE_HALFWIDTH = 0.0;
    Threshold       = 0.0;

    nSpikeShape     = 0;
    SpikeShape      = NULL;
  }

  Tau_Membrane  = RN->GaussNum (iCmp->Tau_Membrane);
  Leak_Reversal = RN->GaussNum (iCmp->Leak_Reversal);
  Leak_G        = RN->GaussNum (iCmp->Leak_G);
  R_Membrane    = RN->GaussNum (iCmp->R_Membrane);
  CaSpikeInc    = RN->GaussNum (iCmp->CaSpikeInc);
 
// calculate time constant -> dt/C where dt = 1/FSV and C = Tau_Membrane_mean/R-Membrane_mean

  if (NI->FSV != 0.0)
    dt = 1.0 / (double) NI->FSV;
  else 
    dt = 0.0;
  nDT = 0;

  if (R_Membrane != 0.0)
  {
    Capacitance = Tau_Membrane / R_Membrane;
    dt_C = dt / Capacitance;
  }
  else 
  {
    Capacitance = dt_C = 0.0;
  }

  if (Tau_Membrane != 0.0)
    Persistance =   1.0 - (dt / Tau_Membrane);     // typically this is 1 - dt/Tau_mem, but
  else                                             // resting membrane is negative here, not 0
    Persistance = 1.0;

  Current_Leak         = 0.0;
  TotalChannelCurrent  = 0.0;
  TotalSynapseCurrent  = 0.0;
  TotalStimulusCurrent = 0.0;
  NetCurrent           = 0.0;
  VoltageClamp         = false;
  CompartmentDelay     = 0;   //  temp value for now, will calculate it later

 // channel data

  CalciumInternal = RN->GaussNum (iCmp->CaInt);
  CalciumExternal = RN->GaussNum (iCmp->CaExt);
  Calcium_Tau     = RN->GaussNum (iCmp->CaTau);


  if (Calcium_Tau == 0.0)
    Ca_Persistence = 1.0;
  else
    Ca_Persistence = (double) (1.0 - dt / Calcium_Tau);
 
  if (Ca_Persistence > 1.0)
    Ca_Persistence = 1.0;
  if (Ca_Persistence < 0.0)
    Ca_Persistence = 0.0;

  nSynapse     = nPosLearnSynapse = 0;
  SynapseList = NULL;
  nSendTo      = 0;
  SendTo       = NULL;
  ActiveList   = NULL;
#ifdef SAME_PSC
  nActiveIn   = 0;
#endif
  
/* Create channels for this compartment, as specified in iChan */
  
  nChannels = iCmp->nChannels;
  if (nChannels <= 0)
    vChannels = NULL;
  else
    vChannels = (Channel **) calloc (nChannels, sizeof (Channel *));

  for (i = 0; i < nChannels; i++)
  {
    idx   = iCmp->Channels [i];
    iChan = AR->Channel [idx];
    RN    = RM->GetChannelRand (idx);
    vChannels [i] = new Channel (iChan, RN);
  }

  nForward = nReverse = 0;
  forwardConn = reverseConn = NULL;

//  printf ("Compartment %d: Initial values\n", CellID);
//  printf ("    VoltageMembrane = %f\n", VoltageMembrane);
//  printf ("    Tau_Membrane  = %f\n", Tau_Membrane );
//  printf ("    Leak_Reversal = %f\n", Leak_Reversal);
//  printf ("    Leak_G        = %f\n", Leak_G);
//  printf ("    R_Membrane    = %f\n", R_Membrane);
//  printf ("    CaSpikeInc    = %f\n", CaSpikeInc);
}

/*------------------------------------------------------------------------*/

Compartment::~Compartment ()
{
  int i;

  MEMFREEOBJ (MEM_KEY);
  for (i = 0; i < nChannels; i++) //delete all channels
    delete vChannels [i];
  if (vChannels != NULL) free (vChannels);

  for (i = 0; i < nSynapse; i++) //delete all synapses
    delete SynapseList [i];
  if (SynapseList != NULL) free (SynapseList);

  if (SendTo != NULL) free (SendTo);
}

/*---------------------------------------------------------------------------*/
/* This is the main processing loop of the compartment.  It calls all the    */
/* syapses and channels owned by the compartment                             */

void Compartment::DoProcessCompartment (int TimeStep)
{
  nDT++;
  TotalAdjacentCompartmentCurrent = 0.0;
  double targetI;

/* Update the channels with current Membrane Voltage.  Changes the channels' */
/* internal state, so must be called even during refractory period.          */
  TotalChannelCurrent = UpdateChannelsAndGetCurrentValue (TimeStep);  

/* If cmp is refractory or the voltage is clamped by a stim, then the        */
/* membrane voltage is forced to the spike or stim value, so there's no need */
/* to do the rest of the calculations.                                       */
  NetCurrent = 0;

  if (RefractoryTime >= 0)
  {
      VoltageMembrane = SpikeShape [RefractoryTime--];
  }
  else 
  {
      Current_Leak = Leak_G * (VoltageMembrane - Leak_Reversal);
      TotalSynapseCurrent = GetSynapseCurrent ();

      ProcessAdjacentCompartments( TimeStep );
      
      NetCurrent = TotalChannelCurrent + TotalSynapseCurrent + TotalStimulusCurrent  
                 + TotalAdjacentCompartmentCurrent - Current_Leak;

      //If Voltage Clamp, find the difference beween my current voltage and
      //clamped voltage.  How much current (I) is required to maintain that?
      //How much current (I) is needed to enhance/counter-balance the current (I) flowing in?
      if( VoltageClamp )
      {
        targetI = ( StimulusVoltage - VoltageMembraneRest -
                   (VoltageMembrane-VoltageMembraneRest)*Persistance )/dt_C;
        NetCurrent = targetI-NetCurrent;
        VoltageMembrane = StimulusVoltage;
        VoltageClamp    = false;
      }
      else
        VoltageMembrane = VoltageMembraneRest
                        + (VoltageMembrane - VoltageMembraneRest) * Persistance
                        + dt_C * NetCurrent;

/* If this cmp is active, then check to see if firing threshold is reached,  */
/* and if so, fire.                                                          */

    if (Active && (VoltageMembrane >= Threshold))
      Fire (TimeStep);
    nDT = 0;
  }

/* The stimulus must be set to zero AFTER the processing loop.  Stimulus     */
/* msgs are delivered directly to the destination compartment by the         */
/* MessageBus (that is, the ReceiveStimX function for the compartment is     */
/* called by the MessageBus::DeliverMsgs function), and this happens before  */
/* the DoProcessCmp function is called.                                      */

   /*
    Introduced function 'PrepareStimulusCurrent' to be called after Reporting
    in DoThink loop completes.  This allows The value to be reset to zero
    before the MessageBus starts sending messages, but allows a stimulus
    report to display how much stimulus current was injected during a timestep.
   */

  //TotalStimulusCurrent = 0.0;
#ifdef SAME_PSC
  nActiveIn   = 0;
#endif

}

/*---------------------------------------------------------------------------*/

void Compartment::PrepareStimulusCurrent()
{
  TotalStimulusCurrent = 0.0;
}

/*---------------------------------------------------------------------------*/

void Compartment::ReceiveStimV (Message *Msg)
{
  StimulusVoltage = Msg->StimV.Voltage;
  VoltageClamp    = true;
}

/*---------------------------------------------------------------------------*/

void Compartment::ReceiveStimC (Message *Msg)
{
  TotalStimulusCurrent += Msg->StimC.Current;
}

/*---------------------------------------------------------------------------*/
/* Check to see if the threshold has been reached, and if so fire an action  */
/* potential with the designated spike pattern for this compartment          */

void Compartment::Fire (int TimeStep)
{
  //int i;

  RefractoryTime  = nSpikeShape - 1;
  VoltageMembrane = SpikeShape [RefractoryTime--];

  UpdateCalciumWithSpike ();         // must do this at the beginning of the spike shape
  MsgBus->SendSpikeMsgs (TimeStep, CompartmentDelay, nSendTo, SendTo);

  DoPositiveLearning (TimeStep);         // update synapses that contributed to this spike firing

  LastFireTime = TimeStep;
}

/*------------------------------------------------------------------------*/
/* Note that the Connector places all synapses that do positive learning at  */
/* the head of the SynapseList, so this routine only needs to look at those. */

void Compartment::DoPositiveLearning (int TimeStep)
{
  Synapse *Syn;
  int i;

  for (i = 0; i < nPosLearnSynapse; i++)
  {
    Syn = SynapseList [i];
    Syn->DoPositiveLearning (TimeStep);
  }
}

/*------------------------------------------------------------------------*/

double Compartment::GetMembraneVoltage ()
{
  return VoltageMembrane;
}

/*------------------------------------------------------------------------*/

double Compartment::GetCalciumInternal ()
{
  return CalciumInternal;
}

/*------------------------------------------------------------------------*/

double Compartment::GetCalciumExternal ()
{
  return CalciumExternal;
}

/*------------------------------------------------------------------------*/

double Compartment::GetDT ()
{
  return (dt);
}


/***************** CHANNEL INTERFACE FUNCTIONS *******************************/

/*---------------------------------------------------------------------------*/
/* Loop thru all the channels owned by this compartment and get the current  */
/* value for this timestep                                                   */

double Compartment::UpdateChannelsAndGetCurrentValue (int TimeStep)
{
  double Current;
  int i;

  Current = 0.0;
  UpdateCalciumInternal ();

  for (i = 0; i < nChannels; i++) 
  {
    Current += vChannels [i]->GetCurrent (this);
  }

  return (Current);
}

/*------------------------------------------------------------------------*/

void Compartment::UpdateCalciumInternal ()
{
  CalciumInternal *= Ca_Persistence;
}

/*------------------------------------------------------------------------*/

void Compartment::UpdateCalciumWithSpike ()
{
  CalciumInternal += CaSpikeInc;
}

/*---------------------------------------------------------------------------*/
/* This handles the processing of a spike message from another cell.  It     */
/* adds the synapse to the list of active synapses by getting an ACTIVESYN   */
/* pointer from the pool, filling it with pointers to the Synapse and PSC    */
/* templates, and setting the count.  At each subsequent timestep, the value */
/* of template [count] will be added to the total, and the count             */
/* decremented.  When it reaches zero, the PSC is over, and the synapse      */
/* becomes inactive again.                                                   */

/* Note that any particular synapse may be in the active list several times, */
/* since a new spike may start (spike msg is received) before a prior one    */
/* has reached the end of the template.  Thus there may be several ongoing   */
/* PSCs at different points in the template.                                 */

void Compartment::AddSynapseToActive (Synapse *Syn, double Value)
{
#ifdef SAME_PSC
  if (nActiveIn == 0)
  {
    ASP = ActiveSynMgr->GetActiveSyn ();
    ASP->PSG      = Syn->SynDef->PSGTemplate;
    ASP->Count    = Syn->SynDef->PSGLen;
    ASP->Value    = Value;
    ASP->Reversal = Syn->SynReversal;
    ASP->link     = ActiveList;
    ActiveList    = ASP;
  }
  else
  {
    ASP->Value    += Value;
    ASP->Reversal += Syn->SynReversal;
  }
  nActiveIn++;
#else
  ACTIVESYN *ASP;
 
  ASP = ActiveSynMgr->GetActiveSyn ();
  ASP->Syn   = Syn;
  ASP->Count = Syn->SynDef->PSGLen;
  ASP->Value = Value;
  ASP->link  = ActiveList;
  ActiveList = ASP;
#endif
}

/*---------------------------------------------------------------------------*/
/* This sums up the contributions from all active synapses to give a total   */
/* synapse current.  It checks each synapse to see if it has reached the end */
/* of the template, and if so, removes it from the active list.              */

double Compartment::GetSynapseCurrent  ()
{
  ACTIVESYN *A, *prev, *next;
#ifndef SAME_PSC
  Synapse *Syn;                          
#endif
  double Current;
 
  Current = 0.0;
  prev = NULL;
  A  = ActiveList;

#ifdef SAME_PSC
  if (nActiveIn > 1)
  {
    A->Reversal = A->Reversal / nActiveIn;
  }
#endif

  while (A != NULL)
  {
    A->Count -= nDT;
    if (A->Count >= 0)
    { 
#ifdef SAME_PSC
      Current += A->Value * A->PSG [A->Count] * (A->Reversal - VoltageMembrane);
#else
      Syn = A->Syn;
      Current += A->Value * Syn->SynDef->PSGTemplate [A->Count] *
                 (Syn->SynReversal - VoltageMembrane);
#endif
//    printf ("Value = %f, Template [%3d] = %f, SR = %f, VM = %f\n",
//           A->Value, A->Count, Syn->SynDef->PSGTemplate [A->Count], Syn->SynReversal, VoltageMembrane);
    }

    if (A->Count <= 0)
    { 
      if (prev == NULL)
        ActiveList = A->link;
      else
        prev->link = A->link;

      next = A->link;
      ActiveSynMgr->FreeActiveSyn (A);
      A = next;
    }
    else
    {
      prev = A;
      A  = A->link;
    }
  }

  return (Current);
}

/*---------------------------------------------------------------------------*/
/* Returns number of items this Compartment's channels report                */

int Compartment::TotalItems (int family)
{
  int i, total;

  total = 0;
  for (i = 0; i < nChannels; i++) 
  {
    if (vChannels [i]->family == family)
      total += vChannels [i]->GetNumItems ();
  }
  return (total);
}

/*---------------------------------------------------------------------------*/
/* Returns number of channels of a family in this Compartment                */

int Compartment::NumFamily (int family)
{
  int i, total;

  total = 0;
  for (i = 0; i < nChannels; i++) 
  {
    if (vChannels [i]->family == family)
      total++;
  }
  return (total);
}

/*------------------------------------------------------------------------*/

void Compartment::ReportChannel (int family, float *buf, int *nitems)
{
  Channel *Ch;
  int i;

  *nitems = 0;
  for (i = 0; i < nChannels; i++) 
  {
    Ch = vChannels [i];
    if (Ch->family == family) 
    {
      switch (family)
      {
        case FAM_NA:     
            break;
        case FAM_CA:     
            break;
        case FAM_KM:    buf [0] = (float) pow (Ch->m, Ch->mPower);
                        buf [1] = (float) Ch->m;
                        buf [2] = (float) Ch->m_oo;
                        buf [3] = (float) Ch->t_m;
                        buf [4] = (float) Ch->I;
                        *nitems = 5;
            break;
        case FAM_KA:    buf [0] = (float) pow (Ch->m, Ch->mPower) * pow (Ch->h, Ch->hPower);
                        buf [1] = (float) Ch->m;
                        buf [2] = (float) Ch->m_oo;
                        buf [3] = (float) Ch->t_m;
                        buf [4] = (float) Ch->h;
                        buf [5] = (float) Ch->h_oo;
                        buf [6] = (float) Ch->t_h;
                        buf [7] = (float) Ch->I;
                        *nitems = 8;
            break;
        case FAM_KDR:    
            break;
        case FAM_KNICD:
            break;
        case FAM_KAHP:  buf [0] = (float) pow (Ch->m, Ch->mPower);
                        buf [1] = (float) Ch->m;
                        buf [2] = (float) Ch->m_oo;
                        buf [3] = (float) Ch->t_m;
                        buf [4] = (float) CalciumInternal;
                        buf [5] = (float) Ch->funct_m;
                        buf [6] = (float) Ch->I;
                        *nitems = 7;
            break;
      }
      break;
    }
  }
}

/*------------------------------------------------------------------------*/
/* Set compartment connections moving forward  */

void Compartment::SetForwardConnections ( int size, T_CMPCONNECT **connectionArray )
{
    nForward = size;
    forwardConn = connectionArray;
}

/*------------------------------------------------------------------------*/
/* Set compartment connections moving in reverse */

void Compartment::SetReverseConnections( int size, T_CMPCONNECT **connectionArray )
{
    nReverse = size;
    reverseConn = connectionArray;
}

/*------------------------------------------------------------------------*/
/* Go to all compartments conected to this compartment and collect data   */

void Compartment::ProcessAdjacentCompartments( int TimeStep )
{
  int i;
  double sum = 0;
  double neighborVoltage = 0;
  Cell *parent;

  parent = TheBrain->Cells[CellID];

    //Add together current flowing in
    for( i=0; i<nForward; i++ )
    {
      //speed determines many timesteps into the past we need to access
      neighborVoltage = parent->Compartments[forwardConn[i]->ToCmp]->stateBuffer.at( -(int)forwardConn[i]->delay ).MVoltage;

      sum += (neighborVoltage - VoltageMembrane)*forwardConn[i]->G;  //multiply by conductance G = 1/resistance
    }

    //Subtract current flowing out
    for( i=0; i<nReverse; i++ )
    {
      neighborVoltage = parent->Compartments[reverseConn[i]->FromCmp]->stateBuffer.at( -(int)reverseConn[i]->delay ).MVoltage;
      sum += (neighborVoltage - VoltageMembrane)*reverseConn[i]->retroG;
    }
    TotalAdjacentCompartmentCurrent = sum;
}

/*------------------------------------------------------------------------*/
/* fill the circle buffer with values */

void Compartment::InitBuffer( int size=-1 )
{
   int i;
   CmpVoltage temp;
   temp.MVoltage = VoltageMembraneRest;

   if( size > 0 )
     stateBuffer.resize( size );

   for( i=0; i<stateBuffer.capacity(); i++ )
     stateBuffer.push_front( temp );
}

/*------------------------------------------------------------------------*/
/* Compartment stores its state into a circular buffer for reference */

void Compartment::UpdateBuffer()
{
  CmpVoltage temp;
  temp.MVoltage = VoltageMembrane;

  stateBuffer.push_front( temp );
}

/*------------------------------------------------------------------------*/
//write out compartment data to a file

int Compartment::Save( FILE *out )
{
  int nbytes = 0; //count of total bytes saved, to be returned at end
  int i;
  const int hSize = 18;
  const int iSize = 12;
  const int bSize = 2;

  double holder[hSize];
  int iholder[iSize];
  bool bholder[bSize];

  //I would like to just use fwrite to write a compartment to disk,
  //but this uses stl objects.  I was able to deal with that in ver3 using
  //a work around, but i would rather not do it again as I'm still not certain as to it's
  //robustness.  However I am also loathe to write out each individual data element as that
  //could take a long time and a great deal of tedious code.

  //write out everything else, put all doubles into one array, call write once
  i = 0;   //use i to count so I can add/remove lines with out editing constants
  holder[i++] = VoltageMembraneRest;
  holder[i++] = VoltageMembrane;
  holder[i++] = CalciumInternal;
  holder[i++] = CalciumExternal;
  holder[i++] = CaSpikeInc;
  holder[i++] = Ca_Persistence;
  holder[i++] = Calcium_Tau;
  holder[i++] = dt_C;
  holder[i++] = Capacitance;
  holder[i++] = StimulusVoltage;
  holder[i++] = Tau_Membrane;
  holder[i++] = R_Membrane;
  holder[i++] = Threshold;
  holder[i++] = Leak_Reversal;
  holder[i++] = SPIKE_HALFWIDTH;
  holder[i++] = Leak_G;
  holder[i++] = dt;
  holder[i++] = Persistance;
    fwrite( holder, sizeof( double ), i, out );
    nbytes += sizeof( double ) * i;

  //next: bools then ints
  i=0;
  bholder[i++] = Active;
  bholder[i++] = VoltageClamp;
    fwrite( bholder, sizeof( bool ), i, out );
    nbytes += sizeof( bool ) * i;
  i=0;
  iholder[i++] = nChannels;
  iholder[i++] = nSynapse;
  iholder[i++] = nPosLearnSynapse;
  iholder[i++] = iOtherSynapse;    //can this be calculated after load?
  iholder[i++] = SpikeIndex;       // = -1 if no spike, >= 0 if cmp has spike
  iholder[i++] = RefractoryTime; 
  iholder[i++] = CellID;
  iholder[i++] = CmpID;
  iholder[i++] = nSendTo; //-and the SendTo List-> this should be unique to each compartment
  iholder[i++] = CompartmentDelay;
  iholder[i++] = nForward;  //-the associated lists are shared with compartments of the same type,
  iholder[i++] = nReverse;  //--so those lists should be written elsewhere
//  iholder[i++] = TCmpIdx;       //--so I can find the original cmp from input
    fwrite( iholder, sizeof( int ), i, out );
    nbytes += sizeof( int ) * i;

  //test channel state save
  for( i=0; i<nChannels; i++ )
    nbytes += vChannels[i]->Save( out );

  //synapses
  for( i=0; i<nSynapse; i++ )
    nbytes += SynapseList[i]->Save( out );

  //sendto list for on Node data
  const int multiplier = 2;
  int *sendHolder = new int[ nSendTo * multiplier];
  for( i=0; i<nSendTo; i++ )
  {
    //need to save Node, Delay, and derference Addr to get CellID and CmpID
      //all are ints, so store in int array with X*nSendTo elements?
    sendHolder[multiplier*i]   = SendTo[i].Node;
    sendHolder[multiplier*i+1] = SendTo[i].Delay;
  }
  fwrite( sendHolder, nSendTo*multiplier, sizeof( int ), out );
    nbytes += sizeof( int ) * nSendTo * multiplier;
  delete [] sendHolder;

  //UPDATE - added variable SpikeIndex and saved with other integers

  //active synapses, holds pointers to synapses, need to replace(i.e. save) with index numbers

  //messages still on buffer? will there be any?

    //state buffer
  nbytes += stateBuffer.Save( out );

  return nbytes;
}

//This function gathers information for getting connection data
//If the target Node is not this one, mpi will be used,
//otherwise the data will be directly accessed
int Compartment::SaveConnections( vector<Synapse *> &collection, int targetNode )
{
  int i, nbytes = 0;
  //const int multiplier = 2;
  
  //go through SendTo list - find items that send to targetNode
  
  //SendTo List - the void* Addr points to a Synapse object on the destination cmp
  //side note - instead of giving each synapse an ID, I could calc the offset from
  //front of list - future improvement?
  for( i=0; i<nSendTo; i++ )
    if( SendTo[i].Node == targetNode )
      collection.push_back( (Synapse*) SendTo[i].Addr );

  return nbytes;
}

/*------------------------------------------------------------------------*/
//read in compartment data from a file into an existing structure
//addresses may not be the same

void Compartment::Load( FILE *in, char *nothing )
{
  const int hSize = 18;
  const int iSize = 12;
  const int bSize = 2;

  double holder[hSize];
  int iholder[iSize];
  bool bholder[bSize];

  int i;

  //read data back in same order as output

  //use one fread to get all doubles, ints, bools
  fread( holder, sizeof( double ), hSize, in );
  i = 0;   //use i to count so I can add/remove lines with out editing constants
  VoltageMembraneRest = holder[i++];
  VoltageMembrane = holder[i++];
  CalciumInternal = holder[i++];
  CalciumExternal = holder[i++];
  CaSpikeInc = holder[i++];
  Ca_Persistence = holder[i++];
  Calcium_Tau = holder[i++];
  dt_C = holder[i++];
  Capacitance = holder[i++];
  StimulusVoltage = holder[i++];
  Tau_Membrane = holder[i++];
  R_Membrane = holder[i++];
  Threshold = holder[i++];
  Leak_Reversal = holder[i++];
  SPIKE_HALFWIDTH = holder[i++];
  Leak_G = holder[i++];
  dt = holder[i++];
  Persistance = holder[i++];

  fread( bholder, sizeof( bool ), bSize, in );
  i=0;
  Active = bholder[i++];
  VoltageClamp = bholder[i++];

  fread( iholder, sizeof( int ), iSize, in );
  i=0;
  nChannels = iholder[i++];
  nSynapse = iholder[i++];
  nPosLearnSynapse = iholder[i++];
  iOtherSynapse = iholder[i++];
  SpikeIndex = iholder[i++];  // = -1 if no spike, >= 0 if cmp has spike
  RefractoryTime = iholder[i++];
  CellID = iholder[i++];
  CmpID = iholder[i++];
  nSendTo = iholder[i++]; //-and the SendTo List-> this should be unique to each compartment
  CompartmentDelay = iholder[i++];
  nForward = iholder[i++];  //-the associated lists are shared with compartments of the same type,
  nReverse = iholder[i++];  //--so those lists should be written elsewhere

  //channels
  vChannels = (Channel **) calloc ( nChannels, sizeof (Channel *));
  for( i=0; i<nChannels; i++ )
  {
    vChannels[i] = new Channel();      //is new okay? malloc instead?
    vChannels[i]->Load( in, nothing );
  }

  //spikeshapes - only one needs to be saved since multiple compartments refer to the same one
  //  so when do I save a spikeshape and how do I find it later (out of numerous possible spikeshapes? )
  //  save some index number?  When loading, non-active cmp do not need to look for a spike.
  if( SpikeIndex >= 0 )
  {
    //goto index 
    nSpikeShape = NI->AR->Spike[SpikeIndex]->nVoltages;
    SpikeShape  = NI->AR->Spike[SpikeIndex]->Voltages;
  }
  //synapses
  SynapseList = (Synapse **) calloc ( nSynapse, sizeof (Synapse *) );
  for( i=0; i<nSynapse; i++ )
  if( SynapseList )
    for( i=0; i<nSynapse; i++ )
    {
      SynapseList[i] = new Synapse();
      SynapseList[i]->Load( in, nothing );
      SynapseList[i]->Cmp = this;     //must assign cmp pointer to myself
    }

  //the SendTo list can be allocated and some data filled in,
  // but the rest will be restored later
  SendTo = (SENDITEM *) calloc( sizeof(SENDITEM), nSendTo );
  const int multiplier = 2;  //There are only two ints per SendTo item 
                             // we can restore at this point
  int *sendHolder = new int[ nSendTo * multiplier];
  fread( sendHolder, sizeof(int), multiplier*nSendTo, in );
  for( i=0; i<nSendTo; i++ )
  {
    SendTo[i].Node = sendHolder[multiplier*i];
    SendTo[i].Delay = sendHolder[multiplier*i+1];
  }
  delete [] sendHolder;

  //active synapses, holds pointers to synapses, need to replace(i.e. save) with index numbers

  //previous state buffer
  stateBuffer.Load( in, nothing );

  //restore compartment connects by finding the source cell and cmp in the ARRAYS struct
  // then patch pointers
  int origCell = TheBrain->Cells[CellID]->idx;

  if( nForward )
    forwardConn = NI->AR->Cells[origCell]->forwardConn[ CmpID ];
  if( nReverse )
    reverseConn = NI->AR->Cells[origCell]->reverseConn[ CmpID ];

  ActiveList = NULL;
#ifdef SAME_PSC
  nActiveIn = 0;
#endif

  //initialize some remaining values
  TotalStimulusCurrent = TotalChannelCurrent = Current_Leak = TotalSynapseCurrent = TotalAdjacentCompartmentCurrent = 0.0;
}

/*------------------------------------------------------------------------*/
//read in compartment data from a file into a clean structure
//will hopefully not use FILE streams in the future

void Compartment::LoadConnections( deque<Synapse*> &collection, int targetNode )
{
  //go through SendTo list - if an entry should send to the target node,
  //pop an address of the front of the queue
  
  for( int i=0; i<nSendTo; i++ )
  {
    if( SendTo[i].Node == targetNode )
    {
      SendTo[i].Addr = collection.front();
      collection.pop_front();
    }
  }
}

/*---------------------------------------------------------------------------

Note: It looks like there was a bunch of garbage in here, left from
previous incarnations.  Anything not obviously needed has been removed.

The Compartment object is where most of the actual simulation takes
place.  The Cell object is just a container for Compartments.  At every
timestep, the Cell object each Cmp's DoProcessCompartment function to do
processing on that Cmp.  The result is a Cmp state, the most important
component of which is the membrane voltage (MV).

A Cmp may be either Active or not.  Active Cmps can fire, sending an
action potential (spike) to the other cells in its SendTo list.  Cmps
that aren't active don't fire (and thus shouldn't have anything to
SendTo), but are otherwise the same.

The membrane voltage is normally a function of stimulus input, incoming
synapse messages, and channels.

If the Cmp is active, then if the membrane voltage reaches a certain
level, it fires.  When it fires, it becomes refractory.  While in the
refractory state, the membrane voltage is determined strictly by the
spike shape.  (Specified in input.) This lasts for the number of
timesteps specified in the spike.  During this period, all other
contributions to membrane voltage are ignored: however, the channels have
internal state that is a function of MV, so they must be called.  Synapses
may be skipped.  (They get incoming messages in the DoProcessInput
function, and may become active, but the active list is not processed.)

The ActiveList is a linked list of those synapses which are in the
process of contributing their PSG waveforms to the Cmp's MV.  At any
time, only a fraction of the synapses owned by a Cmp may be active.  On
the other hand, a particular synapse may appear in the list more than
once, if it gets a second spike msg before the first PSG has finished.

As an optimization, if the SAME_PSC compile flag is set, all the active
synapses received by the Cmp at a timestep can be added, and treated
as one.  This makes the ActiveList go faster.  Just how much is gained 
of course depends on the rate at which Cmps get multiple spikes...

Cmps & synapses in combination can exhibit learning, in which the synapse
USE value changes in response to firing patterns.

Positive learning happens when this Cmp fires.  It scans through all
synapses, and for those that have positive learning enabled, and which
have received a PSG during the learning window (that is, those whose
input contributed to the Cmp firing), the USE is increased by an amount
specified in the positive learning table.  This is an array, and the
amount is a function of the difference in times between the input and the
firing.

Negative learning is handled mostly by the synapse.  When some synapse
gets a spike message (and thus starts a PSG), the time since the Cmp's
previous spike is checked, and an amount from the negative table added to
the USE if within the window.

---------------------------------------------------------------------------*/

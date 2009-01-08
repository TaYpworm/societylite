/* Synapses are one-way links between pairs of cells.  The Synapse object is */
/* owned by some compartment within the receiving cell.  This compartment    */
/* will receive SPIKE messages from the sending side, which tell it that the */
/* synapse has fired an AP.  This AP is modelled by a waveform template, and */
/* lasts for many timesteps.  The synapse adds the waveform into the Cmp's   */
/* CurrentPSG.                                                               */


#include <stdlib.h>
#include "Synapse.h"
#include "Managers.h"
#include "debug.h"
#include "memstat.h"

#include <iostream>

#define MEM_KEY  KEY_SYNAPSE

Synapse::Synapse ()
{
  MEMADDOBJ (KEY_SYNAPSE);
}

Synapse::Synapse (SynapseDef *SYNDEF, Compartment *CMP, Random *RN)
{
  MEMADDOBJ (KEY_SYNAPSE);

//printf ("Synapse::Synapse:\n);
 
  SynDef = SYNDEF;
  Cmp    = CMP;
  //SynapseID = 0; //I was going to use these for save/load,  but I think calculating
  //an offset from the front of the list using the Cmp pointer should work just as well

  SynReversal = RN->GaussNum (SynDef->SynReversal);
  MaxG = RN->GaussNum(SynDef->MaxG);
  if( MaxG < 0.0 )
    MaxG = 0.0;

/* The USE can get a gaussian distribution, but must always be between 0.0   */
/* and 1.0.  (Per JM)                                                        */

  USE = 0.0;
  if( SynDef->source->USE[0] < 0.0 || SynDef->source->USE[0] > 1.0 )
    printf( "Warning, Synapse: %s defines USE value (%lf) outside range [0.0, 1.0].\n", SynDef->source->L.name, SynDef->source->USE[0] );
  USE = RN->GaussNum (SynDef->source->USE);
  if( USE <= 0.0 )
    USE = 0.0;
  if( USE >= 1.0 )
    USE = 1.0;

  RSE_New = RN->RandRange (SynDef->source->InitRSE);
  if (RSE_New < 0.0) RSE_New = 0.0;
  if (RSE_New > 1.0) RSE_New = 1.0;

  RSE_Old = USE_Old = 0.0;
  USE_New = USE; 

  Tau_Depr = 0.0;  
  if( (SynDef->RSEType == RSE_BOTH || SynDef->RSEType == RSE_DEPR) &&  //will this synapses use DEPR?
      (SynDef->TauDepr[0] <= 0.0 && SynDef->TauDepr[1] <= 0.0) ) //are values valid?
  {
    fprintf( stderr, "Error in Synapse: Invalid Tau Depr value. Must be greater than zero\n" );
    exit(0);
  }
  if( SynDef->RSEType == RSE_BOTH || SynDef->RSEType == RSE_DEPR ) //needs Depr
    while( Tau_Depr <= 0.0 ) //do I need a maximum?
      Tau_Depr  = RN->GaussNum (SynDef->TauDepr);

  Tau_Depr *= NI->FSV;
  
  Tau_Facil = 0.0;
  if( (SynDef->RSEType == RSE_BOTH || SynDef->RSEType == RSE_FACIL) &&  //using FACIL?
      (SynDef->TauFacil[0] <= 0.0 && SynDef->TauFacil[1] <= 0.0) )     //valid values?
  {
    fprintf( stderr, "Error in Synapse: Invalid Tau Facil value. Must be greater than zero\n" );
    exit(0);
  }
  if( SynDef->RSEType == RSE_BOTH || SynDef->RSEType == RSE_FACIL )
    while( Tau_Facil <= 0.0 ) //do I need a maximum?
      Tau_Facil = RN->GaussNum (SynDef->TauFacil);

  Tau_Facil *= NI->FSV;

// new initialization: initDeltaT[0] is the lower bound, initDeltaT[1] 
// is the upper bound - JK

  PreviousSpikeTime = -(int) (NI->FSV * RN->RandRange (SynDef->source->InitDeltaT));
  
  //does this synapse use synaptic augmentation?
  synAugmentation = NULL;
  if( SynDef->source->AugmentationIdx != -1 )
  {
    //create a new synapse augmentation object
    synAugmentation = new Augmentation( NI->AR->SynAugmentation[ SynDef->source->AugmentationIdx ], RN );
  }

  // Initilize counterPosLearning to zero
  counterPosLearning = 0; 

//  printf ("Synapse: Initial values, source = %s\n", SynDef->source->L.name);
//  printf ("   SynReversal = %f\n", SynReversal);
//  printf ("   USE         = %f\n", USE);
//  printf ("   RSE_New     = %f\n", RSE_New);
//  printf ("   Tau_Depr    = %f\n", Tau_Depr);
//  printf ("   Tau_Facil   = %f\n", Tau_Facil);
}

Synapse::~Synapse ()
{
  MEMFREEOBJ (KEY_SYNAPSE);
}

/*---------------------------------------------------------------------------*/
/* This is called by the compartment that owns the synapse, when it receives */
/* a SPIKE message.  It computes a modification factor for the generic spike */
/* template according to various internal values, and returns it.  It also   */
/* updates spike time histories.                                             */

/* The generic waveform may be modified in one of several ways, depending on */
/* the synapse's internal parameters, which are derived from its definition, */
/* and any learning that may have taken place.                               */

/* RSE_NONE: assumes no RSE is being calculated, so the PSC waveform is      */
/* always the same for each AP received.  Each element of the template is    */
/* Multiplied by MaxConductance and USE (both are user-specified).           */

/* RSE_DEPR: assumes depression but no facilitation, so the equation is to   */
/* calculate Rnew, Rold USE_New = 1 + ((RSE_Old * (1 - USE_mean) - 1) * exp  */
/* (-DeltaT/ Tau_Depr)); USE values are user specified and only the Rnew is  */
/* calculated.                                                               */

/* RSE_FACIL: has facilitation but no depression, calculate USE_New, and     */
/* RSE_New == 1                                                              */

/* RSE_BOTH: has both facilitation and depression, both USE and RSE need to  */
/* be calculated.                                                            */


void Synapse::ReceiveMsg (int TimeStep)
{
  double Value;
  int DeltaT;
  
//printf ("Synapse::ReceiveMsg: cmp = %d, RSEType = %s\n", Cmp->CellID, NameDefine  (SynDef->RSEType) );

  DeltaT = TimeStep - PreviousSpikeTime;

  switch (SynDef->RSEType)
  {
    case RSE_NONE:  Value = USE * MaxG;
        break;

    case RSE_DEPR:  USE_Old = USE;
                    RSE_Old = RSE_New;
 
                    RSE_New = 1.0 + ((RSE_Old * (1.0 - USE) - 1.0) * exp ((double) -DeltaT / (double) Tau_Depr));
                    if (RSE_New < 0.0) RSE_New = 0.0;        /* change per JM */
                    if (RSE_New > 1.0) RSE_New = 1.0;
 
                    Value = USE * RSE_New * MaxG;
        break;

    case RSE_FACIL: USE_Old = USE_New;            //this will have USE_mean as initial value
                    USE_New = USE + ((1.0 - USE) * USE_Old * exp ((double) -DeltaT / (double) Tau_Facil));
 
                    Value = USE_New * MaxG;
        break;

    case RSE_BOTH:  USE_Old = USE_New;
                    USE_New = USE + ((1.0 - USE) * USE_Old * exp ((double) -DeltaT / (double) Tau_Facil));

                    RSE_Old = RSE_New;
                    RSE_New = 1.0 + ((RSE_Old * (1.0 - USE_New) - 1.0) * exp ((double) -DeltaT / (double) Tau_Depr));

                    if (RSE_New < 0.0) RSE_New = 0.0;         /* Change per JM */
                    if (RSE_New > 1.0) RSE_New = 1.0;

                    Value = USE_New * RSE_New * MaxG;
                
//printf ("Synapse: USE_New = %f, RSE_New = %f, MaxG = %f, Value = %f\n",
//        USE_New, RSE_New, SynDef->MaxG, Value);
        break;
  }

  //check if this uses synapse augmentation; update calcium, and get SA
  if( synAugmentation )
  {
    double aug = synAugmentation->getAugmentation( TimeStep, DeltaT );
    Value *= aug;//synAugmentation->getAugmentation( TimeStep, DeltaT );
  }
  
  
//printf ("Synapse: Msg at %d, type %s, value = %f\n", TimeStep, SynDef->source->L.name, Value);

/* This does negative learning, which happens only when synapse receives     */
/* incoming spike msg                                                        */

  if (SynDef->LearnNeg)
  {
    DeltaT = TimeStep - Cmp->LastFireTime;
    if (DeltaT < SynDef->nNegLearn)
    {
      USE += SynDef->NegLearnTable [DeltaT];
      if (USE < 0.0) USE = 0.0;
    }
  }
  PreviousSpikeTime = TimeStep;
  //reset counterPosLearning
  counterPosLearning = 0; 
  Cmp->AddSynapseToActive (this, Value);
}

/*---------------------------------------------------------------------------*/
/* This is called whenever the owning cmp fires.  It checks to see if the    */
/* synapse received an incoming spike message within the positive learning   */
/* window.  (In other words, whether this synapse contributed to the firing. */
/* If so, the USE value is modified according to the value in the learning   */
/* table for the interval.  Note that it's only called if the synapse does   */
/* positive learning                                                         */

void Synapse::DoPositiveLearning (int TimeStep)
{
  int dt;

  //check to see if the positive learning flag is on (may be turned off during run)
  if( SynDef->LearnPos != 1 )
    return;

  dt = TimeStep - PreviousSpikeTime;
  if ( (dt < SynDef->nPosLearn) && (counterPosLearning == 0) )
  {
    USE += SynDef->PosLearnTable [dt];
    if (USE > 1.0) USE = 1.0;
    counterPosLearning += 1;
  }
}

/*---------------------------------------------------------------------------*/

int Synapse::getID()
{
    //go through compartment's synapse list looking for a pointer
    //that matches 'this' object.  Although this is slow,
    //it only needs to be done during saving and removes
    //the need for a separte variable in the class definition
    for( int i=0; i<Cmp->nSynapse; i++ )
        if( Cmp->SynapseList[i] == this ) //match
            return i;

    return -1;
}

/*-----------------------------------------------------*/
//Save Load functions

int Synapse::Save( FILE *out )
{
  int i=0;
  int bytes = 0;
  const int iSize = 3;
  int iHolder[iSize];
  const int hSize = 7;
#ifdef USE_FLOAT
  float holder[hSize];
#else
  double holder[hSize];
#endif

  //SynDef - find index of definition (the offset of the pointer from TheBrain->SynapseDefs)
  //PreviosSpikeTime - time needs to be an offset, to account for different TimeStep on load
  //synAugmentation - flag to indicate it is present or not
  i=0;
  iHolder[i++] = SynDef->getID();
  iHolder[i++] = PreviousSpikeTime - TheBrain->TimeStep;
  iHolder[i++] = (synAugmentation==NULL)?0:1;
  fwrite( iHolder, sizeof( int ), i, out );
    bytes += sizeof( int )*i;

  //floats or doubles
  i = 0;
  holder[i++] = SynReversal;
  holder[i++] = MaxG;
  holder[i++] = USE;
  holder[i++] = RSE_New;
  holder[i++] = USE_New;
  holder[i++] = Tau_Depr;
  holder[i++] = Tau_Facil;

#ifdef USE_FLOAT
  fwrite( holder, i, sizeof( float ), out );
  bytes += sizeof( float )*i;
#else
  fwrite( holder, i, sizeof( double ), out );
  bytes += sizeof( double )*i;
#endif

  //synaptic augmentation (if applicable)
  // -need a better way to monitor saved versions
  if( synAugmentation != NULL )
    bytes += synAugmentation->Save( out, TheBrain->TimeStep );

  return bytes;
}

//----------------------------------------------------------------------

void Synapse::Load( FILE *in, char *nothing )
{
  int i;
  const int iSize = 3;
  int iHolder[iSize];
  const int hSize = 7;
#ifdef USE_FLOAT
  float holder[hSize];
#else
  double holder[hSize];
#endif

  //SynDef - find index of definition
  fread( iHolder, iSize, sizeof(int), in );

  i = 0;
  SynDef = TheBrain->SynapseDefs[iHolder[i++]];
  PreviousSpikeTime = iHolder[i++];
  //last iHolder value determines if synAugmentation exists (end of Load func)

  //The Cmp pointer (i.e. pointer to owner Compartment)
    //I could include the parent cmp's address in the parameter list,
    //but I would like all the load functions to be the same for all objects
    //Would passing the address be best [i.e. less prone to errors? ], or
    // keeping Save functions consistent?
  Cmp = NULL;

  //Will hopefully be using the same format as when saved (i.e. floats vs doubles)
#ifdef USE_FLOAT
  fread( holder, hSize, sizeof( float ), in );
#else
  fread( holder, hSize, sizeof( double ), in );
#endif
  i = 0;
  SynReversal = holder[i++];
  MaxG = holder[i++];
  USE = holder[i++];
  RSE_New = holder[i++];
  	//RSE_Old = holder[i++]; //-not needed in save file
  USE_New = holder[i++];
  	//USE_Old = holder[i++]; //-
  Tau_Depr = holder[i++];
  Tau_Facil = holder[i++];

  if( iHolder[2] )
  {
    synAugmentation = new Augmentation();
    synAugmentation->Load( in, nothing );
  }
  else
    synAugmentation = NULL;
}


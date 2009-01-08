#include <stdio.h>
#include "SynapseDef.h"
#include "memstat.h"
#include "Managers.h"

#define MEM_KEY KEY_SYNDEF

SynapseDef::SynapseDef ()
{
  MEMADDOBJ (KEY_SYNLEARN);
}

SynapseDef::SynapseDef (T_SYNAPSE *iSyn, ARRAYS *AR)
{
  T_SYN_FD *iSFD;
  T_SYNPSG *iPSG;
  T_SYNLEARN *iSLN;

  MEMADDOBJ (KEY_SYNLEARN);

  source = iSyn;

  SynDelay    = iSyn->SynDelay;
  SynReversal = iSyn->SynRever;
#ifdef USE_FLOAT
  MaxG[0]        = iSyn->MaxG[0];
  MaxG[1]        = iSyn->MaxG[1];
#else
  MaxG        = iSyn->MaxG;
#endif

/* Stuff that was in Syn_FD/SynapseRSE */

  if (AR->SynFD != NULL)
  {
    iSFD = AR->SynFD [iSyn->SfdIdx];
    if (iSFD != NULL)
    {
      RSEType  = iSFD->SFD;
      TauDepr  = iSFD->Depr_Tau;
      TauFacil = iSFD->Facil_Tau;
//    printf ("SynapseDef: TauDepr = %lf\n", TauDepr [0]);
    }
  }


/* Stuff that was in Syn_PSG   */

  if (AR->SynPSG != NULL)
  {
    iPSG = AR->SynPSG [iSyn->PsgIdx];
    if (iPSG != NULL)
    {
      PSGLen      = iPSG->nPSG;
      PSGTemplate = iPSG->PSG;
    }
  }

/* Stuff that was in Syn_Learn  */

  if (AR->SynLearn != NULL)
  {
    iSLN = AR->SynLearn [iSyn->LearnIdx];
    if (iSLN != NULL)
    {
      LearnPos      = (iSLN->Learning & LEARN_POS) ? true : false;
      nPosLearn     = iSLN->nPosLearn;
      PosLearnTable = iSLN->PosLearnTable;

      LearnNeg      = (iSLN->Learning & LEARN_NEG) ? true : false;
      nNegLearn     = iSLN->nNegLearn;
      NegLearnTable = iSLN->NegLearnTable;
    }
  }
  
//printf ("SynapseDef: MaxG = %f\n", MaxG[0]);
}

SynapseDef::~SynapseDef ()
{
  MEMFREEOBJ (KEY_SYNDEF);

}

int SynapseDef::SetLearning( int code )
{
  if( code == 0 )
    LearnNeg = LearnPos = 0;
  else if( code == 1 )
    LearnPos = !(LearnNeg = 0);
  else if( code == 2 )
    LearnPos = !(LearnNeg = 1);
  else if( code == 3 )
    LearnPos = LearnNeg = 1;
  else
    return -1;

  return 0;
}

/*-------------------------------------------------*/
//Save Load operations

int SynapseDef::Save( FILE *out )
{
  //how much of this data ~should~ be stored with the original ARRAYS structure?
  //for now, write out all data.  If some can be removed later (due to that information
  //being contained elsewhere e.g. ARRAYS) then we can conserve some space
  
  const int hSize = 8;
  const int bSize = 2;
  const int iSize = 5;
  int i;
  int bytes = 0;

  double holder[hSize];
  bool  bholder[bSize];
  int   iholder[iSize];

  i = 0;
  holder[i++] = TauDepr[0];
  holder[i++] = TauDepr[1];
  holder[i++] = TauFacil[0];
  holder[i++] = TauFacil[1];
  holder[i++] = SynDelay[0];
  holder[i++] = SynDelay[1];
  holder[i++] = SynReversal[0];
  holder[i++] = SynReversal[1];
  fwrite( holder, i, sizeof( double ), out );
  bytes += i*sizeof( double );
 
  i=0;
  iholder[i++] = source->L.idx;  //this will be needed in case of synapse reports
  iholder[i++] = RSEType;
  iholder[i++] = PSGLen;         //will this be replaced with an index numbers?
  iholder[i++] = nPosLearn;      //-
  iholder[i++] = nNegLearn;      //-
  fwrite( iholder, i, sizeof( int ), out );
  bytes += i*sizeof( int );

  i=0;
  bholder[i++] = LearnPos;
  bholder[i++] = LearnNeg;
  fwrite( bholder, i, sizeof( bool ), out );
  bytes += sizeof( bool );

  //since MaxG can be float or double, use its name in sizeof op
    //the user is required to use the same ncs version to save/load
  fwrite( MaxG, 2, sizeof( MaxG[0] ), out );
  bytes += 2*sizeof( MaxG[0] );

  //dynamic arrays: PSG waveform and learning tables
  if( PSGLen > 0 )
    fwrite( PSGTemplate, PSGLen, sizeof( double ), out );
  if( nPosLearn > 0 )
    fwrite( PosLearnTable, nPosLearn, sizeof( double ), out );
  if( nNegLearn > 0 )
    fwrite( NegLearnTable, nNegLearn, sizeof( double ), out );
  bytes += (nPosLearn + nNegLearn + PSGLen)*sizeof( double );

  return bytes;
}

/*-------------------------------------------------*/

void SynapseDef::Load( FILE *in, char *nothing )
{
  const int hSize = 8;
  const int bSize = 2;
  const int iSize = 5;
  int i;

  double holder[hSize];
  bool  bholder[bSize];
  int   iholder[iSize];

  TauDepr = (double *) calloc ( 2, sizeof(double) );
  TauFacil = (double *) calloc ( 2, sizeof(double) );
  SynDelay = (double *) calloc ( 2, sizeof(double) );
  SynReversal = (double *) calloc ( 2, sizeof(double) );

  i = 0;
  fread( holder, hSize, sizeof( double ), in );
  TauDepr[0] = holder[i++];
  TauDepr[1] = holder[i++];
  TauFacil[0] = holder[i++];
  TauFacil[1] = holder[i++];
  SynDelay[0] = holder[i++];
  SynDelay[1] = holder[i++];
  SynReversal[0] = holder[i++];
  SynReversal[1] = holder[i++];

  i=0;
  fread( iholder, iSize, sizeof( int ), in );

  source = NI->AR->Synapse[iholder[i++]];
  RSEType = iholder[i++];
  PSGLen = iholder[i++];
  nPosLearn = iholder[i++];
  nNegLearn = iholder[i++];

  i=0;
  fread( bholder, bSize, sizeof( bool ), in );
  LearnPos = bholder[i++];
  LearnNeg = bholder[i++];

  //since MaxG can be float or double, use its name in sizeof op
    //the user is required to use the same ncs version to save/load
  fread( MaxG, 2, sizeof( MaxG[0] ), in );

  //dynamic arrays: PSG waveform and learning tables
  if( PSGLen > 0 )
  {
    PSGTemplate = (double *) calloc ( PSGLen, sizeof(double) );
    fread( PSGTemplate, PSGLen, sizeof( double ), in );
  }
  if( nPosLearn > 0 )
  {
    PosLearnTable = (double *) calloc ( nPosLearn, sizeof(double) );
    fread( PosLearnTable, nPosLearn, sizeof( double ), in );
  }
  if( nNegLearn > 0 )
  {
    NegLearnTable = (double *) calloc ( nNegLearn, sizeof(double) );
    fread( NegLearnTable, nNegLearn, sizeof( double ), in );
  }
}

/*-------------------------------------------------*/

int SynapseDef::getID()
{
  for( int i=0; i<TheBrain->nSynapseDef; i++ )
  {
    if( TheBrain->SynapseDefs[i] == this ) //same address?
      return i;
  }
  return -1;
}


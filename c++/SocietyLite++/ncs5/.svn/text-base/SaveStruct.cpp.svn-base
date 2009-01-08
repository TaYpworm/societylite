/*--------------------------------------------------------------------------
//  This file provides functions that will write out the INPUT structure
//  to a file for future use
//
//  This will allow a loaded brain to use the same names to access the 
//  structures such as columns, layers, cells, etc.
//
//  Taken from ncs ver. 3 updated to work with ncs ver. 5
//  Need to write out the T_structs now that the parser uses only them.
--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "InitStruct.h" //also include in .h file
#include "SaveStruct.h"

#include "Managers.h"

int saveString( char *string, FILE *out )
{
  int size = 0;
  
  if ( string != NULL )
  {
    size = strlen( string ) + 1;
    fwrite( &size, sizeof( int ), 1, out );
    fwrite( string, sizeof( char ), size, out );
  }
  else  //else just write out 0 for size
    fwrite( &size, sizeof( int ), 1, out );

  return sizeof( size ) + size*sizeof( char );
}

//-----------------------------------------------------------------------------

int saveLocator( LOCATOR *L, FILE *out )
{
  int bytes = 0;

  //i'm not sure how much is necessary.  It is probably better to write too much
  //rather than too little.
  fwrite( &L->kind, sizeof( int ), 1, out );
  fwrite( &L->idx, sizeof( int ), 1, out );
  bytes += sizeof( int ) * 2;

  bytes += saveString( L->name, out );

  //next pointer does not need to be written
    //--do I need to follow the next pointer!?
  //I won't write filename or line out for now - maybe I will at a future time
  return bytes;
}

//-----------------------------------------------------------------------------

int saveList( LIST *List, FILE *out )
{
  int i, nbytes = 0;
  LIST *Item = List;
  const int dSize = 3;
  double dHolder[ dSize ];

  //need to follow the locator object
  while( Item != NULL )
  {
    nbytes += saveLocator( &Item->L, out ); //writes out name, kind, idx

    fwrite( &Item->Qty, sizeof( int ), 1, out );
      nbytes += sizeof( int );
    nbytes += saveString( Item->label, out );
    
    i = 0;
    dHolder[ i++ ] = Item->x;  //shouldn't these be in an array to begin with?
    dHolder[ i++ ] = Item->y;
    dHolder[ i++ ] = Item->z;
    fwrite( dHolder, sizeof( double ), dSize, out );
      nbytes += sizeof( double ) * dSize;
    Item = (LIST *) Item->L.next; //move to the next item
  }

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveConnect( T_CONNECT *iConnect, FILE *out )
{
  int nbytes = 0;
  const int iSize = 9;
  int iHolder[ iSize ];
  int i;

  nbytes += saveLocator( &iConnect->L, out );

  //user defined strings
  nbytes += saveString( iConnect->fromColName, out );
  nbytes += saveString( iConnect->fromLayName, out );
  nbytes += saveString( iConnect->fromCellName, out );
  nbytes += saveString( iConnect->fromCmpName, out );
  nbytes += saveString( iConnect->toColName, out );
  nbytes += saveString( iConnect->toLayName, out );
  nbytes += saveString( iConnect->toCellName, out );
  nbytes += saveString( iConnect->toCmpName, out );
  nbytes += saveString( iConnect->SynName, out );

  //indices of corresponding objects
  i = 0;
  iHolder[ i++ ] = iConnect->FromCol;
  iHolder[ i++ ] = iConnect->FromLay;
  iHolder[ i++ ] = iConnect->FromCell;
  iHolder[ i++ ] = iConnect->FromCmp;
  iHolder[ i++ ] = iConnect->ToCol;
  iHolder[ i++ ] = iConnect->ToLay;
  iHolder[ i++ ] = iConnect->ToCell;
  iHolder[ i++ ] = iConnect->ToCmp;
  iHolder[ i++ ] = iConnect->SynType;
  //iHolder[ i++ ] = iConnect->speed;
  fwrite( iHolder, sizeof( int ), iSize, out );
  nbytes += iSize * sizeof( int );

//speed needs to be moved to a double
  fwrite( &iConnect->speed, 1, sizeof( double ), out );

  fwrite( &iConnect->Prob, 1, sizeof( double ), out );
  nbytes += 2*sizeof( double );

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveCmpConnect( T_CMPCONNECT *iConnect, FILE *out )
{
  int i, nbytes = 0;
  const int iSize = 2;  //same size for both arrays
  const int dSize = 4;
  int iHolder[iSize];
  double dHolder[dSize];

  nbytes += saveLocator( &iConnect->L, out );

  nbytes += saveString( iConnect->fromCmpName, out );
  nbytes += saveString( iConnect->toCmpName, out );

  i=0;
  iHolder[i++] = iConnect->FromCmp;
  iHolder[i++] = iConnect->ToCmp;
  //iHolder[i++] = iConnect->Speed;
  fwrite( iHolder, sizeof( int ), i, out );
    nbytes += sizeof( int ) * i;

  i=0;
  dHolder[i++] = iConnect->G;
  dHolder[i++] = iConnect->retroG;
  dHolder[i++] = iConnect->delay;
  dHolder[i++] = iConnect->Speed;
  fwrite( dHolder, sizeof( double ), i, out );
    nbytes += sizeof( double ) * i;
  
  return nbytes;
}

//-----------------------------------------------------------------------------

int saveTBrain( T_BRAIN *iBrain, FILE *out )
{
  int i, nbytes = 0;
  const int iSize = 5;
  int iHolder[iSize];

  //Locator
  nbytes += saveLocator( &iBrain->L, out );

  //write out all integers
  i=0;
  iHolder[i++] = iBrain->ConnectRpt;
  iHolder[i++] = iBrain->SpikeRpt;
  iHolder[i++] = iBrain->FSV;
  iHolder[i++] = iBrain->nColumns;
//iHolder[i++] = iBrain->nStInject;
//iHolder[i++] = iBrain->nReports;
  iHolder[i++] = iBrain->nConnect;
  fwrite( iHolder, sizeof( int ), i, out );
    nbytes += sizeof( int ) * i;

//write out the olny long - it's the SEED value: is there any point to saving it?
  fwrite( &iBrain->Seed, sizeof( long ), 1, out );
    nbytes += sizeof( long );

//write out only double - it's the DURATION value: won't this be replaced next run?
  fwrite( &iBrain->Duration, sizeof( double ), 1, out );
    nbytes += sizeof( double );

//  nbytes += saveLocator( &iBrain->L, out );
  nbytes += saveString( iBrain->job, out );
  nbytes += saveString( iBrain->distribute, out );

  //write out column info - the list writes out excess info & wastes space
  nbytes += saveList( iBrain->ColumnNames, out );
  fwrite( iBrain->Columns, sizeof( int ), iBrain->nColumns, out );
    nbytes += sizeof( int ) * iBrain->nColumns;

  //write out connection info
  for( i=0; i<iBrain->nConnect; i++ )
    nbytes += saveConnect( iBrain->Connect[i], out );

  //don't save...
    //save info: savefile, savetime, loadfile
    //port info: Port, Hostname
    //reports and stims: nReports, nStInject, LIST *StInjNames, StInject, ReportNames, Reports

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveTCShell( T_CSHELL *iShell, FILE *out )
{
  int i, nbytes = 0;
  const int dSize = 4;
  double dHolder[dSize];

  nbytes += saveLocator( &iShell->L, out );

  i = 0;
  dHolder[ i++ ] = iShell->width;
  dHolder[ i++ ] = iShell->height;
  dHolder[ i++ ] = iShell->x;
  dHolder[ i++ ] = iShell->y;
  fwrite( dHolder, sizeof( double ), i, out );
    nbytes += sizeof( double ) * i;

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveTColumn( T_COLUMN *iColumn, FILE *out )
{
  int i, nbytes = 0;
  const int iSize = 3;
  int iHolder[ iSize ];

  nbytes += saveLocator( &iColumn->L, out );
  nbytes += saveString( iColumn->shellName, out );

  i=0;
  iHolder[i++] = iColumn->CShell;
  iHolder[i++] = iColumn->nLayers;
  iHolder[i++] = iColumn->nConnect;
  fwrite( iHolder, sizeof( int ), i, out );
    nbytes += sizeof( int ) * i;

  //write out layer info
  nbytes += saveList( iColumn->LayerNames, out );
  fwrite( iColumn->Layers, sizeof( int ), iColumn->nLayers, out );
    nbytes += sizeof( int ) * iColumn->nLayers;

  //write out connection info
  for( i=0; i<iColumn->nConnect; i++ )
    nbytes += saveConnect( iColumn->Connect[i], out );

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveTLShell( T_LSHELL *iShell, FILE *out )
{
  int nbytes = 0;

  nbytes += saveLocator( &iShell->L, out );

  //only 2 doubles - just write each out
  fwrite( &iShell->Lower, sizeof( double ), 1, out );
  fwrite( &iShell->Upper, sizeof( double ), 1, out );
  nbytes += sizeof( double ) * 2;

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveTLayer( T_LAYER *iLayer, FILE *out )
{
  int i, nbytes = 0;
  const int iSize = 3;
  int iHolder[iSize];

  nbytes += saveLocator( &iLayer->L, out );

  nbytes += saveString( iLayer->shellName, out );

  i = 0;
  iHolder[i++] = iLayer->LShell;
  iHolder[i++] = iLayer->nCellTypes;
  iHolder[i++] = iLayer->nConnect;
  fwrite( iHolder, sizeof( int ), i, out );
    nbytes += sizeof( int ) * 3;

  //Cell info
  nbytes += saveList( iLayer->CellNames, out );
  fwrite( iLayer->CellTypes, sizeof( int ), iLayer->nCellTypes, out );
  fwrite( iLayer->CellQty, sizeof( int ), iLayer->nCellTypes, out );
    nbytes += sizeof( int ) * 2 * iLayer->nCellTypes;

  //Connection info
  for( i=0; i<iLayer->nConnect; i++ )
    nbytes += saveConnect( iLayer->Connect[i], out );

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveTCell( T_CELL *iCell, FILE *out )
{
  int i, nbytes = 0;

  nbytes += saveLocator( &iCell->L, out );

  //check to see if nConnect has been allocated. If not, set it to zero
  if( !iCell->Connect )
    iCell->nConnect = 0;

  //saving 2 ints
  fwrite( &iCell->nCmp, sizeof( int ), 1, out );
  fwrite( &iCell->nConnect, sizeof( int ), 1, out );
    nbytes += sizeof( int ) * 2;

  nbytes += saveList( iCell->CmpNames, out );
  
  //indices where cmp data can be found in AR
  fwrite( iCell->Cmp, sizeof( int ), iCell->nCmp, out ); 
    nbytes += sizeof(int) * iCell->nCmp;

  //individual compartment info
  fwrite( iCell->Xpos, sizeof( double ), iCell->nCmp, out );
  fwrite( iCell->Ypos, sizeof( double ), iCell->nCmp, out );
  fwrite( iCell->Zpos, sizeof( double ), iCell->nCmp, out );
    nbytes += sizeof( double ) * 3 * iCell->nCmp;

  //don't save Rpos - it is only included for compatibility
  //fwrite( &iCell->Rpos, sizeof( double ), iCell->nCmp, out );

  for( i=0; i<iCell->nConnect; i++ )
  {
    if( iCell->Connect ) //if it's not allocated, we've got bigger problems
      nbytes += saveCmpConnect( iCell->Connect[i], out );
    else
      fprintf( stderr, "Warning, cell %s has a NULL Compartment Connection Pointer\n" );
  }
  //Labels are pointers to the labels in the CmpNames list - don't save

  //forward/reverse connections
    // -> this is just data that can be recomputed - don't save

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveTCmp( T_CMP *iCmp, FILE *out )
{
  int i, nbytes = 0;
  const int iSize = 3;
  const int dSize = 22;
  int iHolder[iSize];
  double dHolder[dSize];

  nbytes += saveLocator( &iCmp->L, out );

  //save SEED?
  fwrite( &iCmp->Seed, sizeof( long ), 1, out );
    nbytes += sizeof( long );
 
  i=0;
  iHolder[i++] = iCmp->Active;
  iHolder[i++] = iCmp->Spike;
  iHolder[i++] = iCmp->nChannels;
  fwrite( iHolder, sizeof( int ), i, out );
    nbytes += sizeof( int ) * i;

  i=0;
  dHolder[i++] = iCmp->Spike_HW[0];
  dHolder[i++] = iCmp->Spike_HW[1];
  dHolder[i++] = iCmp->Tau_Membrane[0];
  dHolder[i++] = iCmp->Tau_Membrane[1];
  dHolder[i++] = iCmp->R_Membrane[0];
  dHolder[i++] = iCmp->R_Membrane[1];
  dHolder[i++] = iCmp->Threshold [0];
  dHolder[i++] = iCmp->Threshold [1];
  dHolder[i++] = iCmp->Leak_Reversal[0];
  dHolder[i++] = iCmp->Leak_Reversal[1];
  dHolder[i++] = iCmp->VMREST[0];
  dHolder[i++] = iCmp->VMREST[1];
  dHolder[i++] = iCmp->Leak_G [0];
  dHolder[i++] = iCmp->Leak_G [1];
  dHolder[i++] = iCmp->CaInt[0];
  dHolder[i++] = iCmp->CaInt[1];
  dHolder[i++] = iCmp->CaExt[0];
  dHolder[i++] = iCmp->CaExt[1];
  dHolder[i++] = iCmp->CaSpikeInc[0];
  dHolder[i++] = iCmp->CaSpikeInc[1];
  dHolder[i++] = iCmp->CaTau[0];
  dHolder[i++] = iCmp->CaTau[1];
  fwrite( dHolder, sizeof( double ), i, out );
    nbytes += sizeof( double ) * i;

  //spike info
  nbytes += saveString( iCmp->SpikeName, out );

  //channel info
  nbytes += saveList( iCmp->ChannelNames, out );
  fwrite( iCmp->Channels, sizeof( int ), iCmp->nChannels, out );
    nbytes += sizeof( int ) * iCmp->nChannels;

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveTChannel( T_CHANNEL *iChannel, FILE *out )
{
  int i, nbytes = 0;
  const int iSize = 7;
  const int dSize = 56;
  int iHolder[ iSize ];
  double dHolder[ dSize ];

  nbytes += saveLocator( &iChannel->L, out );
  
  //do I need to save SEED
  fwrite( &iChannel->Seed, sizeof( long ), 1, out );
    nbytes += sizeof( long );

  i=0;
  iHolder[ i++ ] = iChannel->family;
  iHolder[ i++ ] = iChannel->nTauM;
  iHolder[ i++ ] = iChannel->nValM;
  iHolder[ i++ ] = iChannel->nVoltM;
  iHolder[ i++ ] = iChannel->nTauH;
  iHolder[ i++ ] = iChannel->nValH;
  iHolder[ i++ ] = iChannel->nVoltH;
  fwrite( iHolder, sizeof( int ), i, out );
    nbytes += sizeof( int ) * i;

  i=0;
  //try a different technique - use memcpy
  memcpy( &dHolder[i], &iChannel->unitaryG[0], sizeof( double ) * 22 );
    i+=22;
  dHolder [i++] = iChannel->ValM_stdev;
  dHolder [i++] = iChannel->VoltM_stdev;
  dHolder [i++] = iChannel->ValH_stdev;
  dHolder [i++] = iChannel->VoltH_stdev;
  memcpy( &dHolder[i], &iChannel->alphaScaleFactorM[0], sizeof( double ) * 30 );
    i+=30;
  fwrite( dHolder, sizeof( double ), i, out );
    nbytes += sizeof( double ) * i;

  //dynamic arrays
  fwrite( iChannel->TauValM, sizeof( double ), iChannel->nValM, out );
  fwrite( iChannel->TauVoltM, sizeof( double ), iChannel->nVoltM, out );
  fwrite( iChannel->TauValH, sizeof( double ), iChannel->nValH, out );
  fwrite( iChannel->TauVoltH, sizeof( double ), iChannel->nVoltH, out );
    nbytes += sizeof( double ) * ( iChannel->nValM + iChannel->nVoltM + iChannel->nValH + iChannel->nVoltH );

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveTSynapse( T_SYNAPSE *iSynapse, FILE *out )
{
  int i, nbytes = 0;
  const int iSize = 4;
  const int dSize = 11;
  int iHolder[iSize];
  double dHolder[dSize];

  nbytes += saveLocator( &iSynapse->L, out );

  fwrite( &iSynapse->Seed, sizeof( long ), 1, out );
    nbytes += sizeof( long );

  nbytes += saveString( iSynapse->SfdName, out );
  nbytes += saveString( iSynapse->LearnName, out );
  nbytes += saveString( iSynapse->DataName, out );
  nbytes += saveString( iSynapse->PsgName, out );

  //try the same technique as channel
  i=0;
  memcpy( &iHolder[i], &iSynapse->SfdIdx, sizeof( int ) * 4 );
  i+= 4;
  fwrite( iHolder, sizeof( int ), i, out );
    nbytes += sizeof( int ) * i;

  i=0;
  memcpy( &dHolder[i], &iSynapse->USE[0], sizeof( double ) * 11 );
  i+=11;
  fwrite( dHolder, sizeof( double ), i, out );
    nbytes += sizeof( double ) * i;

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveTSynLearn( T_SYNLEARN *iLearn, FILE *out )
{
  int i, nbytes = 0;
  const int dSize = 12;
  double dHolder[dSize];

  nbytes += saveLocator( &iLearn->L, out );

  fwrite( &iLearn->Seed, sizeof( long ), 1, out );
    nbytes += sizeof( long );

  //only two ints
  fwrite( &iLearn->nPosLearn, sizeof( int ), 1, out );
  fwrite( &iLearn->nNegLearn, sizeof( int ), 1, out );
    nbytes += sizeof( int ) * 2;
  
  i=0;
  memcpy( &dHolder[i], &iLearn->Neg_Heb_Peak_Delta_Use[0], sizeof( double ) * 12 );
  i += 12;
  fwrite( dHolder, sizeof( double ), i, out );
    nbytes += sizeof( double ) * i;

  //dynamic arrays - need to go back and test previous objects for nItems > 0
  fwrite( iLearn->PosLearnTable, sizeof( double ), iLearn->nPosLearn, out );
  fwrite( iLearn->NegLearnTable, sizeof( double ), iLearn->nNegLearn, out );
    nbytes += sizeof( double ) * ( iLearn->nPosLearn + iLearn->nNegLearn );

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveTSynFD( T_SYN_FD *iFD, FILE *out )
{
  int i, nbytes = 0;
  const int dSize = 4;
  double dHolder[dSize];

  nbytes += saveLocator( &iFD->L, out );

  fwrite( &iFD->Seed, sizeof( long ), 1, out );
    nbytes += sizeof ( long );

  fwrite( &iFD->SFD, sizeof( int ), 1, out );
    nbytes += sizeof( int );

  i=0;
  memcpy( &dHolder[0], &iFD->Facil_Tau[0], sizeof( double ) * 4 );
  i += 4;
  fwrite( dHolder, sizeof( double ), i, out );
    nbytes += sizeof( double ) * 4;

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveTSynPSG( T_SYNPSG *iPsg, FILE *out )
{
  int nbytes = 0;

  nbytes += saveLocator( &iPsg->L, out );

  fwrite( &iPsg->nPSG, sizeof( int ), 1, out );
    nbytes += sizeof( int );

  fwrite( iPsg->PSG, sizeof( double ), iPsg->nPSG, out );
    nbytes += sizeof( double ) * iPsg->nPSG;

  //don't save file name where the data was obtained from

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveTSynData( T_SYNDATA *iData, FILE *out )
{
  int i, nbytes = 0;
  const int dSize = 6;
  double dHolder[dSize];

  nbytes += saveLocator( &iData->L, out );

  fwrite( &iData->Seed, sizeof( long ), 1, out );
    nbytes += sizeof( long );

  i=0;
  memcpy( &dHolder[0], &iData->MaxG, sizeof( double ) * dSize );
  i += dSize;
  fwrite( dHolder, sizeof( double ), i, out );
    nbytes += sizeof( double );

  return nbytes;
}

//-----------------------------------------------------------------------------

int saveTSpike( T_SPIKE *iSpike, FILE *out )
{
  int nbytes = 0;

  nbytes += saveLocator( &iSpike->L, out );

  fwrite( &iSpike->nVoltages, sizeof( int ), 1, out );
    nbytes += sizeof( int );
  fwrite( iSpike->Voltages, sizeof( double ), iSpike->nVoltages, out );
    nbytes += sizeof( double ) * iSpike->nVoltages;

  return nbytes;
}

//not going to save stimulus, stim_inject, or reports unless need to

//-----------------------------------------------------------------------------

int saveArrays( ARRAYS *AR, FILE *out )
{
  const int iSize = 18;
  int iHolder[iSize];
  int i, nbytes = 0;

  i=0;
  iHolder[i++] = AR->LayerCount;
  iHolder[i++] = AR->CellCount;
  iHolder[i++] = AR->ClusterCount;
  iHolder[i++] = AR->CmpCount;
  iHolder[i++] = AR->ChanCount;
  iHolder[i++] = AR->nCsh;
  iHolder[i++] = AR->nColumns;
  iHolder[i++] = AR->nLsh;
  iHolder[i++] = AR->nLayers;
  iHolder[i++] = AR->nCells;
  iHolder[i++] = AR->nCmp;
  iHolder[i++] = AR->nChannel;
  iHolder[i++] = AR->nSynapse;
  iHolder[i++] = AR->nSynLearn;
  iHolder[i++] = AR->nSynFD;
  iHolder[i++] = AR->nSynPSG; 
  iHolder[i++] = AR->nSynData;
  iHolder[i++] = AR->nSpike;
//  iHolder[i++] = AR->nStimulus;
//  iHolder[i++] = AR->nStInject;
//  iHolder[i++] = AR->nReports;
  fwrite( iHolder, sizeof( int ), i, out );
    nbytes += sizeof( int ) * i;

  //there can only be one brain - so no loop required
  nbytes += saveTBrain( AR->Brain, out );
  for( i=0; i<AR->nCsh; i++ )
    nbytes += saveTCShell( AR->Csh[i], out );
  for( i=0; i<AR->nColumns; i++ )
    nbytes += saveTColumn( AR->Columns[i], out );
  for( i=0; i<AR->nLsh; i++ )
    nbytes += saveTLShell( AR->Lsh[i], out );
  for( i=0; i<AR->nLayers; i++ )
    nbytes += saveTLayer( AR->Layers[i], out );
  for( i=0; i<AR->nCells; i++ )
    nbytes += saveTCell( AR->Cells[i], out );
  for( i=0; i<AR->nCmp; i++ )
    nbytes += saveTCmp( AR->Cmp[i], out );
  for( i=0; i<AR->nChannel; i++ )
    nbytes += saveTChannel( AR->Channel[i], out );
  for( i=0; i<AR->nSynapse; i++ )
    nbytes += saveTSynapse( AR->Synapse[i], out );
  for( i=0; i<AR->nSynLearn; i++ )
    nbytes += saveTSynLearn( AR->SynLearn[i], out );
  for( i=0; i<AR->nSynFD; i++ )
    nbytes += saveTSynFD( AR->SynFD[i], out );
  for( i=0; i<AR->nSynPSG; i++ )
    nbytes += saveTSynPSG( AR->SynPSG[i], out );
  for( i=0; i<AR->nSynData; i++ )
    nbytes += saveTSynData( AR->SynData[i], out );
  for( i=0; i<AR->nSpike; i++ )
    nbytes += saveTSpike( AR->Spike[i], out );
  return nbytes;
}


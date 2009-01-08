/*--------------------------------------------------------------------------
//  This file provides functions that will read in the INPUT structure
//  to a file for future use
//
//  This will allow a loaded brain to use the same names to access the
//  structures such as columns, layers, cells, etc.
--------------------------------------------------------------------------*/

#include <stdio.h>
#include "InitStruct.h"
#include "LoadStruct.h"
#include <math.h>
#include "defines.h"
#include "Managers.h"
#include "parse/share.c"
#include <set>

using namespace std;

struct ObjectName
{
  char *name;
  int index;

  ObjectName() { name = NULL; index = -1; }
  ~ObjectName() {}

  ObjectName operator=( const ObjectName &RHS )
  {
    if( RHS.name == NULL )
      fprintf( stderr, "ObjectName::operator= given null string\n" );
    name = RHS.name;
    index = RHS.index;
  }

  int operator<( const ObjectName &RHS ) const
  {
    if( RHS.name == NULL )
      fprintf( stderr, "ObjectName::operator< given null string\n" );
    if( strcmp( name, RHS.name ) < 0 )
      return 1;
    else
      return 0;
  }

  int operator==( const ObjectName &RHS ) const
  {
    if( RHS.name == NULL )
      fprintf( stderr, "ObjectName::operator== given null string\n" );
    if( strcmp( name, RHS.name ) == 0 )
      return 1;
    else
      return 0;
  }
};

set<ObjectName, std::less<ObjectName> > oNames;

int findObject( char* name );
int findCompartment( char* name );

/**
 * Read a dynamically allocated string from a file. reads in
 * a 4 byte integer (the string length) followed by the characters
 * comprising the string.  
 * @param string character string passed by reference
 * @param in FILE pointer to file
 * @return total number of bytes read
 */
int loadString( char* &string, FILE *in )
{
  int nbytes = 0;
  string = NULL;

  fread( &nbytes, sizeof( int ), 1, in );

  if( nbytes > 0 )
  {
    string = (char *) calloc ( nbytes, sizeof( char ) );
    fread( string, sizeof( char ), nbytes, in );
  }

  return nbytes + sizeof( int );
}

/**
 * Alternate function for reading strings.  The same file format is used
 * (int containing number of characters, then the characters), but the 
 * actual pointer is not sent.  Rather, the allocated string is returned
 * so that it can be assigned to a pointer instead.
 * @param in FILE pointer to file
 * @return the allocated string.
 */
char *loadString( FILE *in )
{
  char *string = NULL;
  int nbytes = 0;

  fread( &nbytes, sizeof( int ), 1, in );
  if( nbytes > 0 )
  {
    string = (char *) calloc ( nbytes, sizeof( char ) );
    fread( string, sizeof( char ), nbytes, in );
  }

  return string;

}

//-----------------------------------------------------------------------------

int loadLocator( LOCATOR &L, FILE *in )
{
  int bytes = 0;
  ObjectName activeObject;

  fread( &L.kind, sizeof( int ), 1, in );
  fread( &L.idx, sizeof( int ), 1, in );
  bytes += sizeof( int ) * 2;

  bytes += loadString( L.name, in );

  //insert into object tree
  if( L.name )
  {
    activeObject.name = L.name;
    activeObject.index = L.idx;

    oNames.insert( activeObject );
  }

  //next pointer does not need to be written
    //--do I need to follow the next pointer!?
  //I won't write filename or line in for now - maybe I will at a future time
  return bytes; 
}

//-----------------------------------------------------------------------------

int loadList( int count, LIST* &List, FILE *in )
{
  int i, iteration, nbytes = 0;
  LIST *Item = NULL, *prev;//List;
  const int dSize = 3;
  double dHolder[ dSize ];

  //need to follow the locator object
  for( iteration=0; iteration<count; iteration++ )//while( Item != NULL )
  {
    Item = (LIST *) calloc ( 1, sizeof( LIST ) );
    nbytes += loadLocator( Item->L, in ); //get name, kind, idx
    fread( &Item->Qty, sizeof( int ), 1, in );
      nbytes += sizeof( int );
    nbytes += loadString( Item->label, in );

    i = 0;
    fread( dHolder, sizeof( double ), dSize, in );
	Item->x = dHolder[ i++ ];  //shouldn't these be in an array to begin with?
	Item->y = dHolder[ i++ ];
	Item->z = dHolder[ i++ ];
      nbytes += sizeof( double ) * dSize;
    
    //Item = (LIST *) Item->L.next; //move to the next item
    if( iteration == 0 )
      List = Item;
    else
      prev->L.next = Item;

    prev = Item;
  }

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadConnect( T_CONNECT* &iConnect, FILE *in )
{
  int nbytes = 0;
  const int iSize = 9;
  int iHolder[ iSize ];
  int i;

  iConnect = (T_CONNECT *) calloc ( sizeof(T_CONNECT), 1 );

  nbytes += loadLocator( iConnect->L, in );

  //user defined strings
  nbytes += loadString( iConnect->fromColName, in );
  nbytes += loadString( iConnect->fromLayName, in );
  nbytes += loadString( iConnect->fromCellName, in );
  nbytes += loadString( iConnect->fromCmpName, in );
  nbytes += loadString( iConnect->toColName, in );
  nbytes += loadString( iConnect->toLayName, in );
  nbytes += loadString( iConnect->toCellName, in );
  nbytes += loadString( iConnect->toCmpName, in );
  nbytes += loadString( iConnect->SynName, in );

  //indices of corresponding objects
  fread( iHolder, sizeof( int ), iSize, in );
  i = 0;
  iConnect->FromCol = iHolder[ i++ ];
  iConnect->FromLay = iHolder[ i++ ];
  iConnect->FromCell = iHolder[ i++ ];
  iConnect->FromCmp = iHolder[ i++ ];
  iConnect->ToCol = iHolder[ i++ ];
  iConnect->ToLay = iHolder[ i++ ];
  iConnect->ToCell = iHolder[ i++ ];
  iConnect->ToCmp = iHolder[ i++ ];
  iConnect->SynType = iHolder[ i++ ];
  //iConnect->speed = iHolder[ i++ ];  //->switched to type double
  nbytes += iSize * sizeof( int );

  fread( &iConnect->speed, 1, sizeof( double ), in );
  fread( &iConnect->Prob, 1, sizeof( double ), in );
  nbytes += 2*sizeof( double );

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadCmpConnect( T_CMPCONNECT* &iConnect, FILE *in )
{
  int i, nbytes = 0;
  const int iSize = 2;
  const int dSize = 4;
  int iHolder[iSize];
  double dHolder[dSize];
  
  iConnect = (T_CMPCONNECT *) calloc ( sizeof( T_CMPCONNECT), 1 );
  
  nbytes += loadLocator( iConnect->L, in );

  nbytes += loadString( iConnect->fromCmpName, in );
  nbytes += loadString( iConnect->toCmpName, in );

  fread( iHolder, sizeof( int ), iSize, in );
  i=0;
  iConnect->FromCmp = iHolder[i++];
  iConnect->ToCmp = iHolder[i++];
  //iConnect->Speed = iHolder[i++];
    nbytes += sizeof( int ) * i;

  fread( dHolder, sizeof( double ), dSize, in );
  i=0;
  iConnect->G = dHolder[i++];
  iConnect->retroG = dHolder[i++];
  iConnect->delay = dHolder[i++];
  iConnect->Speed = dHolder[i++];
    nbytes += sizeof( double ) * i;
  
  return nbytes;
}

//-----------------------------------------------------------------------------

int loadTBrain( T_BRAIN* &iBrain, FILE *in )
{
  int i, nbytes = 0;
  const int iSize = 5;
  int iHolder[iSize];

  //if iBrain is NULL, allocate space; otherwise, assume correct amount has 
  //already been allocated
//  if( !iBrain )
  iBrain = (T_BRAIN *) calloc( sizeof( T_BRAIN ), 1 );

  //locator
  nbytes += loadLocator( iBrain->L, in );

  //read all integers
  fread( iHolder, sizeof( int ), iSize, in );
  i=0;
  iBrain->ConnectRpt = iHolder[i++];
  iBrain->SpikeRpt = iHolder[i++];
  iBrain->FSV = iHolder[i++];
  iBrain->nColumns = iHolder[i++];
  iBrain->nConnect = iHolder[i++];
    nbytes += sizeof( int ) * i;

//write in the olny long - it's the SEED value: is there any point to saving it?
  fread( &iBrain->Seed, sizeof( long ), 1, in );
    nbytes += sizeof( long );

//write in only double - it's the DURATION value: won't this be replaced next run?
  fread( &iBrain->Duration, sizeof( double ), 1, in );
    nbytes += sizeof( double );

//  nbytes += loadLocator( iBrain->L, in );
  nbytes += loadString( iBrain->job, in );
  nbytes += loadString( iBrain->distribute, in );

  //write in column info - the list writes in excess info & wastes space
  nbytes += loadList( iBrain->nColumns, iBrain->ColumnNames, in );
  
  iBrain->Columns = (int *) calloc( sizeof( int ), iBrain->nColumns );
  fread( iBrain->Columns, sizeof( int ), iBrain->nColumns, in );
    nbytes += sizeof( int ) * iBrain->nColumns;

  //write in connection info
  iBrain->Connect = (T_CONNECT **) calloc ( sizeof(T_CONNECT*), iBrain->nConnect );
  for( i=0; i<iBrain->nConnect; i++ )
    nbytes += loadConnect( iBrain->Connect[i], in );

  //don't save...
    //save info: savefile, savetime, loadfile
    //port info: Port, Hostname
    //reports and stims: nReports, nStInject, LIST *StInjNames, StInject, ReportNames, Reports

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadTCShell( T_CSHELL* &iShell, FILE *in )
{
  int i, nbytes = 0;
  const int dSize = 4;
  double dHolder[dSize];

  iShell = (T_CSHELL *) calloc( sizeof( T_CSHELL) , 1 );

  nbytes += loadLocator( iShell->L, in );

  fread( dHolder, sizeof( double ), dSize, in );
  i = 0;
  iShell->width = dHolder[ i++ ];
  iShell->height = dHolder[ i++ ];
  iShell->x = dHolder[ i++ ];
  iShell->y = dHolder[ i++ ];
    nbytes += sizeof( double ) * i;

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadTColumn( T_COLUMN* &iColumn, FILE *in )
{
  int i, nbytes = 0;
  const int iSize = 3;
  int iHolder[ iSize ];

  iColumn = (T_COLUMN *) calloc ( sizeof(T_COLUMN), 1 );

  nbytes += loadLocator( iColumn->L, in );
  nbytes += loadString( iColumn->shellName, in );

  fread( iHolder, sizeof( int ), iSize, in );
  i=0;
  iColumn->CShell = iHolder[i++];
  iColumn->nLayers = iHolder[i++];
  iColumn->nConnect = iHolder[i++];
    nbytes += sizeof( int ) * i;

  //write in layer info
  nbytes += loadList( iColumn->nLayers, iColumn->LayerNames, in );
  
  iColumn->Layers = (int *) calloc( sizeof(int), iColumn->nLayers );
  fread( iColumn->Layers, sizeof( int ), iColumn->nLayers, in );
    nbytes += sizeof( int ) * iColumn->nLayers;
	
  //write in connection info
  iColumn->Connect = (T_CONNECT **) calloc ( sizeof(T_CONNECT*), iColumn->nConnect );
  for( i=0; i<iColumn->nConnect; i++ )
    nbytes += loadConnect( iColumn->Connect[i], in );

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadTLShell( T_LSHELL* &iShell, FILE *in )
{
  int nbytes = 0;

  iShell = (T_LSHELL *) calloc ( sizeof(T_LSHELL), 1 );

  nbytes += loadLocator( iShell->L, in );

  //only 2 doubles - just write each in
  fread( &iShell->Lower, sizeof( double ), 1, in );
  fread( &iShell->Upper, sizeof( double ), 1, in );
  nbytes += sizeof( double ) * 2;

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadTLayer( T_LAYER* &iLayer, FILE *in )
{
  int i, nbytes = 0;
  const int iSize = 3;
  int iHolder[iSize];

  iLayer = (T_LAYER *) calloc ( sizeof(T_LAYER), 1 );

  nbytes += loadLocator( iLayer->L, in );
  nbytes += loadString( iLayer->shellName, in );

  fread( iHolder, sizeof( int ), iSize, in );
  i = 0;
  iLayer->LShell = iHolder[i++];
  iLayer->nCellTypes = iHolder[i++];
  iLayer->nConnect = iHolder[i++];
    nbytes += sizeof( int ) * 3;
  //Cell info
  nbytes += loadList( iLayer->nCellTypes, iLayer->CellNames, in );
  iLayer->CellTypes = (int *) calloc ( sizeof( int ), iLayer->nCellTypes );
  iLayer->CellQty = (int *) calloc ( sizeof( int ), iLayer->nCellTypes );
  fread( iLayer->CellTypes, sizeof( int ), iLayer->nCellTypes, in );
  fread( iLayer->CellQty, sizeof( int ), iLayer->nCellTypes, in );
    nbytes += sizeof( int ) * 2 * iLayer->nCellTypes;
  //Connection info
  iLayer->Connect = (T_CONNECT **) calloc ( sizeof(T_CONNECT*), iLayer->nConnect );
  for( i=0; i<iLayer->nConnect; i++ )
    nbytes += loadConnect( iLayer->Connect[i], in );

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadTCell( T_CELL* &iCell, FILE *in )
{
  int i, nbytes = 0;

  iCell = (T_CELL *) calloc ( sizeof(T_CELL), 1 );

  nbytes += loadLocator( iCell->L, in );

  //saving 2 ints
  fread( &iCell->nCmp, sizeof( int ), 1, in );
  fread( &iCell->nConnect, sizeof( int ), 1, in );
    nbytes += sizeof( int ) * 2;

  nbytes += loadList( iCell->nCmp, iCell->CmpNames, in );

  //indices to find compartments in AR (once fully loaded, of course)
  iCell->Cmp = (int *) calloc ( sizeof(int), iCell->nCmp );
  fread( iCell->Cmp, sizeof( int ), iCell->nCmp, in );
    nbytes += sizeof( int ) * iCell->nCmp;

  //individual compartment info
  iCell->Xpos = (double *) calloc ( sizeof( double ), iCell->nCmp );
  iCell->Ypos = (double *) calloc ( sizeof( double ), iCell->nCmp );
  iCell->Zpos = (double *) calloc ( sizeof( double ), iCell->nCmp );
  fread( iCell->Xpos, sizeof( double ), iCell->nCmp, in );
  fread( iCell->Ypos, sizeof( double ), iCell->nCmp, in );
  fread( iCell->Zpos, sizeof( double ), iCell->nCmp, in );
    nbytes += sizeof( double ) * 3 * iCell->nCmp;

  //don't load Rpos - it is only included for compatibility
  //fwrite( &iCell->Rpos, sizeof( double ), iCell->nCmp, in );

  //connection info
  iCell->Connect = (T_CMPCONNECT **) calloc ( sizeof(T_CMPCONNECT *), iCell->nConnect );
  for( i=0; i<iCell->nConnect; i++ )
  {
    //if(iCell->Connect ) //if it's not allocated, we've got bigger problems
      nbytes += loadCmpConnect( iCell->Connect[i], in );
  }

  //reallocate labels and point to appropriate locations
  //patch other data? for example, I write out Xpos/Ypos/Zpos twice - in CmpNames (LIST) and as separate arrays
  //by not saving the individual arrays, I could save some space [not much really]
  iCell->Labels = (char **) calloc ( sizeof(char *), iCell->nCmp );
  LIST *item = iCell->CmpNames;
  for( i=0; i<iCell->nCmp; i++ )
  {
    iCell->Labels[i] = item->label;
	
	//I can also put x,y,z info here and not save it twice (like I currently do )
	iCell->Xpos[i] = item->x;
	iCell->Ypos[i] = item->y;
	iCell->Zpos[i] = item->z;

	item = (LIST *) item->L.next;
  }

  //forward/reverse connections
    // -> this is just data that can be recomputed - don't save
	
  //on the other hand, by saving it, can I keep everything in this function?

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadTCmp( T_CMP* &iCmp, FILE *in )
{
  int i, nbytes = 0;
  const int iSize = 3;
  const int dSize = 22;
  int iHolder[iSize];
  double dHolder[dSize];

  iCmp = (T_CMP *) calloc ( sizeof(T_CMP), 1 );

  nbytes += loadLocator( iCmp->L, in );

  //save SEED?
  fread( &iCmp->Seed, sizeof( long ), 1, in );
    nbytes += sizeof( long );
 
  fread( iHolder, sizeof( int ), iSize, in );
  i=0;
  iCmp->Active = iHolder[i++];
  iCmp->Spike = iHolder[i++];
  iCmp->nChannels = iHolder[i++];
    nbytes += sizeof( int ) * i;

  fread( dHolder, sizeof( double ), dSize, in );
  i=0;
  iCmp->Spike_HW[0] = dHolder[i++];
  iCmp->Spike_HW[1] = dHolder[i++];
  iCmp->Tau_Membrane[0] = dHolder[i++];
  iCmp->Tau_Membrane[1] = dHolder[i++];
  iCmp->R_Membrane[0] = dHolder[i++];
  iCmp->R_Membrane[1] = dHolder[i++];
  iCmp->Threshold [0] = dHolder[i++];
  iCmp->Threshold [1] = dHolder[i++];
  iCmp->Leak_Reversal[0] = dHolder[i++];
  iCmp->Leak_Reversal[1] = dHolder[i++];
  iCmp->VMREST[0] = dHolder[i++];
  iCmp->VMREST[1] = dHolder[i++];
  iCmp->Leak_G [0] = dHolder[i++];
  iCmp->Leak_G [1] = dHolder[i++];
  iCmp->CaInt[0] = dHolder[i++];
  iCmp->CaInt[1] = dHolder[i++];
  iCmp->CaExt[0] = dHolder[i++];
  iCmp->CaExt[1] = dHolder[i++];
  iCmp->CaSpikeInc[0] = dHolder[i++];
  iCmp->CaSpikeInc[1] = dHolder[i++];
  iCmp->CaTau[0] = dHolder[i++];
  iCmp->CaTau[1] = dHolder[i++];
    nbytes += sizeof( double ) * i;
	
  //spike info
  nbytes += loadString( iCmp->SpikeName, in );

  //channel info
  nbytes += loadList( iCmp->nChannels, iCmp->ChannelNames, in );
  iCmp->Channels = (int *) calloc ( sizeof(int), iCmp->nChannels );
  fread( iCmp->Channels, sizeof( int ), iCmp->nChannels, in );
    nbytes += sizeof( int ) * iCmp->nChannels;

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadTChannel( T_CHANNEL* &iChannel, FILE *in )
{
  int i, nbytes = 0;
  const int iSize = 7;
  const int dSize = 56;
  int iHolder[ iSize ];
  double dHolder[ dSize ];

  iChannel = (T_CHANNEL *) calloc ( sizeof(T_CHANNEL), 1 );

  nbytes += loadLocator( iChannel->L, in );
  
  //do I need to save SEED
  fread( &iChannel->Seed, sizeof( long ), 1, in );
    nbytes += sizeof( long );

  fread( iHolder, sizeof( int ), iSize, in );
  i=0;
  iChannel->family = iHolder[i++];
  iChannel->nTauM = iHolder[i++];
  iChannel->nValM = iHolder[i++];
  iChannel->nVoltM = iHolder[i++];
  iChannel->nTauH = iHolder[i++];
  iChannel->nValH = iHolder[i++];
  iChannel->nVoltH = iHolder[i++];
    nbytes += sizeof( int ) * i;

  fread( dHolder, sizeof( double ), dSize, in );
  i=0;
  //try a different technique - use memcpy
  memcpy( &iChannel->unitaryG[0], &dHolder[i], sizeof( double ) * 22 );
    i+=22;
  iChannel->ValM_stdev = dHolder[i++];
  iChannel->VoltM_stdev = dHolder[i++];
  iChannel->ValH_stdev = dHolder[i++];
  iChannel->VoltH_stdev = dHolder[i++];
  memcpy( &iChannel->alphaScaleFactorM[0], &dHolder[i], sizeof( double ) * 30 );
    i+=30;
    nbytes += sizeof( double ) * i;

  //dynamic arrays - should check for size = 0
  if( iChannel->nValM > 0 )
  {
    iChannel->TauValM = (double *) calloc (sizeof(double), iChannel->nValM );
    fread( iChannel->TauValM, sizeof( double ), iChannel->nValM, in );
  }
  if( iChannel->nVoltM > 0 )
  {
    iChannel->TauVoltM = (double *) calloc (sizeof(double), iChannel->nVoltM );
    fread( iChannel->TauVoltM, sizeof( double ), iChannel->nVoltM, in );
  }
  if( iChannel->nValH > 0 )
  {
    iChannel->TauValH = (double *) calloc (sizeof(double), iChannel->nValH );
    fread( iChannel->TauValH, sizeof( double ), iChannel->nValH, in );
  }
  if( iChannel->nVoltH > 0 )
  {
    iChannel->TauVoltH = (double *) calloc (sizeof(double), iChannel->nVoltH );
    fread( iChannel->TauVoltH, sizeof( double ), iChannel->nVoltH, in );
  }
  
    nbytes += sizeof( double ) * ( iChannel->nValM + iChannel->nVoltM + iChannel->nValH + iChannel->nVoltH );

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadTSynapse( T_SYNAPSE* &iSynapse, FILE *in )
{
  int i, nbytes = 0;
  const int iSize = 4;
  const int dSize = 11;
  int iHolder[iSize];
  double dHolder[dSize];

  iSynapse = (T_SYNAPSE *) calloc ( sizeof(T_SYNAPSE), 1 );
  
  nbytes += loadLocator( iSynapse->L, in );

  fread( &iSynapse->Seed, sizeof( long ), 1, in );
    nbytes += sizeof( long );

  nbytes += loadString( iSynapse->SfdName, in );
  nbytes += loadString( iSynapse->LearnName, in );
  nbytes += loadString( iSynapse->DataName, in );
  nbytes += loadString( iSynapse->PsgName, in );

  //try the same technique as channel
  fread( iHolder, sizeof( int ), iSize, in );
  i=0;
  memcpy( &iSynapse->SfdIdx, &iHolder[i], sizeof( int ) * 4 );
  i+= 4;
    nbytes += sizeof( int ) * i;

  fread( dHolder, sizeof( double ), dSize, in );
  i=0;
  memcpy( &iSynapse->USE[0], &dHolder[i], sizeof( double ) * 11 );
  i+=11;
    nbytes += sizeof( double ) * i;
	
  return nbytes;
}

//-----------------------------------------------------------------------------

int loadTSynLearn( T_SYNLEARN* &iLearn, FILE *in )
{
  int i, nbytes = 0;
  const int dSize = 12;
  double dHolder[dSize];

  iLearn = (T_SYNLEARN *) calloc ( sizeof(T_SYNLEARN), 1 );

  nbytes += loadLocator( iLearn->L, in );

  fread( &iLearn->Seed, sizeof( long ), 1, in );
    nbytes += sizeof( long );

  //only two ints
  fread( &iLearn->nPosLearn, sizeof( int ), 1, in );
  fread( &iLearn->nNegLearn, sizeof( int ), 1, in );
    nbytes += sizeof( int ) * 2;
  
  i=0;
  fread( dHolder, sizeof( double ), dSize, in );
  memcpy( &iLearn->Neg_Heb_Peak_Delta_Use[0], &dHolder[i], sizeof( double ) * 12 );
  i += 12;
    nbytes += sizeof( double ) * i;
	
  //dynamic arrays - need to go back and test previous objects for nItems > 0
  if( iLearn->nPosLearn > 0 )
  {
    iLearn->PosLearnTable = (double *) calloc ( sizeof(double), iLearn->nPosLearn );
    fread( iLearn->PosLearnTable, sizeof( double ), iLearn->nPosLearn, in );
  }
  if( iLearn->nNegLearn > 0 )
  {
	iLearn->NegLearnTable = (double *) calloc ( sizeof(double), iLearn->nNegLearn );
    fread( iLearn->NegLearnTable, sizeof( double ), iLearn->nNegLearn, in );
  }
    nbytes += sizeof( double ) * ( iLearn->nPosLearn + iLearn->nNegLearn );

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadTSynFD( T_SYN_FD* &iFD, FILE *in )
{
  int i, nbytes = 0;
  const int dSize = 4;
  double dHolder[dSize];

  iFD = (T_SYN_FD *) calloc ( sizeof(T_SYN_FD), 1 );

  nbytes += loadLocator( iFD->L, in );

  fread( &iFD->Seed, sizeof( long ), 1, in );
    nbytes += sizeof ( long );

  fread( &iFD->SFD, sizeof( int ), 1, in );
    nbytes += sizeof( int );

  fread( dHolder, sizeof( double ), dSize, in );
  i=0;
  memcpy( &iFD->Facil_Tau[0], &dHolder[0], sizeof( double ) * 4 );
  i += 4;
    nbytes += sizeof( double ) * 4;

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadTSynPSG( T_SYNPSG* &iPsg, FILE *in )
{
  int nbytes = 0;

  iPsg = (T_SYNPSG *) calloc ( sizeof(T_SYNPSG), 1 );
  nbytes += loadLocator( iPsg->L, in );

  fread( &iPsg->nPSG, sizeof( int ), 1, in );
    nbytes += sizeof( int );

  iPsg->PSG = (double *) calloc ( sizeof(double), iPsg->nPSG );
  fread( iPsg->PSG, sizeof( double ), iPsg->nPSG, in );
    nbytes += sizeof( double ) * iPsg->nPSG;

  //don't save file name where the data was obtained from

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadTSynData( T_SYNDATA* &iData, FILE *in )
{
  int i, nbytes = 0;
  const int dSize = 6;
  double dHolder[dSize];

  iData = (T_SYNDATA *) calloc ( sizeof(T_SYNDATA), 1 );
  
  nbytes += loadLocator( iData->L, in );

  fread( &iData->Seed, sizeof( long ), 1, in );
    nbytes += sizeof( long );

  fread( dHolder, sizeof( double ), dSize, in );
  i=0;
  memcpy( &iData->MaxG, &dHolder[0], sizeof( double ) * dSize );
  i += dSize;
    nbytes += sizeof( double );

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadTSpike( T_SPIKE* &iSpike, FILE *in )
{
  int nbytes = 0;

  iSpike = (T_SPIKE *) calloc ( sizeof(T_SPIKE), 1 );

  nbytes += loadLocator( iSpike->L, in );
  fread( &iSpike->nVoltages, sizeof( int ), 1, in );
    nbytes += sizeof( int );
  iSpike->Voltages = (double *) calloc ( sizeof(double), iSpike->nVoltages );
  fread( iSpike->Voltages, sizeof( double ), iSpike->nVoltages, in );
    nbytes += sizeof( double ) * iSpike->nVoltages;

  return nbytes;
}

//-----------------------------------------------------------------------------

int loadArrays( ARRAYS* &AR, FILE *in )
{
  int i, nbytes = 0;
  const int iSize = 18;
  int iHolder[iSize];
  ARRAYS *oldArs = AR;

  //preserver Brain, Stims, and Reports from Arrays
  T_BRAIN     *newBrain = AR->Brain;
  T_STIMULUS **newStimulus = AR->Stimulus;
  T_STINJECT **newStInject = AR->StInject;
  T_REPORT   **newReport = AR->Reports;

  //allocate ARRAYS
  AR = (ARRAYS *) calloc ( sizeof( ARRAYS ), 1 );

  //read in ints
  fread( iHolder, sizeof( int ), iSize, in );
  i = 0;
  AR->LayerCount = iHolder[i++];
  AR->CellCount = iHolder[i++];
  AR->ClusterCount = iHolder[i++];
  AR->CmpCount = iHolder[i++];
  AR->ChanCount = iHolder[i++];
  AR->nCsh = iHolder[i++];
  AR->nColumns = iHolder[i++];
  AR->nLsh = iHolder[i++];
  AR->nLayers = iHolder[i++];
  AR->nCells = iHolder[i++];
  AR->nCmp = iHolder[i++];
  AR->nChannel = iHolder[i++];
  AR->nSynapse = iHolder[i++];
  AR->nSynLearn = iHolder[i++];
  AR->nSynFD = iHolder[i++];
  AR->nSynPSG = iHolder[i++]; 
  AR->nSynData = iHolder[i++];
  AR->nSpike = iHolder[i++];
  nbytes += sizeof( int ) * iSize;

  nbytes += loadTBrain( AR->Brain, in );
  //move certain pieces of new Brain data back into AR
  if( newBrain->job ) //if this is NULL, then leave job as loaded
    AR->Brain->job = newBrain->job;
    
  AR->Brain->Duration = newBrain->Duration;
  
  if( newBrain->FSV ) //will this be expected to be reused?
    AR->Brain->FSV = newBrain->FSV;
    
  if( newBrain->savefile )
  {
    AR->Brain->savefile = newBrain->savefile;
    AR->Brain->savetime = newBrain->savetime;
  }
    
  //do I need to remember loadfile?
  AR->Brain->loadfile = newBrain->loadfile;
  
  AR->Brain->nStInject = newBrain->nStInject;
  AR->Brain->StInjNames = newBrain->StInjNames;
  AR->Brain->StInject = newBrain->StInject;
  
  AR->Brain->nReports = newBrain->nReports;
  AR->Brain->ReportNames = newBrain->ReportNames;
  AR->Brain->Reports = newBrain->Reports;

  //done preserving specific new brain info
  AR->Csh = (T_CSHELL **) calloc ( sizeof( T_CSHELL *), AR->nCsh );
  for( i=0; i<AR->nCsh; i++ )
    nbytes += loadTCShell( AR->Csh[i], in );

  AR->Columns = (T_COLUMN **) calloc ( sizeof(T_COLUMN *), AR->nColumns );
  for( i=0; i<AR->nColumns; i++ )
    nbytes += loadTColumn( AR->Columns[i], in );

  AR->Lsh = (T_LSHELL **) calloc ( sizeof(T_LSHELL *), AR->nLsh );
  for( i=0; i<AR->nLsh; i++ )
    nbytes += loadTLShell( AR->Lsh[i], in );

  AR->Layers = (T_LAYER **) calloc (sizeof(T_LAYER *), AR->nLayers );
  for( i=0; i<AR->nLayers; i++ )
    nbytes += loadTLayer( AR->Layers[i], in );

  AR->Cells = (T_CELL **) calloc ( sizeof( T_CELL *), AR->nCells );
  for( i=0; i<AR->nCells; i++ )
    nbytes += loadTCell( AR->Cells[i], in );

  AR->Cmp = (T_CMP **) calloc ( sizeof(T_CMP *), AR->nCmp );
  for( i=0; i<AR->nCmp; i++ )
    nbytes += loadTCmp( AR->Cmp[i], in );

  AR->Channel = (T_CHANNEL **) calloc ( sizeof(T_CHANNEL *), AR->nChannel );
  for( i=0; i<AR->nChannel; i++ )
    nbytes += loadTChannel( AR->Channel[i], in );

  AR->Synapse = (T_SYNAPSE **) calloc ( sizeof(T_SYNAPSE *), AR->nSynapse );
  for( i=0; i<AR->nSynapse; i++ )
    nbytes += loadTSynapse( AR->Synapse[i], in );

  AR->SynLearn = (T_SYNLEARN **) calloc ( sizeof( T_SYNLEARN *), AR->nSynLearn );
  for( i=0; i<AR->nSynLearn; i++ )
    nbytes += loadTSynLearn( AR->SynLearn[i], in );

  AR->SynFD = (T_SYN_FD **) calloc ( sizeof(T_SYN_FD *), AR->nSynFD );
  for( i=0; i<AR->nSynFD; i++ )
    nbytes += loadTSynFD( AR->SynFD[i], in );

  AR->SynPSG = (T_SYNPSG **) calloc ( sizeof(T_SYNPSG *), AR->nSynPSG );
  for( i=0; i<AR->nSynPSG; i++ )
    nbytes += loadTSynPSG( AR->SynPSG[i], in );

  AR->SynData = (T_SYNDATA **) calloc ( sizeof(T_SYNDATA *), AR->nSynData );
  for( i=0; i<AR->nSynData; i++ )
    nbytes += loadTSynData( AR->SynData[i], in );

  AR->Spike = (T_SPIKE **) calloc ( sizeof(T_SPIKE), AR->nSpike );
  for( i=0; i<AR->nSpike; i++ )
    nbytes += loadTSpike( AR->Spike[i], in );

  //before leaving - reconfigure cmp connection pointers
  for( i=0; i<AR->nCells; i++ )
    buildCellCmpConnects( AR->Cells[i] );

  //patch pointers for reports and stims, and find indexes
    //stims don't need conversion since they don't contain names of objects
  AR->Stimulus = newStimulus;
  AR->nStimulus = oldArs->nStimulus;

  //manage Stimulus Injects - find names and associated indices
  AR->StInject = newStInject;
  AR->nStInject = oldArs->nStInject;
  for( i=0; i<AR->nStInject; i++ )
  {
    AR->StInject[i]->STIM_TYPE = findObject( AR->StInject[i]->StimName );
    AR->StInject[i]->Column = findObject( AR->StInject[i]->ColName );
    AR->StInject[i]->Layer = findObject( AR->StInject[i]->LayName );
    AR->StInject[i]->Cell = findObject( AR->StInject[i]->CellName );

    int found = 0; //cmp; need to find within cell
    for( int j=0; j<AR->Cells[ AR->StInject[i]->Cell ]->nCmp & !found; j++ )
    {
      if( strcmp( AR->StInject[i]->CmpName, AR->Cells[ AR->StInject[i]->Cell ]->Labels[j] ) == 0 )
      {
        AR->StInject[i]->Cmp = j;
        found = 1;
      }
    }
    if( !found ) //not in list
      fprintf( stderr, "Error: could not find Cmp named %s for stiminj\n", oldArs->StInject[i]->CmpName );
  }
  
  //Manage Reports
  AR->nReports = oldArs->nReports;
  AR->Reports = newReport;
  for( i=0; i<AR->nReports; i++ )
  {
    AR->Reports[i]->Column = findObject( AR->Reports[i]->ColName );
    AR->Reports[i]->Layer  = findObject( AR->Reports[i]->LayName );
    AR->Reports[i]->Cell   = findObject( AR->Reports[i]->CellName );

    int found = 0; //cmp - unique in that we must look through the cell labels
    for( int j=0; j<AR->Cells[AR->Reports[i]->Cell]->nCmp & !found; j++ )
    {
      if( strcmp( AR->Reports[i]->CmpName, AR->Cells[AR->Reports[i]->Cell]->Labels[j] ) == 0 )
      {
        AR->Reports[i]->Cmp = j;
        found = 1;
      }
    }
    if( !found ) //not in list
      fprintf( stderr, "Error: could not find Cmp named %s\n", AR->Reports[i]->CmpName );
      
    //special - 
    if ( AR->Reports[i]->ReportOn == SYNAPSE_USE || AR->Reports[i]->ReportOn == SYNAPSE_RSE ||
         AR->Reports[i]->ReportOn == SYNAPSE_UF  || AR->Reports[i]->ReportOn == SYNAPSE_SA ||
         AR->Reports[i]->ReportOn == SYNAPSE_CA )
    {
        AR->Reports[i]->Thing = findObject( AR->Reports[i]->Name );
    }
    else if( AR->Reports[i]->ReportOn == FIRE_COUNT)
    {
        AR->Reports[i]->reportFlag |= INTEGER_CAST;
    }
    else if (AR->Reports[i]->ReportOn == CHANNEL_RPT)
    {
        AR->Reports[i]->Thing = findObject( AR->Reports[i]->Name );
    }
  }

  //I don't need the name tree anymore, clear it
  oNames.clear();

  return nbytes;
}

int findObject( char* name )
{
  ObjectName searchObject;
  searchObject.name = name;
  set< ObjectName, std::less< ObjectName > >::iterator result;

  result = oNames.find( searchObject );

  if( result == oNames.end() )
  {
    fprintf( stderr, "Error: unable to find object named %s\n", name );
    return -1;
  }
  else
    return result->index;
}


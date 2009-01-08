/* This is the Cell object.  It does nothing except loop through the         */
/* compartments that make up the cell                                        */

#include <stdio.h>

#include "Cell.h"
#include "Managers.h"
#include "debug.h"
#include "memstat.h"

#define MEM_KEY  KEY_CELL
Cell::Cell ()      
{
}


Cell::Cell (int CELLID, int iColDef, int iLayDef, int iCelDef)
{
  MEMADDOBJ (MEM_KEY);

  CellID = CELLID;       // ID of cell on machine (0...nCells-1)
  idx    = iCelDef;
}

Cell::~Cell ()      // class destructor
{
  int i;
  
  MEMFREEOBJ (MEM_KEY);

  for (i = 0; i < nCompartments; i++) //delete all compartment objects
    delete Compartments [i];
}

/*---------------------------------------------------------------------------*/
/* This is the main cell loop, which is called from the main brain loop.  It */
/* is just a container which calls the compartments that make up the cell.   */

void Cell::DoProcessCell (int TimeStep)
{       
  int i;

  for (i = 0; i < nCompartments; i++)
  {
    Compartments [i]->DoProcessCompartment (TimeStep);
  }

  for (i = 0; i < nCompartments; i++)
    Compartments[i]->UpdateBuffer();
}

/*---------------------------------------------------------------------------*/
/* write cell data out to file                                               */

int Cell::Save (FILE *out)
{
  int i=0;
  int bytes = 0;
  const int iSize = 3;
  int iholder[iSize];

  //save data individually
  iholder[i++] = CellID;
  iholder[i++] = idx;
  iholder[i++] = nCompartments;
  fwrite( iholder, sizeof( int ), iSize, out );
  bytes += iSize*sizeof(int);

  for (i = 0; i < nCompartments; i++)
    bytes += Compartments [i]->Save (out);

  return bytes;
}

//-----------------------------------------------------------------------------

//alter to have it gather synapse addressses for a specified node into a vector
//return to brain
int Cell::SaveConnections( vector<Synapse *> &collection, int targetNode )//FILE *out )
{
  int nbytes = 0;
  
  //for each compartment, save synapse and sendto info
  for( int i=0; i<nCompartments; i++ )
    nbytes += Compartments[i]->SaveConnections( collection, targetNode ); //out );
  
  return nbytes;
}

/*---------------------------------------------------------------------------*/
/* read in cell data from a file.  Assume same amount of memory available,   */
/* but not same addresses                                                    */

void Cell::CellReset (FILE *in, char *nothing)
{
}

/*---------------------------------------------------------------------------*/
/* read in new cell data from a file into fresh data structure               */

void Cell::Load (FILE *in, char *nothing)
{
  int i=0;
  const int iSize = 3;
  int iholder[iSize];

  fread( iholder, sizeof( int ), iSize, in );
  CellID = iholder[i++];
  idx = iholder[i++];
  nCompartments = iholder[i++];
  Compartments = NULL;

  //vCompartments is a list of pointers - must reallocate it as that
  Compartments = (Compartment **) calloc (nCompartments, sizeof (Compartment *));
  for( int i=0; i<nCompartments; i++ )
  {
   Compartments[i] = new Compartment;
   Compartments[i]->Load( in, nothing );
  }
  
}

//---------------------------------------------------------------------------

void Cell::LoadConnections( deque<Synapse *> &collection, int targetNode )
{
  int i;
  
  //load compartment connection data once all memory has been allocated using
  //the normal load function
  for( i=0; i<nCompartments; i++ )
  {
    Compartments[i]->LoadConnections( collection, targetNode );
  }
}


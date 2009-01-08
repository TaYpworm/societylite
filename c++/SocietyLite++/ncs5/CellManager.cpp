/* Cell Manager source code - See extensive comments at end of file - JF */

#include "CellManager.h"
#include "Managers.h"
#include "debug.h"
#include "memstat.h"

#define MEM_KEY  KEY_CELLMGR

CellManager::CellManager ()
{
  MEMADDOBJ (MEM_KEY);
  HasError = DistDone = false;
}

/*------------------------------------------------------------------------*/

CellManager::~CellManager ()
{
  MEMFREEOBJ (MEM_KEY);
}

/*---------------------------------------------------------------------------*/
/* This makes the Global Cluster List, which has an entry for each cluster   */
/* in the brain.                                                             */


void CellManager::MakeGCList (ARRAYS *AR, RandomManager *RM)
{
  T_BRAIN  *Brain;
  T_COLUMN *Col;
  T_CSHELL *Csh;
  T_LAYER  *Lay;
  T_LSHELL *Lsh;
  T_CELL   *Cel;
  T_CONNECT *con;
  CLUSTER *CS;
  CONNECT *CD;
  double xpos, ypos, zpos;
  int i, j, k, iCS, count, idx, ColID, LayID;

//printf ("CellManager::MakeGCList: cells = %d, clusters = %d\n", AR->CellCount, AR->ClusterCount);

  RN = RM->Connect;


/* Allocate the arrays (sizes are known from input).  Note that the Clusters */
/* list must be an array of Cluster pointers so that it can be sorted for    */
/* the lookup functions.                                                     */

  Clusters = (CLUSTER **) calloc (AR->ClusterCount, sizeof (CLUSTER *));
  CSData   = (CLUSTER *)  calloc (AR->ClusterCount, sizeof (CLUSTER));

/* Starting from the Brain, iterate through its Columns and Layers, and for  */
/* each CELL_TYPE specifier (= Cluster definition), fill in the appropriate  */
/* entries in the Global Cluster List.  Note that this code skips clusters   */
/* that are defined in input, but not actually used in the brain.            */

  nGlobalCells = iCS = 0;
  Brain = AR->Brain;
  for (i = 0; i < Brain->nColumns; i++)      /* For each column in the brain */
  {
    Col     = AR->Columns [Brain->Columns [i]];       /* Get column & column */
    Csh     = AR->Csh [Col->CShell];                  /* shell definitions   */
    xpos    = Csh->x;
    ypos    = Csh->y;

    for (j = 0; j < Col->nLayers; j++)       /* For each layer in the column */
    {
      Lay  = AR->Layers [Col->Layers [j]];           /* Get layer and layer  */
      Lsh  = AR->Lsh [Lay->LShell];                  /* shell definitions    */
      zpos = Csh->height * 0.005 * (Lsh->Upper + Lsh->Lower);    /* height is % */

      for (k = 0; k < Lay->nCellTypes; k++)   /* for all cell clusters in layer */
      {   
        Cel = AR->Cells [Lay->CellTypes [k]];

        CS = &(CSData [iCS]);            /* Fill in Cluster entry for this cluster */
        CS->iCol = Brain->Columns [i];
        CS->iLay = Col->Layers    [j];
        CS->iCel = Lay->CellTypes [k];
        CS->Qty  = Lay->CellQty   [k];
        nGlobalCells += CS->Qty;

        CS->x    = xpos;
        CS->y    = ypos;
        CS->z    = zpos;

        CS->iSelf = iCS;
        Clusters [iCS++] = CS;            /* Fill in Cluster entry for this cluster */
      } 
    } 
  }
  nClusters = iCS;

  CSSort (Clusters, nClusters);

  NI->CellCount = nGlobalCells;

/* The GCList has been created, now loop through the connection definitions, */
/* adding connection info to the GCList.  The first step is to count the     */
/* number of connect groups (from and to separately) that each cluster has.  */
/* Error checking for existence of clusters is done here, so that if it's    */
/* going to bomb out, it will do so ASAP.                                    */

/* First get all Column-Column (brain level) connects. */

  count = 0;
  for (i = 0; i < Brain->nConnect; i++)
  {
    con  = Brain->Connect [i];
    CheckCD (con, &count, AR, "Column");
  }

/* Now the column-level connections, which connect layers within a column. */

  for (i = 0; i < Brain->nColumns; i++)
  {
    Col   = AR->Columns [Brain->Columns [i]];
    ColID = Col->L.idx;
    for (j = 0; j < Col->nConnect; j++)
    {
      con  = Col->Connect [j];             
      con->FromCol = con->ToCol = ColID;
      CheckCD (con, &count, AR, "Layer");
    }

/* For each layer in the column, the connects between clusters in that layer */

    for (j = 0; j < Col->nLayers; j++)  
    {
      Lay   = AR->Layers [Col->Layers [j]];
      LayID = Lay->L.idx;
      for (k = 0; k < Lay->nConnect; k++)                  /* and for each connect there */
      {
        con  = Lay->Connect [k];                           /* get the connect definition */
        con->FromCol = con->ToCol = ColID;
        con->FromLay = con->ToLay = LayID;
        CheckCD (con, &count, AR, "Cell");
      }
    }
  } 

/* The connects are counted, now create them, and go through GCList again    */
/* and fill them in.                                                         */

  CDList = (CONNECT *) calloc (count, sizeof (CONNECT));   
  nCD    = count;

  maxQty = 0;
  for (i = 0; i < nClusters; i++)
  {
    CS = Clusters [i];
    if (CS->Qty > maxQty) maxQty = CS->Qty;
  }

  maxCon = idx = 0;
  for (i = 0; i < Brain->nConnect; i++)
  {
    con  = Brain->Connect [i];
    CD  = &(CDList [idx++]);
    AddCDToLists (con, CD);
  }

  for (i = 0; i < Brain->nColumns; i++)
  {
    Col   = AR->Columns [Brain->Columns [i]];
    ColID = Col->L.idx;
    for (j = 0; j < Col->nConnect; j++)
    {
      con  = Col->Connect [j];             
      con->FromCol = con->ToCol = ColID;
      CD  = &(CDList [idx++]);
      AddCDToLists (con, CD);
    }

/* And for each layer in the column, connect the cells in that layer */

    for (j = 0; j < Col->nLayers; j++)  
    {
      Lay   = AR->Layers [Col->Layers [j]];
      LayID = Lay->L.idx;
      for (k = 0; k < Lay->nConnect; k++)                    /* and for each connect there */
      {
        con  = Lay->Connect [k];                           /* get the connect definition */
        con->FromCol = con->ToCol = ColID;
        con->FromLay = con->ToLay = LayID;
        CD  = &(CDList [idx++]);
        AddCDToLists (con, CD);
      }
    }
  } 

  if (NI->Node == 0)
    printf ("CellManager::MakeGCList: %d clusters, %d connects, maxCon = %d, maxQty = %d\n",
            nClusters, count, maxCon, maxQty);
}

/*---------------------------------------------------------------------------*/
/* Check and count connect descriptor                                        */

void CellManager::CheckCD (T_CONNECT *con, int *count, ARRAYS *AR, char *type)
{
  CLUSTER  *From, *To;

  From = GetCSItem (con->FromCol, con->FromLay, con->FromCell);
  To   = GetCSItem (con->ToCol,   con->ToLay,   con->ToCell);

  if (From == NULL) 
    Error1 (type, 0, con, AR);
  else if (To == NULL)
    Error1 (type, 1, con, AR);
  else
    (*count)++;
}

/*---------------------------------------------------------------------------*/
/* Add connect descriptor to lists                                           */

void CellManager::AddCDToLists (T_CONNECT *con, CONNECT *CD)
{
  CLUSTER *From, *To;
  int nCon;

  From = GetCSItem (con->FromCol, con->FromLay, con->FromCell);
  To   = GetCSItem (con->ToCol,   con->ToLay,   con->ToCell);

  nCon   = (int) ((double (From->Qty * To->Qty) * con->Prob));
  if (nCon > maxCon) maxCon = nCon;
  From->nSendTo += nCon;
  To->nSyn += nCon;

  CD->From = From;
  CD->To   = To;

  CD->nCon = nCon;
  CD->Seed = -RN->iRand ();
  CD->iCon = con;
}

/*---------------------------------------------------------------------------*/
/* Sorts list of clusters by Col, Layer, Cell.                               */

void CellManager::CSSort (CLUSTER *array [], int len)
{
  CLUSTER *tmp;
  int i, j, gap;

  for (gap = len / 2; gap > 0; gap /= 2)
  {
    for (i = gap; i < len; i++)
    {
      for (j = i - gap; j >= 0; j -= gap)
      {
        if (CScompare (array [j], array [j + gap]) <= 0) break;
        tmp = array [j];
        array [j] = array [j+gap];
        array [j+gap] = tmp;
      }
    }
  }
}

/*------------------------------------------------------------------------*/

int CellManager::CScompare (CLUSTER *a, CLUSTER *b)
{
  int rc;

  if (a->iCol < b->iCol)
    rc = -1;
  else if (a->iCol > b->iCol)
    rc = 1;
  else
  {
    if (a->iLay < b->iLay)
      rc = -1;
    else if (a->iLay > b->iLay)
      rc = 1;
    else
    {
      if (a->iCel < b->iCel)
        rc = -1;
      else if (a->iCel > b->iCel)
        rc = 1;
      else
        rc = 0;
    }
  }
  return (rc);
}
 
/*------------------------------------------------------------------------*/
/* Locate a cluster by Column, Layer, Cell ids.  Returns index of Cluster,   */
/* or -1 if not found.  Standard binary search: assumes CSList is sorted!    */

int CellManager::FindCSItem (int Col, int Lay, int Cel)
{
  CLUSTER *CS;
  int low, high, mid, cmp;

  low = 0;
  high = nClusters - 1;
  while (low <= high)
  {
    mid = low + (high - low) / 2;
    CS = Clusters [mid];

    if (Col < CS->iCol)
      cmp = -1;
    else if (Col > CS->iCol)
      cmp = 1;
    else 
    {
      if (Lay < CS->iLay)
        cmp = -1;
      else if (Lay > CS->iLay)
        cmp = 1;
      else 
      {
        if (Cel < CS->iCel)
          cmp = -1;
        else if (Cel > CS->iCel)
          cmp = 1;
        else 
          cmp = 0;
      }
    }

    if (cmp < 0)
      high = mid - 1;
    else if (cmp > 0)
      low = mid + 1;
    else
    {
      return (mid);
    }
  }
  return (-1);
}

/*------------------------------------------------------------------------*/
/* Return pointer to Clusters structure for Col, Lay, Cell cluster.  The    */
/* idx member of this is the index of the first matching cell in GCList.  */

CLUSTER *CellManager::GetCSItem (int Col, int Lay, int Cel)
{
  CLUSTER *CS;
  int idx;
     
  CS = NULL;
  idx = FindCSItem (Col, Lay, Cel);
  if (idx >= 0) CS = Clusters [idx];
  return (CS);
}

/*------------------------------------------------------------------------*/
/* The Node & Cell values in GCList aren't valid until this is set...     */

void CellManager::SetDistFlag ()
{
  DistDone = true;
}

/*---------------------------------------------------------------------------*/
/* Get the first CellID matching (Col, Lay, Cel).  Assumes CellIDs in GCList */
/* are sequential and in increasing order.                                   */

int CellManager::GetFirstCellID (int Col, int Lay, int Cel)
{
  CLUSTER *CS;
  int idx, id;
     
  id = -1;
  idx = FindCSItem (Col, Lay, Cel);
  if (idx >= 0) 
  {
    CS = Clusters [idx];
    id = CS->idx;
  }
  return (id);
}

/*---------------------------------------------------------------------------*/
/* Returns an array matching the CLC arguments.  The array size is the       */
/* quantity of cells; elements contain either the local CellID if it is on   */
/* the node, or -1 if not.  Allocates memory which the caller should free.   */

int *CellManager::ListCells (int Col, int Lay, int Cel, int *qty, int *count)
{
  CLUSTER *CS;
  int i, idx, *list;
     
  list = NULL;
  *count = *qty = 0;
  if (DistDone)            /* Not meaningful until this is true */
  {
    CS = NULL;
    idx = FindCSItem (Col, Lay, Cel);
    if (idx >= 0) 
    {
      CS = Clusters [idx];
      idx = CS->idx;
      *qty = CS->Qty;
      list = (int *) calloc (CS->Qty, sizeof (int));
      if (CS->Node == NI->Node)
      {
        *count = CS->Qty;
        for (i = 0; i < CS->Qty; i++)
          list [i] = idx++;
      }
      else
      {
        for (i = 0; i < CS->Qty; i++)
          list [i] = -1;
      }
    }
  }

  return (list);
}

/*---------------------------------------------------------------------------*/
/* Returns an array of cells (GCList indexes) on the node which match the    */
/* CLC arguments.  This allocates memory for the list: the caller should     */
/* free it.                                                                  */

int *CellManager::ListCellsOnNode (int Col, int Lay, int Cel, int *count)
{
  CLUSTER *CS;
  int i, idx, *list;
     
  list = NULL;
  *count = 0;

  if (DistDone)            /* Not meaningful until this is true */
  {
    CS = NULL;
    idx = FindCSItem (Col, Lay, Cel);
    if (idx >= 0) 
    {
      CS = Clusters [idx];
      if (CS->Node == NI->Node)
      {
        idx = CS->idx;
        *count = CS->Qty;
        list = (int *) calloc (CS->Qty, sizeof (int));
        for (i = 0; i < CS->Qty; i++)
          list [i] = idx++;
      }
    }
  }

  return (list);
}

/*---------------------------------------------------------------------------*/

void CellManager::Error1 (char *name, int side, T_CONNECT *con, ARRAYS *AR)
{
  T_COLUMN *Col;
  T_LAYER  *Lay;
  T_CELL   *Cel;
  char *which;

  if (side == 0)
  {
    Col = AR->Columns [con->FromCol];
    Lay = AR->Layers  [con->FromLay];
    Cel = AR->Cells   [con->FromCell];
    which = "From";
  }
  else
  {
    Col = AR->Columns [con->ToCol];
    Lay = AR->Layers  [con->ToLay];
    Cel = AR->Cells   [con->ToCell];
    which = "To";
  }

  printf ("CellManager::Error %s:%d: Cluster not found for %s side of %s connect\n  Missing cluster is (%s, %s, %s)\n",
           con->L.file, con->L.line, which, name, Col->L.name, Lay->L.name, Cel->L.name);

  HasError = true;
}

/*---------------------------------------------------------------------------*/

void CellManager::FreeCDList ()
{
  free (CDList);
  CDList = NULL;
  nCD    = 0;
}

/*---------------------------------------------------------------------------*/
/* Write Cluster list to a file                                              */

void CellManager::PrintClusterList (ARRAYS *AR)
{
  CLUSTER *CS;
//CONNECT *CD;
  int i;

  printf ("Cluster list - %d clusters\n", nClusters);
  printf ("      Cell idx   Qty  (  C   L   C)     nSyn    weight  node  nsend  nrecv\n");
  for (i = 0; i < nClusters; i++)
  {
    CS = Clusters [i];
    printf ("!CL %5d: (%5d, %5d) -> %2d\n", i, CS->idx, CS->Qty, CS->Node);

//  for (j = 0; j < C->nCD; j++)
//  {
//    CD = (CONNECT *) C->CDList [j];
//   printf ("    %4d->%4d: %4d, %6d\n", j, CD->From->iSelf, CD->To->iSelf, CD->nCon);
//  }
  }
}

/*---------------------------------------------------------------------------*/
//Save/Load functions

int CellManager::Save( FILE *out )
{
  int iHolder[9], i, k, nbytes=0;
  double dHolder[4];
  CLUSTER *CS;

  if( NI->Node != 0 )  //in case this function is called by any node except the head node
    return 0;

  //write out an int (later overwrite it wil nbytes) - do I include this in the byte count?
  int recall = ftell( out );  //so I can track back
  fwrite( &nbytes, sizeof( int ), 1, out );
    nbytes += sizeof( int );

  //bools HasError, DistDone arn't needed
  i=0;
  iHolder[i++] = nLocalCells;
  iHolder[i++] = nGlobalCells;
  iHolder[i++] = nClusters;  //I could forgo saving this and get it from AR->ClusterCount
  fwrite( iHolder, sizeof( int ), i, out );
    nbytes += sizeof( int ) * i;

  //If I save using CSData, I will need to resort after I load,
  //If I save using Clusters, I will write the data out in order which could have unseen effects
  for( k=0; k<nClusters; k++ )
  {
    CS = &CSData[k];

    i=0;
    iHolder[i++] = CS->iCol;
    iHolder[i++] = CS->iLay;
    iHolder[i++] = CS->iCel;
    iHolder[i++] = CS->idx;
    iHolder[i++] = CS->Qty;
    iHolder[i++] = CS->nSendTo;
    iHolder[i++] = CS->nSyn;
    iHolder[i++] = CS->Node;
    iHolder[i++] = CS->iSelf;
    fwrite( iHolder, sizeof( int ), i, out );
      nbytes += sizeof( int ) * i;

    i=0;
    dHolder[i++] = CS->weight;
    dHolder[i++] = CS->x;
    dHolder[i++] = CS->y;
    dHolder[i++] = CS->z;
    fwrite( dHolder, sizeof( double ), i, out );
      nbytes += sizeof( double ) * i;
  }

  //go back to overwrite nbytes with correct value
  fseek( out, recall, SEEK_SET );
  fwrite( &nbytes, sizeof( int ), 1, out );
  fseek( out, 0, SEEK_END );

  return nbytes;

}

/*---------------------------------------------------------------------------*/

void CellManager::Load( FILE *in )
{
  int i, k, nbytes=0;
  CLUSTER *CS;
  char *dataStream = NULL;
  double *dArea;
  int *iArea;

  if( NI->Node == 0 ) //Node 0 will read from file and scatter to everyone
  {
    //read in nbytes
    fread( &nbytes, sizeof( int ), 1, in );
      nbytes -= sizeof( int ); //deduct size of itself
  }

  MPI_Bcast( &nbytes, 1, MPI_INT, 0, MPI_COMM_WORLD );
  dataStream = new char[ nbytes];

  if( NI->Node == 0 )
    fread( dataStream, nbytes, sizeof( char ), in );

  MPI_Bcast( dataStream, nbytes, MPI_CHAR, 0, MPI_COMM_WORLD );

  //now, everyone has the data, they can write it to a local file and read it in properly
  //or I could just copy the data from memory
    //I'll try plan B and hope it works

  i = 0;
  iArea = (int*) dataStream;
  nLocalCells = iArea[i++];
  nGlobalCells = iArea[i++];
  nClusters = iArea[i++];

  //If I save using CSData, I will need to resort after I load,
  //If I save using Clusters, I will write the data out in order which could have unseen effect
  Clusters = (CLUSTER **) calloc (nClusters, sizeof (CLUSTER *));
  CSData   = (CLUSTER *)  calloc (nClusters, sizeof (CLUSTER));

  for( k=0; k<nClusters; k++ )
  {
    CS = &CSData[k];

    CS->iCol = iArea[i++];
    CS->iLay = iArea[i++];
    CS->iCel = iArea[i++];
    CS->idx = iArea[i++];

    CS->Qty = iArea[i++];
    CS->nSendTo = iArea[i++];
    CS->nSyn = iArea[i++];
    CS->Node = iArea[i++];
    CS->iSelf = iArea[i++];

    //move dArea to where iArea is and reset i
    dArea = (double *) &(iArea[i]);
    i = 0;

    CS->weight = dArea[i++];
    CS->x = dArea[i++];
    CS->y = dArea[i++];
    CS->z = dArea[i++];

    CS->CDList = NULL;

    Clusters[k] = &CSData[k];

    //advance iArea for next iteration, reset i
    iArea = (int *) &(dArea[i]);
    i = 0;
  }

  //resort Clusters
  CSSort (Clusters, nClusters);

  delete [] dataStream;
}

/*---------------------------------------------------------------------------

There are two addressing schemes used by the program.  In the logical scheme 
which is used in the input file, each cell is addressed by its Column, Layer, 
and Cell (CLC).  In the running program, Columns and Layers become largely 
irrelevant (they specify organization, but have no physical component that needs 
to be simulated), and addressing is by Node (Machine), Cell, and Compartment 
within the Cell (currently always 0).  The purpose of this module is to provide 
a sort of translation layer between the CLC specification of the input file, and 
the internal workings of the program, which passes messages between 
(compartments of) cells which may be on different machines.

The code is based on two structures.  The Global Cell List (GCList) is an array 
of pointers to GCLIST structs.  (The actual structs are in the GCData array, 
which should be private.) GCList is created by CellManager, and has an entry for 
each cell in the entire brain.  The DistributionManager assigns values to the 
Node member of the struct, and each node will later scan this list to create 
Cell objects for the cells on the node.  Note two points here: 1) This list must 
be identical across all nodes; 2) The index of a cell in the list serves as its 
global CellID.

When the DistributionManager distributes the cells among nodes, they are 
assigned a new Local CellID, which is stored in the Cell member of the GCLIST 
struct, and used in all later addressing.  This CellID is also the Cell's index 
in the brain's cell array.

The second struct, Clusters is a list of cell clusters, which are groups of 
identical cells, as specified by the CELL_TYPE keyword in the LAYER element. 
Each cluster should be expressed as a continuous range of cells in the 
GCData/GCList arrays.  The Clusters entry contains the index of the start of the 
range, and the number of cells, as well as indexes back to the definitions of 
the Column, Layer, and Cell.  Each cell in the GCList has a pointer to the 
Clusters item that defines it.

Clusters is used by ConnectionManager to create the connections between cells.  It 
is used for lookups, and must be sorted by Column, Layer, and Cell indexes.

Note: GCList might work better if it were implemented as a sparse 3D array...

---------------------------------------------------------------------------*/

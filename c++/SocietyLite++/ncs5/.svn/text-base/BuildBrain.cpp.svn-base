/* This creates the actual brain.  Input has been parsed and stored in AR,   */
/* the GCList has been created, and the cells assigned to their nodes.  Each */
/* node only creates Cell objects (and their components) for the cells that  */
/* are on this node.                                                         */

/* Note that a cell and its components are built in a top-down fashion.  One */
/* reason for this is that they should thus tend to occupy contiguous        */
/* memory, and so be a little faster because of cache effects.  Also there   */
/* may be elements - columns, cells, etc - that are defined in the input,    */
/* but not used, so creating them would waste resources.                     */

#include "defines.h"
#include <math.h>

#include "BuildBrain.h"
#include "Managers.h"
#include "SendList.h"
#include "SynapseDef.h"
#include "Random.h"

#include "debug.h"
#include "memstat.h"

#define MEM_KEY KEY_BUILDBRAIN

static void MakeCells (ARRAYS *, RandomManager *);
static void MakeSynapseDefs (ARRAYS *);
void assignCellPositions( int offset, ARRAYS *AR, CLUSTER *cluster, Random &RN );
void assignCompartmentDelays( int offset, ARRAYS *AR, CLUSTER *Cluster );
void outputCellPositions( int offset, CLUSTER *cluster, FILE *fout );

/*------------------------------------------------------------------------*/

void BuildBrain (ARRAYS *AR, RandomManager *RM)
{
  MakeSynapseDefs (AR);
  MakeCells (AR, RM);
}       


/*---------------------------------------------------------------------------*/
/* For each Cell in the GCList that is on this machine, create a Cell object */
/* from the corresponding Column, Layer & Cell definitions  in the input     */
/* structure.                                                                */

static void MakeCells (ARRAYS *AR, RandomManager *RM)
{
  CLUSTER *Cluster;
  T_CELL *iCell;
  T_CMP  *iCmp;
  Cell *Cel;
  Compartment *Cmp;
  int i, j, k, cellID, kdx, ncells;
  Random RN;
    RN.SeedIt( AR->Brain->Seed );  //for assigning cell positions
  FILE *fout;

  ncells = CellMgr->nGlobalCells;
//printf ("BuildBrain::MakeCells: this node has %d of %d Cells, first = %d\n",
//        CellMgr->nLocalCells, CellMgr->nGlobalCells, NI->firstCell [NI->Node]);

/* Allocate the Cells.  This is an array of pointers to all the Cell objects */
/* on this node.                                                             */
  TheBrain->nCells = CellMgr->nLocalCells;
  TheBrain->Cells = (Cell **) calloc (TheBrain->nCells, sizeof (Cell *));
  
  //if we are going to use decayingConnections later in Connector.cpp
  // -not just decaying Connections, but if distance matters for synaptic delay as well.
  if( NI->AR->Brain->flag & USE_DISTANCE_FLAG )
  {
    TheBrain->CellPosition[0] = (double *) calloc( sizeof( double ), TheBrain->nCells*3 );
    TheBrain->CellPosition[1] = &TheBrain->CellPosition[0][TheBrain->nCells];
    TheBrain->CellPosition[2] = TheBrain->CellPosition[1] + TheBrain->nCells;
    
    if( NI->Node == 0 && NI->AR->Brain->flag & OUTPUT_CELLS_FLAG )
    {
        char fname[128];
        sprintf( fname, "%s.cells.dat", NI->job );
        fout = fopen( fname, "w" );
        if( !fout )
          fprintf( stderr, "Error: could not open %s for cell position output\n", fname );
    }
  }

/* Now loop through the GCList, and for those clusters distributed to this   */
/* node, create Cell objects and their compartments, synapses, etc.          */

  cellID = 0;
  for (i = 0; i < CellMgr->nClusters; i++)
  {
    Cluster = CellMgr->Clusters [i];
    if (Cluster->Node == NI->Node)
    {
      //assign cell positions (for synapse delay in Connector.cpp)
      if( NI->AR->Brain->flag & USE_DISTANCE_FLAG )
      {
        assignCellPositions( cellID, AR, Cluster, RN );
        if( NI->AR->Brain->flag & OUTPUT_CELLS_FLAG )  //need to be assigning positions in order to output them
          outputCellPositions( cellID, Cluster, fout );
      }
     
      //create cells
      iCell = AR->Cells [Cluster->iCel];           /* Get corresponding cell definition */
      for (j = 0; j < Cluster->Qty; j++)
      {
        Cel = new Cell (cellID, Cluster->iCol, Cluster->iLay, Cluster->iCel);
        TheBrain->Cells [cellID] = Cel;

        /* Create the compartment object(s) owned by this cell */
        //iCell = AR->Cells [Cel->idx];           /* Get corresponding cell definition */ //do this once before loop

        Cel->nCompartments = iCell->nCmp;
        Cel->Compartments  = (Compartment **) calloc (Cel->nCompartments, sizeof (Compartment *));

        for (k = 0; k < Cel->nCompartments; k++)        /* create the compartments from definition */
        {
          kdx  = iCell->Cmp [k];
          iCmp = AR->Cmp [kdx];           /* Compartment definition */

          Cmp = new Compartment (iCmp, cellID, k, kdx, RM, AR);
          Cel->Compartments [k] = Cmp;
        }
        cellID++;
      }
      
      //handle any compartmental delays for the cluster
      assignCompartmentDelays( cellID - (Cluster->Qty), AR, Cluster );      
    }
    else if( NI->Node == 0 && NI->AR->Brain->flag & OUTPUT_CELLS_FLAG )
      //will need to receive the final coordinates to output them
      outputCellPositions( -1, Cluster, fout );
    
  } 
  
  if( NI->AR->Brain->flag & OUTPUT_CELLS_FLAG && NI->Node == 0 )
    fclose( fout );
}

/*---------------------------------------------------------------------------*/
/* This creates the synapse definitions, which are structs (or objects?)     */
/* containing the information needed to define a synapse of a particular     */
/* type.  In order to save space, there is one copy of this info for all     */
/* synapses of a type.  The Synapse objects all point to the one struct.     */

static void MakeSynapseDefs (ARRAYS *AR)
{
  T_SYNLEARN *iSL;
  double v, dPre, dPost;
  int i, j, nPre, nPost;

//FILE *out;
//char slname [256];


/* First go through the SYN_LEARN definitions, and calculate the learning    */
/* tables.  There is only one copy of the tables for any definition, so      */
/* Gaussian deviation should not be applied.  (Remove it from parser &       */
/* input.)  The tables are specified as two straight line segments, meeting  */
/* at the peak value.  (Wouldn't tables like the PSGs work better?)          */

  for (i = 0; i < AR->nSynLearn; i++)
  {
    iSL = AR->SynLearn [i];
    TheBrain->SynapseDefs = (SynapseDef **) calloc (AR->nSynapse, sizeof (SynapseDef*));

    iSL->nNegLearn = (int) (NI->FSV * iSL->DownWind [0]);
    iSL->NegLearnTable = (double *) calloc (iSL->nNegLearn, sizeof (double));
  
    nPre  = (int) (NI->FSV * iSL->Neg_Heb_Peak_Time [0]);
    nPost = iSL->nNegLearn - nPre;

    dPre  = (nPre  == 0) ? 0.0 : iSL->Neg_Heb_Peak_Delta_Use [0] / (double) nPre;
    dPost = (nPost == 0) ? 0.0 : iSL->Neg_Heb_Peak_Delta_Use [0] / (double) nPost;

    v = 0.0;
    for (j = 0; j < nPre; j++)
    {
      iSL->NegLearnTable [j] = v;
      v -= dPre;
    }
  
    for ( ; j < iSL->nNegLearn; j++)
    {
      iSL->NegLearnTable [j] = v;
      v += dPost;
    }
  
/* Repeat process for positive learning */

    iSL->nPosLearn = (int) (NI->FSV * iSL->UpWind [0]);
    iSL->PosLearnTable = (double *) calloc (iSL->nPosLearn, sizeof (double));
  
    nPre  = (int) (NI->FSV * iSL->Pos_Heb_Peak_Time [0]);
    nPost = iSL->nPosLearn - nPre;

    dPre  = (nPre  == 0) ? 0.0 : iSL->Pos_Heb_Peak_Delta_Use [0] / (double) nPre;
    dPost = (nPost == 0) ? 0.0 : iSL->Pos_Heb_Peak_Delta_Use [0] / (double) nPost;

    v = 0.0;
    for (j = 0; j < nPre; j++)
    {
      iSL->PosLearnTable [j] = v;
      v += dPre;
    }
  
    for ( ; j < iSL->nPosLearn; j++)
    {
      iSL->PosLearnTable [j] = v;
      v -= dPost;
    }

// debug

//  if (NI->Node == 0)
//  {
//    sprintf (slname, "SL.%d", i);
//    out = fopen (slname, "w");
//
//    for (j = 0; j < iSL->nNegLearn; j++)
//      fprintf (out, "%3d %f\n", j, iSL->NegLearnTable [j]);
//
//    for (j = 0; j < iSL->nPosLearn; j++)
//      fprintf (out, "%3d %f\n", j, iSL->PosLearnTable [j]);
//
//    fclose (out);
//  }
  }

/* Now the synapse definitions can be created.  This is somewhat less than   */
/* optimal, as it will probably create some duplicate definitions.           */

  TheBrain->nSynapseDef = AR->nSynapse;
  TheBrain->SynapseDefs = (SynapseDef **) calloc (AR->nSynapse, 
                                                  sizeof (SynapseDef *));
  for (i = 0; i < AR->nSynapse; i++)
  {
    TheBrain->SynapseDefs [i] = new SynapseDef (AR->Synapse [i], AR);
  }
}

/*---------------------------------------------------------------------------*/

/**
 * This function is called if the simulator needs cell positions for synaptic
 * delay.  This function should have access to TheBrain, so it does not need
 * to be passed as a parameter
 * @param offset The offset into the cell position array where this cluster will write its cell positions
 * @param AR Pointer to parsed data objects (although, I could probably access this through the NI pointer)
 * @param Cluster the current cell cluster containing cell quantity and indices to find column and layer definitions
 */
void assignCellPositions( int offset, ARRAYS *AR, CLUSTER *Cluster, Random &RN )
{
    double zmin, zmax;
    double radiusMax, radius;
    double angle;

    //cluster has x,y,z position - but doesn't retain height,width,layer min/max
    radiusMax = AR->Csh[ AR->Columns[Cluster->iCol]->CShell ]->width/2.0;
    zmin = AR->Csh[ AR->Columns[Cluster->iCol]->CShell ]->height * 0.01 * AR->Lsh[ AR->Layers[Cluster->iLay]->LShell ]->Lower;
    zmax = AR->Csh[ AR->Columns[Cluster->iCol]->CShell ]->height * 0.01 * AR->Lsh[ AR->Layers[Cluster->iLay]->LShell ]->Upper;
    
    for( int i=0; i<Cluster->Qty; i++ )
    {
        radius = radiusMax * RN.Rand();
        angle = 2.0 * M_PI * RN.Rand();
        TheBrain->CellPosition[0][i+offset] = Cluster->x + radius*cos( angle );
        TheBrain->CellPosition[1][i+offset] = Cluster->y + radius*sin( angle );
        TheBrain->CellPosition[2][i+offset] = zmin + RN.Rand()*(zmax-zmin);
    }
}

/*---------------------------------------------------------------------------*/

/**
 * This function will assign delays between compartments in cells with multiple compartments.
 * Since each cell has the same layout, maxDelay and other values can be calculated once, then
 * distributed to each cell in the cluster
 * @param offset The offset into TheBrain's cell list where this cluster begins
 * @param AR Pointer to parsed data objects
 * @param Cluster the current cell cluster contining cell quantity and index to find cell definition
 */
void assignCompartmentDelays( int offset, ARRAYS *AR, CLUSTER *Cluster )
{
    T_CELL *iCell = AR->Cells [Cluster->iCel];           /* Get corresponding cell definition */
    Cell *Cel = NULL;
    double gDelay;
    int n, k, maxDelay = 0;
    
    if ( !(iCell->nConnect > 0) )
        return;
        
    gDelay = 0.0;                           //determine maximum delay first
    for( n=0; n<iCell->nCmp; n++ )      //go through each compartment in this cell
    {
        for( k=0; k<iCell->nforward[n]; k++ ) //look at the compartment's forward connections
        {
            if( gDelay < iCell->forwardConn[n][k]->delay )
                gDelay = iCell->forwardConn[n][k]->delay;
        }
        for( k=0; k<iCell->nreverse[n]; k++ ) //look at the compartment's reverse connections
        {
            if( gDelay < iCell->reverseConn[n][k]->delay )
                gDelay = iCell->reverseConn[n][k]->delay;
        }
    }

    //have maximum delay, convert to timeticks and init cmp state buffer
    maxDelay = (int) (gDelay * NI->FSV);
    if( maxDelay < 0 )
        maxDelay = 1;

    for( n=0; n<Cluster->Qty; n++ ) //for each cell in the cluster
    {
        Cel = TheBrain->Cells[offset+n];
        for( k=0; k<Cel->nCompartments; k++ )    //for each compartment, init its state buffer
        {
            //all compartments will use the maximum delay in setting buffer size
            // (truncate the delay for now; future: round up? )
            //Cel->Compartments[k]->InitBuffer( (int)(gDelay * NI->FSV) );
            Cel->Compartments[k]->InitBuffer( maxDelay );

            Cel->Compartments[k]->SetForwardConnections( iCell->nforward[k], iCell->forwardConn[k] );
            Cel->Compartments[k]->SetReverseConnections( iCell->nreverse[k], iCell->reverseConn[k] );
        }
    }
}

/**
 * print cell coordinates to a file *.cells.dat where * is the job name
 */
void outputCellPositions( int offset, CLUSTER *cluster, FILE *fout )
{
    double *tempPosition = NULL;
    MPI_Status status;
    
    if( NI->Node == 0 && offset == -1 )  //get coordinates from off node
    {
        //MPI_Recv( &tempCells, 1, MPI_INT, cluster->Node, 0, MPI_COMM_WORLD, &status );
        tempPosition = new double[3*cluster->Qty];
        MPI_Recv( &tempPosition[0], cluster->Qty, MPI_DOUBLE, cluster->Node, 0, MPI_COMM_WORLD, &status );
        MPI_Recv( &tempPosition[cluster->Qty], cluster->Qty, MPI_DOUBLE, cluster->Node, 0, MPI_COMM_WORLD, &status );
        MPI_Recv( &tempPosition[cluster->Qty*2], cluster->Qty, MPI_DOUBLE, cluster->Node, 0, MPI_COMM_WORLD, &status );
        
        //had to do communication first to prevent deadlock; now check for file ptr
        if( !fout )
          return;
          
        //output cluster info, then coordinates
        //column layer cell names
        fprintf( fout, "%s %s %s\n", NI->AR->Columns[ cluster->iCol ]->L.name,
                                   NI->AR->Layers[ cluster->iLay ]->L.name,
                                   NI->AR->Cells[ cluster->iCel ]->L.name );
        //qty
        fprintf( fout, "%d\n", cluster->Qty );
        //coordinates
        for( int i=0; i<cluster->Qty; i++ )
        {
            fprintf( fout, "%lf %lf %lf\n", tempPosition[ i ],
                                            tempPosition[ cluster->Qty+i ],
                                            tempPosition[ cluster->Qty*2+i ] );
        }
        fprintf( fout, "\n" );
        
        delete [] tempPosition;
    }
    else if( NI->Node == 0 ) //output normal
    {
        //only Node 0 will have fout open
        if( !fout )
            return;
            
        //output cluster info, then coordinates
        //column layer cell names
        fprintf( fout, "%s %s %s\n", NI->AR->Columns[ cluster->iCol ]->L.name,
                                   NI->AR->Layers[ cluster->iLay ]->L.name,
                                   NI->AR->Cells[ cluster->iCel ]->L.name );
        //qty
        fprintf( fout, "%d\n", cluster->Qty );
        //coordinates
        for( int i=0; i<cluster->Qty; i++ )
        {
            fprintf( fout, "%lf %lf %lf\n", TheBrain->CellPosition[0][ offset+i ],
                                            TheBrain->CellPosition[1][ offset+i ],
                                            TheBrain->CellPosition[2][ offset+i ] );
        }
        fprintf( fout, "\n" );
    }
    else //send coordinates to node 0
    {
        MPI_Send( &TheBrain->CellPosition[0][offset], cluster->Qty, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD );
        MPI_Send( &TheBrain->CellPosition[1][offset], cluster->Qty, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD );
        MPI_Send( &TheBrain->CellPosition[2][offset], cluster->Qty, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD );
    }
}

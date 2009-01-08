/* Makes the connections between cells.  See comments at end of file.        */

#include "Connector.h"

#include "debug.h"
#include "memstat.h"

#define MEM_KEY KEY_CONNECT

#define XMIT_SIZE 4096
#define FILELEN (HOSTLEN + 8)

void PrintClusters (ARRAYS *, FILE *);
void PrintMap (int, int, char *);

static Random *RN;
static TMPCELL *tCells;

#include <vector>
#include <iostream>

using namespace std;

/*---------------------------------------------------------------------------*/

void Connector (ARRAYS *AR, RandomManager *RM)
{
  FILE *RptFile;

  MPI_Comm ConComm;       /* Separate MPI group & communicator for messages */
  MPI_Status status;
  MPI_Datatype MsgType, type [5];
  MPI_Aint base, addr, disp [5];

  CLUSTER *CFrom, *CTo;
  SynapseDef *iSyn;
  Synapse *sp;
  Cell *Cel;
  Compartment *Cmp;
  CONNECT *CD;
  CONNECTMSG *CMBuf, *XmitPtr, dummyCM;
  TMPCELL *tCel;
  TMPCMP  *tCmp;

  double t0;
  int i, j, n, num, maxBuf, nCD, *Order;
  int CelID, CmpID, SynIdx, nXmit, nSent, nRecv, nMade, *CelIDs, *CmpIDs;
  int blen [5], nFrom, nTo, del;
  char RptName [FILELEN], *Map;

  //JK: added these for distance calculations
  double *distanceMap;  //matrix of cell distances, later converted to delays
  int *haveSentPositions; //flags to indicate if cell position info has been exchanged
    haveSentPositions = new int[NI->nNodes];
  double **allPositions;  //temporary holder for cell positions that are off node
    allPositions = new double*[NI->nNodes];
  int *allCellCounts;     //temporary holder for cell counts that are off node
    allCellCounts = new int[NI->nNodes];
    
    //init arrays
    for( i=0; i<NI->nNodes; i++ ) 
        allPositions[i] = 0, haveSentPositions[i] = allCellCounts[i] = 0;
    //insert data for this node - don't delete this cell's positions later!
    allPositions[NI->Node] = TheBrain->CellPosition[0];
    allCellCounts[NI->Node] = TheBrain->nCells;
    
  //if outputting connect maps
  FILE *fout;
    if( NI->AR->Brain->flag & OUTPUT_CONNECT_MAP_FLAG && NI->Node == 0 )
    {
        char fname[128];
        sprintf( fname, "%s.synapse.dat", NI->job );
        fout = fopen( fname, "w" );
        if( !fout )
            fprintf( stderr, "Error: could not open %s for output of connect maps\n", fname );
    }

  t0 = MPI_Wtime ();

  RN = RM->Connect;
  nCD = CellMgr->nCD;

  printf ("Connector::Connector: %d connects\n", nCD);

/* Map is a 2D array of all possible connections between the two clusters in */
/* a connect.  It ensures that no duplicate connections will be made.  For   */
/* high connection probability, a full map is created, and pairs deleted     */
/* until the required number is reached; for low probability connects, pairs */
/* are added to an initially empty map.                                      */

  Map = (char *) calloc (CellMgr->maxQty * CellMgr->maxQty, sizeof (char));
  
  if( NI->AR->Brain->flag & USE_DISTANCE_FLAG )  //the user wants synaptic delays augmented by distance
  {
    distanceMap = new double[ CellMgr->maxQty * CellMgr->maxQty ];
  }

/* Order contains indexes of all the connect descriptors, in random order.   */
/* This is to speed up the transfer: generally the connects are in node      */
/* order, so the channel between successive pairs overloads while the rest   */
/* are idle.                                                                 */

  Order = (int *) calloc (nCD, sizeof (int));
  for (n = 0; n < nCD; n++)
    Order [n] = n;

  RN->SeedIt (CellMgr->CDList [0].Seed);
  for (n = 0; n < nCD; n++)
  {
    if (Order [n] == n)
    {
      i = (int) ((double) nCD * RN->Rand ());
      num = Order [n];
      Order [n] = Order [i];
      Order [i] = num;
    }
  }

/* tCells is temporary collecting area for connect stuff */

  tCells = (TMPCELL *) calloc (TheBrain->nCells, sizeof (TMPCELL));
  
  for (i = 0; i < TheBrain->nCells; i++)
  {
    Cel = TheBrain->Cells [i];
    tCells [i].Cmp = (TMPCMP *) calloc (Cel->nCompartments, sizeof (TMPCMP));
  }

/* CMBuf is space to store the information (FromCell, fromCmp, Addr) that is */
/* transmitted for each connection.  Parallel arrays CellIDs and CmpIDs hold */
/* the ID on this side.  As each connect descriptor is processed, the        */
/* connect info is put in CMBuf, and the space is used to transfer the data. */

  nXmit = XMIT_SIZE / sizeof (CONNECTMSG);

  maxBuf = CellMgr->maxCon;
  if (maxBuf < nXmit) maxBuf = nXmit;
  CMBuf  = (CONNECTMSG *) calloc (maxBuf, sizeof (CONNECTMSG));
  CelIDs = (int *) calloc (maxBuf, sizeof (int));
  CmpIDs = (int *) calloc (maxBuf, sizeof (int));

//if (NI->Node == 0)
//{
//  printf ("XMIT_SIZE = %d, sizeof (CONNECTMSG) = %d. nXmit = %d, CMBuf size = %d\n",
//          XMIT_SIZE, sizeof (CONNECTMSG), nXmit, maxBuf);
//}


/* Create MPI datatype that corresponds to CONNECTMSG */
  MPI_Comm_dup (MPI_COMM_WORLD, &ConComm);

  type [0] = type [1] = type [2] = type [3] = MPI_INT;
  blen [0] = blen [1] = blen [2] = blen [3] = 1;
  MPI_Address (&(dummyCM.FromCell), &base);
  disp [0] = 0;
  MPI_Address (&(dummyCM.FromCmp),  &addr);
  disp [1] = addr - base;
  MPI_Address (&(dummyCM.Addr),     &addr);
  disp [2] = addr - base;
  MPI_Address (&(dummyCM.Delay),    &addr);
  disp [3] = addr - base;

  MPI_Type_struct (4, blen, disp, type, &MsgType);
  MPI_Type_commit (&MsgType);

/* Open report file if needed                                                */

  if ((NI->Node == 0) && NI->ConnectRpt)
  {
    sprintf (RptName, "%s.connect", NI->job);
    RptFile = fopen (RptName, "w");
    fprintf (RptFile, "Connections defined\n");
    fprintf (RptFile, "Create  Number     From         To           nConnects\n");
  }

/* :1: First create the connections on the receiving side of the connect     */
/* descriptor, and send that information to the sending side.                */

  nFrom = nTo = 0;
  for (i = 0; i < nCD; i++)
  {
    n     = Order [i];
    CD    = &(CellMgr->CDList [n]);
    CFrom = CD->From;
    CTo   = CD->To;

    //fprintf (stderr, "%5d  %5d/%d  (%s %s %s) -> (%s %s %s) %8d\n", i, n, nCD,
    //            AR->Columns [CFrom->iCol]->L.name,
    //            AR->Layers  [CFrom->iLay]->L.name,
    //            AR->Cells   [CFrom->iCel]->L.name,
    //            AR->Columns [CTo->iCol]->L.name,
    //            AR->Layers  [CTo->iLay]->L.name,
    //            AR->Cells   [CTo->iCel]->L.name,
    //            CD->nCon);    

    //check to see if this connection was part of a recurrent connection
    if( CD->iCon->disabled )
    {
      continue;
    }

    if ((NI->Node == 0) && NI->ConnectRpt)
    {
      fprintf (RptFile, "%5d  %5d/%d  (%s %s %s) -> (%s %s %s) %8d\n", i, n, nCD,
              AR->Columns [CFrom->iCol]->L.name,
              AR->Layers  [CFrom->iLay]->L.name,
              AR->Cells   [CFrom->iCel]->L.name,
              AR->Columns [CTo->iCol]->L.name,
              AR->Layers  [CTo->iLay]->L.name,
              AR->Cells   [CTo->iCel]->L.name,
              CD->nCon);
    }

/* If the receiving cluster of this connect is on this node, proceed to      */
/* create the connections                                                    */
    if (CTo->Node == NI->Node)
    {
      //if distance matters for connections, receive from "From Node"
      if( NI->AR->Brain->flag & USE_DISTANCE_FLAG )
      {
        //do I need to get data from off Node?
        if( CFrom->Node != NI->Node && allPositions[CFrom->Node] == NULL )
        {
            //get number of cells
            MPI_Recv( &(allCellCounts[CFrom->Node]), 1, MPI_INT, CFrom->Node, 0, ConComm, &status);
            //allocate memory, then read in the data
            allPositions[CFrom->Node] = new double[ allCellCounts[CFrom->Node]*3 ];
            MPI_Recv( allPositions[CFrom->Node], allCellCounts[CFrom->Node]*3, MPI_DOUBLE, CFrom->Node, 0, ConComm, &status);
        }

        MakeConnects (CD, Map, &nMade, CMBuf, CelIDs, CmpIDs, allPositions[CFrom->Node],
                      allCellCounts[CFrom->Node], allPositions[NI->Node], allCellCounts[NI->Node], distanceMap);
      }
      else
        MakeConnects (CD, Map, &nMade, CMBuf, CelIDs, CmpIDs);

      if( CD->iCon->recurrentProbability[0] > 0 || CD->iCon->recurrentProbability[1] > 0 )
        MakeRecurrentConnects( CD, Map, nMade, CMBuf, CelIDs, CmpIDs, allPositions[CFrom->Node],
                               allCellCounts[CFrom->Node], allPositions[NI->Node], allCellCounts[NI->Node],
                               distanceMap, tCel, ConComm, status, MsgType, fout );

      //output map
      if( NI->AR->Brain->flag & OUTPUT_CONNECT_MAP_FLAG )
        outputConnectMap( CD, Map, fout );

//    printf ("%5d/%d  (%s %s %s) -> (%s %s %s) (%d x %d, prob = %8.5f, nCon = %8d\n", n, nCD,
//            AR->Columns [CFrom->iCol]->L.name,
//            AR->Layers  [CFrom->iLay]->L.name,
//            AR->Cells   [CFrom->iCel]->L.name,
//            AR->Columns [CTo->iCol]->L.name,
//            AR->Layers  [CTo->iLay]->L.name,
//            AR->Cells   [CTo->iCel]->L.name,
//            CFrom->Qty, CTo->Qty, CD->iCon->Prob, CD->nCon);
//    PrintMap (CFrom->Qty, CTo->Qty, Map);

      SynIdx = CD->iCon->SynType;
      iSyn = TheBrain->SynapseDefs [SynIdx];
      //create synapses
      for (j = 0; j < nMade; j++)
      {
        CelID = CelIDs [j];
        CmpID = CmpIDs [j];
  
        Cmp  = TheBrain->Cells [CelID]->Compartments [CmpID];
        tCmp = &(tCells [CelID].Cmp [CmpID]);
  
//      printf ("Connect: From %d to %d, Syn %s\n",
//              CMBuf [i].FromCell, CelID, iSyn->source->L.name);
        sp = new Synapse (iSyn, Cmp, RN);

        if (sp == NULL)
        {
          printf ("Memory allocation error in Connector, mem = %f.3 Mbytes\n", 
                  (double) GetMemoryUsed () / 1024.0);
          fprintf (stderr, "Memory allocation error in Connector, mem = %f.3 Mbytes\n",
                  (double) GetMemoryUsed () / 1024.0);
        }

        tCmp->Syn.push_back (sp);
        del = (int) (NI->FSV * RN->RandRange (iSyn->SynDelay));

        if( NI->AR->Brain->flag & USE_DISTANCE_FLAG ) //distance matters - augment delay
            del += (int) distanceMap[ (CMBuf[j].FromCell - CD->From->idx) + (CelID - CD->To->idx)*CD->From->Qty ];
        
        if (del < 1) del = 1;
        if (del > NI->MaxSynDelay)
          NI->MaxSynDelay = del;

        CMBuf [j].Addr  = sp;
        CMBuf [j].Delay = del;
      }
      //all synapses are declared for this particular connection

      nTo += nMade;
      NI->Recv [CFrom->Node] += nMade;
      
/* If From & To are both this node, just copy in memory, otherwise transfer  */
/* info via MPI.  The transfer is broken into blocks because of MPI errors   */
/* on transmitting large amounts of data in a single send.                   */

      if (CFrom->Node == NI->Node)
      {
        AddToSendTo (nMade, CFrom->Node, SynIdx, CMBuf);
        nFrom += nMade;
        NI->Send [CTo->Node] += nMade;
      }
      else
      {
        if( CD->iCon->step > 0 ) //decaying probability, must communicate number of connections
            MPI_Send( &(CD->nCon), 1, MPI_INT, CFrom->Node, 0, ConComm );
      
        nSent = 0;

        while (nSent < nMade)
        {
          XmitPtr = CMBuf + nSent;

          num = nXmit;
          if ((nSent + nXmit) > nMade)
            num = nMade - nSent;
            
          MPI_Send (XmitPtr, num, MsgType, CFrom->Node, 0, ConComm);

          nSent += num;
        }
      }
    }
/* If the To cluster wasn't on this node, then listen for the connection     */
/* info to arrive.  CD->nCon contains the number of items that are expected  */

    else if (CFrom->Node == NI->Node)
    {
        //if distance matters, I need to send coordinate info to the receiving node
        if( NI->AR->Brain->flag & USE_DISTANCE_FLAG )
        {
            if( !haveSentPositions[ CTo->Node ] )  //coordinates only need to be sent once for all connections with this Node
            {
                //send number of Global Cells (not just number in cluster), followed by the coordinate array
                //  I should probably break this up into smaller data chunks
                MPI_Send( &(TheBrain->nCells), 1, MPI_INT, CTo->Node, 0, ConComm );
                MPI_Send( TheBrain->CellPosition[0], TheBrain->nCells*3, MPI_DOUBLE, CTo->Node, 0, ConComm );
                haveSentPositions[ CTo->Node ] = 1; //set flag so I remember I already have the coordinates for this node
            }
        }

        //if recurrent connections are being made, I should check now, before I receive the regular connection info
        if( CD->iCon->recurrentProbability[0] > 0 || CD->iCon->recurrentProbability[1] > 0 )
            acceptRecurrentConnection( CD, Map, distanceMap, tCel, ConComm, status, MsgType, fout );

        //need to accept connection map if this is node 0
        if( NI->AR->Brain->flag & OUTPUT_CONNECT_MAP_FLAG && NI->Node == 0 )
            outputConnectMap( CD, Map, fout );

        nRecv = 0;
        // if using decaying Probability, the number of connections made may be different.
        if( CD->iCon->step > 0 )
            MPI_Recv( &(CD->nCon), 1, MPI_INT, CTo->Node, 0, ConComm, &status );
        while (nRecv < CD->nCon)
        {
            MPI_Recv (CMBuf, nXmit, MsgType, CTo->Node, 0, ConComm, &status);
            MPI_Get_count (&status, MsgType, &num);

            AddToSendTo (num, status.MPI_SOURCE, SynIdx, CMBuf);
            nRecv += num;
            NI->Send [CTo->Node] += num;
        }
        nFrom += nRecv;
    }

    //outputting recurrent connections: send to node 0 if it was not NI->To->Node
    if( (CD->iCon->recurrentProbability[0] > 0 || CD->iCon->recurrentProbability[1] > 0) &&
        NI->AR->Brain->flag & OUTPUT_CONNECT_MAP_FLAG &&
        (NI->Node == 0 && NI->Node != CTo->Node && NI->Node != CFrom->Node) && (CTo != CFrom) )
        outputConnectMap( CD, Map, fout, 1 );

    //The Node with the map should send it to Node 0 if outputing
    if( NI->AR->Brain->flag & OUTPUT_CONNECT_MAP_FLAG && ( NI->Node == 0 && NI->Node != CTo->Node && NI->Node != CFrom->Node ) )
        outputConnectMap( CD, Map, fout );
  }

  //delete memory
  free (Map);
  if( NI->AR->Brain->flag & USE_DISTANCE_FLAG )
    delete [] distanceMap;  //distanceMap is only allocated if flag is set
  delete [] haveSentPositions;  //everything else gets allocated regardless of the flag
  for( i=0; i<NI->nNodes; i++ )
    if( NI->Node != i && allCellCounts[i] ) //don't delete this Node's cell positions!
        delete [] allPositions[i];
  delete [] allPositions;
  delete [] allCellCounts;

  if( NI->AR->Brain->flag & OUTPUT_CONNECT_MAP_FLAG && NI->Node == 0 )
    fclose( fout );

  printf ("Connector: step 1 done!, time = %f, memory = %.2f MBytes\n",
          MPI_Wtime () - t0, (double) GetMemoryUsed () / 1024.0);

/* :2: Now that the SendTos and Synapses have been counted for all the       */
/* cells, allocate the space needed for them.                                */

  for (i = 0; i < TheBrain->nCells; i++)
  {
    Cel  = TheBrain->Cells [i];
    tCel = &(tCells [i]);
    for (j = 0; j < Cel->nCompartments; j++)
    {
      Cmp  = Cel->Compartments [j];
      tCmp = &(tCel->Cmp [j]);

      Cmp->nSendTo = tCmp->Addr.size ();
      if (Cmp->nSendTo > 0)
      {
        Cmp->SendTo = (SENDITEM *) calloc (Cmp->nSendTo, sizeof (SENDITEM));
        for (n = 0; n < Cmp->nSendTo; n++)
        {
          Cmp->SendTo [n].Node  = tCmp->Node [n];
          Cmp->SendTo [n].Addr  = tCmp->Addr [n];
          Cmp->SendTo [n].Delay = tCmp->Delay [n];
        }
        tCmp->Node.clear ();
        tCmp->Addr.clear ();
        tCmp->Delay.clear ();
      }
      Cmp->nSynapse = tCmp->Syn.size ();
      if (Cmp->nSynapse > 0)
      {
        Cmp->iOtherSynapse = Cmp->nSynapse - 1;
        Cmp->SynapseList = (Synapse **) calloc (Cmp->nSynapse, sizeof (Synapse *));
        for (n = 0; n < Cmp->nSynapse; n++)
        {
          sp = tCmp->Syn [n];
          if (sp->SynDef->LearnPos)
          {
            Cmp->SynapseList [Cmp->nPosLearnSynapse++] = sp;
          }
          else
          {
            Cmp->SynapseList [Cmp->iOtherSynapse--] = sp;
          }
        }
      }
    }
    free( tCel->Cmp );
  }
  
  free (tCells);
  free (CMBuf);
  free (Order);

  MPI_Comm_free (&ConComm);
  
  printf ("Connector::Connector: connection done, time = %f, memory = %.2f MBytes\n",
          MPI_Wtime () - t0, (double) GetMemoryUsed () / 1024.0);


  for (i = 0; i < NI->nNodes; i++)
  {
    if ((i != NI->Node) && (NI->Send [i] > 0)) NI->nSend++;
    if ((i != NI->Node) && (NI->Recv [i] > 0)) NI->nRecv++;

    if (NI->check)
      printf ("Connector::ConnectMap: for node %3d, Send = %8d. Recv = %8d\n",
              i, NI->Send [i], NI->Recv [i]);
  }

  if ((NI->Node == 0) && NI->ConnectRpt)
  {
    PrintClusters (AR, RptFile);
    fclose (RptFile);
  }

//printf ("Connect: %d cells\n", TheBrain->nCells);
//for (i = 0; i < TheBrain->nCells; i++)
//{
//  Cmp = TheBrain->Cells [i]->Compartments [0];
//  printf ("Connect: Cell %3d=%3d, syn = %3d, %3d\n",
//         i, Cmp->CellID, Cmp->nSynapse, Cmp->nPosLearnSynapse);
//}

}

/*---------------------------------------------------------------------------*/

void sendRecurrentInfo( CONNECT *CD, char *map, double* distanceMap, MPI_Comm &ConComm, MPI_Status &status )
{
    int count = CD->To->Qty*CD->From->Qty;
    
    int bytesSent = 0;
    while( bytesSent < count )
    {
        if( count-bytesSent >= XMIT_SIZE )
            MPI_Send( &map[bytesSent], XMIT_SIZE, MPI_CHAR, CD->To->Node, 0, ConComm );
        else
            MPI_Send( &map[bytesSent], count-bytesSent, MPI_CHAR, CD->To->Node, 0, ConComm );
        bytesSent+=XMIT_SIZE;
    }
    
    if( NI->AR->Brain->flag & USE_DISTANCE_FLAG )
    {
        int blockSize = (int) XMIT_SIZE/sizeof(double);  //number of doubles we can send per transmission
        
        bytesSent=0;
        while( bytesSent < count )
        {
            if( count-bytesSent >= blockSize )
                MPI_Send( &distanceMap[bytesSent], blockSize, MPI_DOUBLE, CD->To->Node, 0, ConComm );
            else
                MPI_Send( &distanceMap[bytesSent], count-bytesSent, MPI_DOUBLE, CD->To->Node, 0, ConComm );
            bytesSent += blockSize;
        }
    }    
}

/*---------------------------------------------------------------------------*/

void acceptDestinationInfo( CONNECT* CD, int nMade, CONNECTMSG *CMBuf,
                            MPI_Comm &ConComm, MPI_Status &status, MPI_Datatype &MsgType )
{
    //welcome back data (need mem addresses of synapses)
    int nXmit = XMIT_SIZE / sizeof (CONNECTMSG);
    int num = 0;
    
    int nRecv = 0;
    if( CD->iCon->step > 0 )// decayingProbability: need to ask how many connections were made
        MPI_Recv( &(CD->nCon), 1, MPI_INT, CD->To->Node, 0, ConComm, &status );
            
    while (nRecv < CD->nCon)
    {
        MPI_Recv (CMBuf, nXmit, MsgType, CD->To->Node, 0, ConComm, &status);
        MPI_Get_count (&status, MsgType, &num);

        AddToSendTo (num, status.MPI_SOURCE, CD->iCon->SynType, CMBuf);
        nRecv += num;
        NI->Send [CD->To->Node] += num;
    }
    //nFrom += nRecv;
}

/*---------------------------------------------------------------------------*/

void shuffleConnections( int nRecurrent, vector<int> &target )
{
    int selectCell, temp;

    //build a random selection list
    for( int i=0; i<(2*nRecurrent) && i<target.size(); i++ ) //need twice as many
    {                                                  // half for potential, half to steal
        selectCell = (int) (RN->Rand()*target.size());         //shuffle
        temp = target[selectCell];
        target[selectCell] = target[i];
        target[i] = temp;
    }
}

/*---------------------------------------------------------------------------*/

void shuffleConnections( int nRecurrent, vector<int> &target, vector<int> &steal )
{
    int selectCell, temp;

    for( int i=0; i<nRecurrent && i<target.size(); i++ )
    {
        selectCell = (int) (RN->Rand()*target.size());    //shuffle potential connects
        temp = target[selectCell];
        target[selectCell] = target[i];
        target[i] = temp;
    }
    for( int i=0; i<nRecurrent && i<steal.size(); i++ )
    {
        selectCell = (int) (RN->Rand()*steal.size());      //shuffle potential disconnects
        temp = steal[selectCell];
        steal[selectCell] = steal[i];
        steal[i] = temp;
    }
}

/*---------------------------------------------------------------------------*/

void MakeSelfRecurrence( CONNECT *CD, char *Map, int nMade, CONNECTMSG *CMBuf,
                            int *CellIDs, int *CmpIDs, double *distanceMap )
{
    //track connections that occur from cell ij to cell ji or vice-versa, but not both
    vector<int> AtoB;
    int nRecurrent = (int) (CD->iCon->recurrentProbability[0]*nMade + EPSILON );
    int row=0, col=0;
    int selectCell = -1;

    for( int i=0; i<CD->To->Qty; i++ )
    {
        for( int j=i+1; j<CD->From->Qty; j++ )  //j will keep us in upper-right corner of matrix
        {
            if( Map[i*CD->From->Qty + j] && Map[ j*CD->From->Qty + i ] ) //already recurrent
                nRecurrent -= 2; //each existing pair means 2 less to insert
            else if ( Map[ i*CD->From->Qty+j ] )  //available location in upper quadrant
                AtoB.push_back( j*CD->From->Qty+i );  //push on mirror point
            else if ( Map[ j*CD->From->Qty+i ] )  //available location in lower quadrant
                AtoB.push_back( i*CD->From->Qty+j );  //push on mirror point
        }
    }

    nRecurrent = (int) nRecurrent / 2;  //special for self connect

    //build a random selection list
    shuffleConnections( nRecurrent, AtoB );

    //for remaining recurrent connections, randomly select a point in AtoB,
    //make it recurrent, select another point and delete it
    for( int i=0; i<nRecurrent*2 && i<AtoB.size(); i+=2 )
    {
        selectCell = AtoB[i];
        Map[selectCell] = 1;

        if( NI->AR->Brain->flag & USE_DISTANCE_FLAG )
        {
            row = (int) selectCell/CD->From->Qty;
            col = selectCell - row*CD->From->Qty;
            distanceMap[selectCell] = distanceMap[col*CD->From->Qty + row];
        }

        //select another point and remove it
        selectCell = AtoB[i+1];

        row = (int) selectCell/CD->From->Qty;
        col = selectCell - row*CD->From->Qty;
        Map[ col*CD->From->Qty + row ] = 0;
    }

    //Since there is only one map for a group recurrent with itself, this can now return

    gatherConnectedIDs( CD, Map, CMBuf, CellIDs, CmpIDs );
}

/*---------------------------------------------------------------------------*/

void CoordinateConnectionsOneWay( CONNECT *CD, char *MapAtoB, char *MapBtoA, int nMade,
                                  double *distanceMapAtoB, double *distanceMapBtoA, int dir )
{
    vector<int> AtoB, BtoA;
    int nMap = CD->From->Qty*CD->To->Qty;
    int nRecurrent = (int) (CD->iCon->recurrentProbability[dir]*nMade + EPSILON );
    int selectCell = -1;

    //collect all connections that are non-recurrent
    // i.e connections from group A to B, but not B to A and vice-versa
    for( int i=0; i<nMap; i++ )
    {
      if( MapAtoB[i] && !MapBtoA[i] )      //potential recurrence
          AtoB.push_back( i );
      else if( MapAtoB[i] && MapBtoA[i] )  //already recurrent
          nRecurrent--;
      else if( MapBtoA[i] && !MapAtoB[i] ) //connections to steal
          BtoA.push_back( i );
    }

    //prepare the collected info so that it is premtively random
    shuffleConnections( nRecurrent, AtoB, BtoA );

    //as long as we need to confirm more connections are recurrent
    //  (and we have non-recurrent connections to steal)
    for( int i=0; i<nRecurrent && i<BtoA.size() && i<AtoB.size(); i++ )
    {
        selectCell = AtoB[i];      //grab the index of next potential connection
        MapBtoA[selectCell] = 1;   //make it reciprocal on the other map

        if( NI->AR->Brain->flag & USE_DISTANCE_FLAG )
            distanceMapBtoA[selectCell] = distanceMapAtoB[selectCell];  //if using distance

        selectCell = BtoA[i];      //grab the index of a connection to eliminate
        MapBtoA[selectCell]=0;     //disconnect it
    }

    //we're done - clear
    AtoB.clear();
    BtoA.clear();
}

/*---------------------------------------------------------------------------*/

void MakeRecurrentConnects( CONNECT *CD, char *MapAtoB, int nMade, CONNECTMSG *CMBuf,
                            int *CellIDs, int *CmpIDs, double *src, int nSrcCells, 
                            double *dest, int nDestCells, double *distanceMapAtoB, TMPCELL *tCel,
                            MPI_Comm &ConComm, MPI_Status &status, MPI_Datatype &MsgType,
                            FILE *fout )
{  
    int nMadeBtoA, nMap = CD->From->Qty*CD->To->Qty;

    //First, reseed the random number generator
    CD->Seed = -RN->iRand();

    //If a cell group is becoming recurrent with itself, it uses a simplified process
    // and can return to normal processing sooner
    if( CD->From == CD->To )
    {
        MakeSelfRecurrence( CD, MapAtoB, nMade, CMBuf, CellIDs, CmpIDs, distanceMapAtoB );
        return;
    }

    //Otherwise, two independant cell groups are forming reciprocal connections

    //allocate memory
    char *MapBtoA = new char[CD->From->Qty*CD->To->Qty];
    CONNECTMSG *CMBufBtoA = new CONNECTMSG[ CellMgr->maxCon ];
    double *distanceMapBtoA = new double[CD->From->Qty*CD->To->Qty];
    int *CellIDsBtoA = new int[ CellMgr->maxCon ];
    int *CmpIDsBtoA = new int[ CellMgr->maxCon ];

    swapInfo( CD ); //switch connection info (probabilies, source, dest, etc. )

    //Since Connection BtoA was disabled, it has not updated info for the clusters
    CD->From->nSendTo += CD->nCon;
    CD->To->nSyn += CD->nCon;

    //Produce another connection map, using the new probability swapped in
    // from the other connection
    if( NI->AR->Brain->flag & USE_DISTANCE_FLAG )  //Distance will augment Synaptic delays
        MakeConnects( CD, MapBtoA, &nMadeBtoA, CMBufBtoA, CellIDsBtoA, CmpIDsBtoA,
                      dest, nDestCells, src, nSrcCells, distanceMapBtoA, 1 );
    else
        MakeConnects( CD, MapBtoA, &nMadeBtoA, CMBufBtoA, CellIDsBtoA, CmpIDsBtoA );

    //coordinate connections from group A to group B
    CoordinateConnectionsOneWay( CD, MapAtoB, MapBtoA, nMade,
                                 distanceMapAtoB, distanceMapBtoA, 0 );

    //now coordinate from group B to group A (swap parameters around)
    CoordinateConnectionsOneWay( CD, MapBtoA, MapAtoB, nMadeBtoA,
                                 distanceMapBtoA, distanceMapAtoB, 1 );

    //transpose the connect MapBtoA ( and distanceMapBtoA if applicable )
    transposeMap( nMap, CD->To->Qty, CD->From->Qty, MapBtoA, distanceMapBtoA );
    
    //Create Synapses just for connections BtoA (we'll do AtoB when the function returns)
    if( CD->To->Node == CD->From->Node )  //No need to communicate via MPI
        createSynapses( CD, MapBtoA, distanceMapBtoA, nMadeBtoA, tCel, ConComm, status, MsgType );
    else
    {
        //Transmit data. Other node will create synapses in function acceptRecurrentConnections
        sendRecurrentInfo( CD, MapBtoA, distanceMapBtoA, ConComm, status );
        //get the result
        acceptDestinationInfo( CD, nMade, CMBufBtoA, ConComm, status, MsgType );
    }

    swapInfo( CD ); //back to normal

    //output BtoA map if Node 0, Node A, Node B are all the same Node
        //Otherwise, only output if Node 0 != Node B
        //(Other conditions are handled elsewhere)
    if(  NI->AR->Brain->flag & OUTPUT_CONNECT_MAP_FLAG && 
         ( CD->From->Node != 0 || ( CD->From->Node == CD->To->Node ) ))
        outputConnectMap( CD, MapBtoA, fout, 1 );

    //redo CMBuf for AtoB since connections have most likely changed
    gatherConnectedIDs( CD, MapAtoB, CMBuf, CellIDs, CmpIDs );

    //free memory
    delete [] MapBtoA;
    delete [] CMBufBtoA;
    delete [] CellIDsBtoA;
    delete [] CmpIDsBtoA;
    delete [] distanceMapBtoA;
}

/*---------------------------------------------------------------------------*/

void transposeMap( int nMap, int nTo, int nFrom, char *map, double *distanceMap )
{
    char *tempMap;
    double *tempDistMap;
    int targetRow, targetCol, oIndex, tIndex;
    
    //Use memcpy to duplicate the current map.  Then I make changes
    //directly to the original map while referring to the copy
    tempMap = new char[ nMap ];
    memcpy( tempMap, map, nMap );
    
    oIndex = tIndex = 0;
    for( targetRow=0; targetRow<nTo; targetRow++ )
    {
        oIndex = targetRow;
        for( targetCol=0; targetCol<nFrom; targetCol++ )
        {
            map[tIndex+targetCol] = tempMap[oIndex];
            oIndex += nTo;
        }
        tIndex += nFrom;
    }
    delete [] tempMap;
    
    if( NI->AR->Brain->flag & USE_DISTANCE_FLAG )
    {
        tempDistMap = new double[ nMap ];
        memcpy( tempDistMap, distanceMap, sizeof(double)*nMap );
        
        oIndex = tIndex = 0;
        for( targetRow=0; targetRow<nTo; targetRow++ )
        {
            oIndex = targetRow;
            for( targetCol=0; targetCol<nFrom; targetCol++ )
            {
                distanceMap[tIndex+targetCol] = tempDistMap[oIndex];
                oIndex += nTo;
            }
            tIndex += nFrom;
        }
        delete [] tempDistMap;
    }
}

/*---------------------------------------------------------------------------*/

void swapInfo( CONNECT *CD )
{
  CLUSTER *temp=CD->From;
  CD->From = CD->To;
  CD->To = temp;

  T_CONNECT *iCon = CD->iCon;
  CD->iCon = iCon->recurrentConnection;
  CD->iCon->recurrentConnection = iCon;  //I should set up this loop in parse/ConvertIn.c
  CD->iCon->recurrentProbability[0] = iCon->recurrentProbability[0];
  CD->iCon->recurrentProbability[1] = iCon->recurrentProbability[1];

  CD->nCon = (int)(CD->iCon->Prob * ((double) CD->From->Qty*CD->To->Qty));
  
  int tCmp = CD->FromCmp;
  CD->FromCmp = CD->ToCmp;
  CD->ToCmp = tCmp;
}

/*---------------------------------------------------------------------------*/

void MakeConnects (CONNECT *CD, char *Map, int *nMade, CONNECTMSG *CMBuf, 
                   int *CelIDs, int *CmpIDs)
{
  CLUSTER *CFrom, *CTo;
  double fMap;
  int i, ndx, nMap, nFrom, nTo, sameGroup;
  char flag;

  CFrom = CD->From;
  CTo   = CD->To;

  sameGroup = 0;
  if( CFrom == CTo )  //we do not want to connect a cell to itself
    sameGroup = 1;

  RN->SeedIt (CD->Seed);

  nFrom = CFrom->Qty;
  nTo   = CTo->Qty;
  nMap  = nFrom * nTo;
  fMap  = (double) nMap;
  flag  = (CD->iCon->Prob > 0.5) ? 1 : 0;
  memset (Map, flag, nMap);

  if (CD->iCon->Prob > 0.5)           /* Proceed by eliminating from all-all */
  {
    *nMade = nMap;

    if( sameGroup )   //remove cell self-connections first
    {
      for( i=0; i<nFrom; i++ )
        Map [ nFrom*i+i ] = 0;
      (*nMade) -= nFrom;
    }

    while (*nMade > CD->nCon)
    {
      ndx = (int) (fMap * RN->Rand ());
      if (Map [ndx] != 0)
      {
        Map [ndx] = 0;
        (*nMade)--;
      }
    }
  }
  else
  {
    *nMade = 0;
    while (*nMade < CD->nCon)
    {
      ndx = (int) (fMap * RN->Rand ());

      if (Map [ndx] == 0 && !(sameGroup && ndx%(nFrom+1)==0) )  //don't connect to self
      {
        Map [ndx] = 1;
        (*nMade)++;
      }
    }
  }

  gatherConnectedIDs( CD, Map, CMBuf, CelIDs, CmpIDs );
}

/*---------------------------------------------------------------------------*/

void gatherConnectedIDs( CONNECT *CD, char *Map, CONNECTMSG *CMBuf, int *CelIDs, int *CmpIDs )
{
  int i, j, ndx, n = 0;
  int nTo = CD->To->Qty, nFrom = CD->From->Qty;
  
  for (j = ndx = 0; j < nTo; j++)
  {
    for (i = 0; i < nFrom; i++, ndx++)
    {
      if (Map [ndx])
      {
        CMBuf [n].FromCell = CD->From->idx + i;
        CMBuf [n].FromCmp  = CD->iCon->FromCmp; //should be compartment index? i.e. not automatically zero
        CelIDs [n] = CD->To->idx + j;
        CmpIDs [n] = CD->iCon->ToCmp;  //should be compartment index? i.e. not automatically zero
        n++;
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void MakeConnects (CONNECT *CD, char *Map, int *nMade, CONNECTMSG *CMBuf, 
                   int *CelIDs, int *CmpIDs, double *src, int nSrcCells, 
                   double *dest, int nDestCells, double *distanceMap, int transpose )
{
    //Use cluster info to find source and destination cell positions
    double *destination = &dest[ CD->To->idx ];
    double *source = &src[ CD->From->idx ];
    int activeCell = 0;

    int sameGroup = 0;
    
    //seed random number generator
    RN->SeedIt (CD->Seed);

    int nFrom = CD->From->Qty, nTo = CD->To->Qty, i, j;
    if( transpose )
    {
        //For reciprocal connections, I want my maps to use the same dimensions
        // e.g. I want to work with two 300x1000 maps, not a 300x1000 and a 1000x300
        // so if the transpose flag is set, I need to just switch the nFrom and nTo values.
        // Also, I assume that cell coordinates are sent correctly (i.e. I could have designed
        // this function to expect src in dest's place, and dest in src's place, but didn't)
        // so destination and source info also need to swap
        nFrom = CD->To->Qty;
        nTo = CD->From->Qty;

        destination = &src[ CD->From->idx ];
        source = &dest[ CD->To->idx ];
        int temp = nSrcCells;
        nSrcCells = nDestCells;
        nDestCells = temp;
    }

    if( CD->From == CD->To )
      sameGroup = 1;
    
    //determine the value that changes distance to delays
    double modifier = 1.0;
    if( CD->iCon->speed > 0 )
      modifier = NI->FSV/(CD->iCon->speed * 1000);
    else
      fprintf( stderr, "possible division by zero in connection - check speed values\n" );
    
    //create connections depending on probability scheme
    if( CD->iCon->step > 0 ) //decaying probability
    {
        //Must visit each possible connection.  Yes, this will be time consuming (O(n^2))
        *nMade = 0;
        memset(Map, 0, nFrom*nTo);
        for( i=0; i<nTo; i++ )  //for each destination cell,
        {
            for( j=0; j<nFrom; j++ ) //look at each source cell.
            {
                //calculate the distance
                distanceMap[ activeCell ] = distance( i, destination, nDestCells, j, source, nSrcCells );

                //do not connect a cell to itself
                if( sameGroup && (i==j) )
                {}
                //otherwise, throw random number, if a connection is made, mark map with 1 and increase count by 1
                else if ( RN->Rand() < CD->iCon->Prob*exp(-distanceMap[activeCell]/CD->iCon->step) )
                {
                    *nMade += (Map[ activeCell ] = 1);
                    distanceMap[ activeCell ] *= modifier;       //convert distance to delay
                }
                else
	                distanceMap[ activeCell ] = -1;
                activeCell++;
            }
        }
        
        CD->nCon = *nMade;
        
        //confirm CellMgr->maxCon > nMade, or update info if old Max exceeeded
        if( CellMgr->maxCon < *nMade )
        {
            CellMgr->maxCon = *nMade;
            CelIDs = (int*) realloc( CelIDs, sizeof(int)*CellMgr->maxCon );
            CmpIDs = (int*) realloc( CmpIDs, sizeof(int)*CellMgr->maxCon );
        }
        
        //make quick reference ID lists
        gatherConnectedIDs( CD, Map, CMBuf, CelIDs, CmpIDs );
    }
    else //probability does not decay, just need distances converted to delays
    {
        //Make connects as normal
        MakeConnects (CD, Map, nMade, CMBuf, CelIDs, CmpIDs);

        //use map to find connections, then calculate distances and modify
        for( i=0; i<nTo; i++ ) //for each destination cell,
        {
            for( j=0; j<nFrom; j++ ) //look at each source cell.
            {
                if( Map[ activeCell ] )  //was a connection made?
                    distanceMap[ activeCell ] = modifier * distance( i, destination, nDestCells, j, source, nSrcCells );
                else
                    distanceMap[ activeCell ] = -1;    //not really necessary, but just to be safe

                activeCell++;
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

inline double distance( int sIndex, double *source, int qty1, int dIndex, double *destination, int qty2 )
{
    int xIndex1, yIndex1, zIndex1, xIndex2, yIndex2, zIndex2;
    //xIndex has no offset, yIndex is offset by the number of cells, zIndex is offset by yIndex and the Number of cells again
    zIndex1 = qty1 + (yIndex1 = qty1 + (xIndex1 = sIndex)) ;
    zIndex2 = qty2 + (yIndex2 = qty2 + (xIndex2 = dIndex)) ;

    return sqrt( ( source[ xIndex1 ] - destination[ xIndex2 ] )*( source[ xIndex1 ] - destination[ xIndex2 ] ) +
                 ( source[ yIndex1 ] - destination[ yIndex2 ] )*( source[ yIndex1 ] - destination[ yIndex2 ] ) +
                 ( source[ zIndex1 ] - destination[ zIndex2 ] )*( source[ zIndex1 ] - destination[ zIndex2 ] ) );
}

/*---------------------------------------------------------------------------*/

void PrintMap (int nFrom, int nTo, char *Map)
{
  int i, j, ndx;
  char str [10240];

  str [0] = '+';
  for (i = 1; i <= nFrom; i++)
    str [i] = '-';
  str [i++] = '+';
  str [i++] = '\n';
  str [i++] = '\0';
  printf ("%s", str);

  for (j = ndx = 0; j < nTo; j++)
  {
    str [0] = '|';
    for (i = 1; i <= nFrom; i++, ndx++)
      str [i] = (Map [ndx] == 1) ? 'X' : ' ';
    str [i++] = '|';
    str [i++] = '\n';
    str [i++] = '\0';
    printf ("%s", str);
  }

  str [0] = '+';
  for (i = 1; i <= nFrom; i++)
    str [i] = '-';
  str [i++] = '+';
  str [i++] = '\n';
  str [i++] = '\0';
  printf ("%s", str);
}


/*---------------------------------------------------------------------------*/

void PrintClusters (ARRAYS *AR, FILE *RptFile)
{
  CLUSTER *CS;
  int i, n, iCol, iLay, iCel;

  fprintf (RptFile, "Cluster assignment by node\n");
  fprintf (RptFile, "  Node  Cluster     Column, Lay,  Cell  #cells  #synapses\n");
             
  for (i = 0; i < NI->nNodes; i++)
  {
    for (n = 0; n < CellMgr->nClusters; n++)
    {
      CS = CellMgr->Clusters [n];
  
      if (CS->Node == i)
      {
        iCol = CS->iCol;
        iLay = CS->iLay;
        iCel = CS->iCel;
    
        fprintf (RptFile, "  %3d %5d       %-s %-s %-s  %6d %8d\n", i, n,
                AR->Columns [iCol]->L.name, AR->Layers [iLay]->L.name, AR->Cells [iCel]->L.name,
                CS->Qty, CS->nSyn);
      }
    }
  }
}

/*--------------------------------------------------------------------------*/

void outputConnectMap( CONNECT *CD, char *Map, FILE *fout, int reversed )
{
    //output connections as:
    //option 1: entire map, with 0 for no connection, 1 for a connection
    //option 2: for each source cell, output destination indices as a list
    // going with option 1 unless we decide different in future
    MPI_Status status;

    if( NI->Node == 0 ) //do I have the map, or do I need to get it?
    {
        if( reversed && CD->From->Node == 0 && CD->To->Node != 0 )
        {
            //received the map in a previous function, so don't need to transmit it again
        }
        else if(CD->To->Node != 0 ) //I need to get the map
            MPI_Recv( Map, CD->From->Qty*CD->To->Qty, MPI_CHAR, CD->To->Node, 0, MPI_COMM_WORLD, &status );
        
        //output
        if( !fout )
            return;

        CLUSTER *A = CD->From, *B = CD->To;
        char *cmpA = CD->iCon->fromCmpName, *cmpB = CD->iCon->toCmpName;
        int qtyA = CD->From->Qty, qtyB = CD->To->Qty;
        if( reversed )
        {
            //dest is source, source is dest
            A = CD->To;
            B = CD->From;
            cmpA = CD->iCon->toCmpName;
            cmpB = CD->iCon->fromCmpName;
            qtyA = CD->To->Qty;
            qtyB = CD->From->Qty;
        }
        //source
        fprintf( fout, "%s %s %s %s\n", NI->AR->Columns[ A->iCol ]->L.name, NI->AR->Layers [ A->iLay ]->L.name,
                                        NI->AR->Cells [ A->iCel ]->L.name, cmpA );
        //dest
        fprintf( fout, "%s %s %s %s\n", NI->AR->Columns[ B->iCol ]->L.name, NI->AR->Layers [ B->iLay ]->L.name,
                                        NI->AR->Cells [ B->iCel ]->L.name, cmpB );
        //synapse
        fprintf( fout, "%s\n", CD->iCon->SynName );
        //nfrom nTo
        fprintf( fout, "%d %d\n", qtyA, qtyB );
        //connection matrix
        int rowOffset=0;
        for( int rowIndex=0; rowIndex<qtyB; rowIndex++ )
        {
            for( int colIndex=0; colIndex<qtyA; colIndex++ )
                fprintf( fout, "%c", Map[rowOffset+colIndex]+(int)'0' );
            fprintf( fout, "\n" );
            rowOffset+=qtyA;
        }
    }
    else //I need to send the map to node 0
    {
        //should I send data in chunks?
        MPI_Send( Map, CD->From->Qty*CD->To->Qty, MPI_CHAR, 0, 0, MPI_COMM_WORLD );
    }
}

/*--------------------------------------------------------------------------*/

static void AddToSendTo (int num, int node, int SynIdx, CONNECTMSG *CMBuf)
{
    TMPCMP *tCmp;
    int i, CelID, CmpID;

    for (i = 0; i < num; i++)
    {
        CelID = CMBuf [i].FromCell;
        CmpID = CMBuf [i].FromCmp;
        tCmp = &(tCells [CelID].Cmp [CmpID]);
        tCmp->Node.push_back (node);
        tCmp->Addr.push_back (CMBuf [i].Addr);
        tCmp->Delay.push_back (CMBuf [i].Delay);
    }
}

/*--------------------------------------------------------------------------*/

void acceptRecurrentConnection( CONNECT *CD, char *map, double *distanceMap, TMPCELL *tCel,
                                MPI_Comm &ConComm, MPI_Status &status, MPI_Datatype &MsgType, FILE *fout )
{
    int count = CD->To->Qty * CD->From->Qty;
    int bytesReceived;
    int nMade = 0;
    
    swapInfo( CD );
    
    if( count <= 0 )
        return;

    //this function is only called during mulitinode run
     
    //get map from "receiving" node
    bytesReceived = 0;
    while( bytesReceived < count )
    {
        if( count-bytesReceived >= XMIT_SIZE )
            MPI_Recv( &map[bytesReceived], XMIT_SIZE, MPI_CHAR, CD->From->Node, 0, ConComm, &status );
        else
            MPI_Recv( &map[bytesReceived], count-bytesReceived, MPI_CHAR, CD->From->Node, 0, ConComm, &status );
        bytesReceived+=XMIT_SIZE;
    }
    
    if( NI->AR->Brain->flag & USE_DISTANCE_FLAG )  //the user wants synaptic delays augmented by distance
    {
        //allocate
        int blockSize = (int) XMIT_SIZE/sizeof(double);
        
        bytesReceived = 0;
        while( bytesReceived < count )
        {
            if( count-bytesReceived >= blockSize )
                MPI_Recv( &distanceMap[bytesReceived], blockSize, MPI_DOUBLE, CD->From->Node, 0, ConComm, &status );
            else
                MPI_Recv( &distanceMap[bytesReceived], count-bytesReceived, MPI_DOUBLE, CD->From->Node, 0, ConComm, &status );
            bytesReceived+=blockSize;
        }
    }
    
    //quick scan to get nMade
    for( int i=0; i<count; i++ )
        if( map[i] ) nMade++;

    //with maps receivied, can create synapses
    createSynapses( CD, map, distanceMap, nMade, tCel, ConComm, status, MsgType );

    swapInfo( CD );

    //if this node (B) is also 0, but not node A - output the connection map BtoA
      //note - only need to check == 0, because if it was also A, we wouldn't be in this function
    if( NI->AR->Brain->flag & OUTPUT_CONNECT_MAP_FLAG && NI->Node == 0 )
      outputConnectMap( CD, map, fout, 1 );
}

/*--------------------------------------------------------------------------*/

void createSynapses( CONNECT *CD, char *map, double *distanceMap, int nMade, TMPCELL *tCel,
                     MPI_Comm &ConComm, MPI_Status &status, MPI_Datatype &MsgType )
{
    SynapseDef* iSyn = TheBrain->SynapseDefs [CD->iCon->SynType];
    int *CelIDs, CelID, CmpID, *CmpIDs;
    Synapse *sp;
    Compartment *Cmp;
    TMPCMP *tCmp;
    int delay, num=0;
    int nXmit = XMIT_SIZE / sizeof (CONNECTMSG);
        
    CONNECTMSG *CMBuf = new CONNECTMSG[nMade];
    CelIDs = new int[nMade];
    CmpIDs = new int[nMade];
    CmpID = CD->ToCmp;
    
    //gatherConnectedIDs
    gatherConnectedIDs( CD, map, CMBuf, CelIDs, CmpIDs );
        
    //create synapses
    for (int j = 0; j < nMade; j++)
    {
        CelID = CelIDs [j];

        Cmp  = TheBrain->Cells [CelID]->Compartments [CmpID];
        tCmp = &(tCells [CelID].Cmp [CmpID]);

        sp = new Synapse (iSyn, Cmp, RN);

        if (sp == NULL)
        {
            printf ("Memory allocation error in Connector, mem = %f.3 Mbytes\n", 
                  (double) GetMemoryUsed () / 1024.0);
            fprintf (stderr, "Memory allocation error in Connector, mem = %f.3 Mbytes\n",
                  (double) GetMemoryUsed () / 1024.0);
        }

        tCmp->Syn.push_back (sp);

        delay = (int) (NI->FSV * RN->RandRange (iSyn->SynDelay));

        if( NI->AR->Brain->flag & USE_DISTANCE_FLAG ) //distance matters - augment delay
            delay += (int) distanceMap[ (CMBuf[j].FromCell - CD->From->idx) + (CelID - CD->To->idx)*CD->From->Qty ];
        
        if (delay < 1) delay = 1;
        if (delay > NI->MaxSynDelay)
            NI->MaxSynDelay = delay;

        CMBuf [j].Addr  = sp;
        CMBuf [j].Delay = delay;
    }
    //all synapses are declared for this particular connection

    NI->Recv [CD->From->Node] += nMade;

    if (CD->From->Node == NI->Node)
    {
        AddToSendTo (nMade, CD->From->Node, CD->iCon->SynType, CMBuf);
        NI->Send [CD->To->Node] += nMade;
    }
    else
    {
        if( CD->iCon->step > 0 ) //decaying probability, must communicate number of connections
            MPI_Send( &(CD->nCon), 1, MPI_INT, CD->From->Node, 0, ConComm );
      
        int nSent = 0;
        while (nSent < nMade)
        {
            CONNECTMSG *XmitPtr = CMBuf + nSent;

            num = nXmit;
            if ((nSent + nXmit) > nMade)
                num = nMade - nSent;
            
            MPI_Send (XmitPtr, num, MsgType, CD->From->Node, 0, ConComm);

            nSent += num;
        }
    }

    delete [] CelIDs;
    delete [] CMBuf;
}

/*---------------------------------------------------------------------------
(08/20/03, JF )

In the input, connections between clusters of cells may be specified at
Brain, Column, or Layer levels.  Each such connection is FROM some group
of cells described by (Column, Layer, Cell, Cmp), TO some cluster, using
some synapse type.  The CellManager has previously looped through all
these input specifiers, and has resolved the CLCC specifiers to clusters
in the Global Cluster List and checked that they exist &c. 

For each such connection specifier, the CellManager creates a connect
descriptor (type CONNECT) in the global list CDList.  An identical copy
of this list exists on each node.  (At this point, clusters haven't yet
been assigned to nodes.) Once this list has been created, the program
goes through the Distributor and BuildBrain phases, to assign clusters to
nodes and create the cell objects.  Once that's done, this routine is
called to make the connections.

Each cmp contains a list of the compartments it sends to (SendTo), and a
list of the receiving synapses (SynapseList).  These lists are the two
halves of the connections.  The halves may (and usually will) be on
different nodes, so there must be some mechanism to insure that the two
match.  There are two possible approaches: the older code computed both
halves of all the connections on each node (using identical random number
streams), and the node selected only the items that are on it.  This was
slow and did not take advantage of parallelism.

In this new method, each node creates only the items that are on it.  The
receiving end is created first, and the information transmitted to the
sending node.  Cmps do not know the number of connections they will have
until the whole CDList has been processed, so STL vectors are used to
hold the information temporarily.

There are two steps.  In the first step, the code loops through the
CDList in random order.  For each descriptor, if the receiving cluster is
on the node, the connect map is created.  For each connection made, a
Synapse object is created and saved in a vector.  The sending-side
information is passed to the sending node, via MPI (or locally, if both
ends are on the node).  The sending side receives this info, and stores
it in other vectors.

(Note that this pattern ensures that MPI sends & recvs are posted in the
same order, so there should be no chance of deadlocking.)

In the second step, the number of SendTos and Synapses is known, so
exact-size arrays are allocated.  Saved information is used to create the
SendTos, the saved Synapse pointers are copied to the new array, and the
temporary structs are freed.

One optimization here is that Synapses which do positive learning are
placed first in the SynapseList, followed by other types.  This allows
the learning code to skip the synapses that don't do learning, rather
than looking at all, and checking (as the old code did).


Making Connections

Each CONNECT specifies a fixed number of connections, given by
(From.nCells * To.nCells * Prob).  In order to ensure that these
connections are unique, the routine creates an N x M map.  Each possible
From-To pair has an entry.  New connects are created as a random pair of
integers, which are indexes into the map.  If the corresponding entry is
already set, the pair is a duplicate.

Note: need to add check for connect-to-self when From and To are identical.


-----------------------------------------------------------------------------*/

/* This module handles assigning cells to nodes.  Given a computational      */
/* weight for each cluster (from CellManager), computes total weight and     */
/* assigns clusters to each node in proportion to the node's compute power   */
/* (determined from bogomips).  Note that the number of synapses a cell has  */
/* of compartments & synapses are just guesses...                            */

#include <stdio.h>
#include <string.h>
#include <mpi.h>

#include "Managers.h"
#include "debug.h"
#include "memstat.h"

#define MEM_KEY  KEY_DISTRIBUTE

#define SYNWEIGHT 0.10

extern NodeInfo *NI;

static void ByCluster ();
static void EqualSynapse ();
static void EqualOrder ();
static void Sort (CLUSTER **, int);
static int Compare (CLUSTER *, CLUSTER *);

/*---------------------------------------------------------------------------*/
/* Select which distribution algorithm to use                                */

void Distributor (char *alg)
{
  if (CellMgr->nClusters < NI->nNodes)
  {
    if (NI->Node == 0) 
    {
      printf ("Error: There are fewer clusters (%d) than nodes (%d)\n",
              CellMgr->nClusters, NI->nNodes);
      printf ("Restart the job on %d or fewer nodes\n", CellMgr->nClusters);
      fprintf (stderr, "Error: There are fewer clusters (%d) than nodes (%d)\n",
               CellMgr->nClusters, NI->nNodes);
      fprintf (stderr, "Restart the job on %d or fewer nodes\n", CellMgr->nClusters);
    }
    Abort (__FILE__, __LINE__, -10);
  }

  if (alg == NULL)
  {
    if (NI->Node == 0) 
      printf ("Distributor::Distributor: No algorithm specified, using default 'ByCluster'\n");
    ByCluster ();
  }
  else if (strcasecmp (alg, "ByCluster") == 0)
  {
    if (NI->Node == 0) 
      printf ("Distributor::Distributor: Using 'ByCluster' algorithm\n");
    ByCluster ();
  }
  else if (strcasecmp (alg, "EqualSynapse") == 0)
  {
    if (NI->Node == 0)
      printf ("Distributor::Distributor: Using 'EqualSynapse' algorithm\n");
    EqualSynapse ();
  }
  else if (strcasecmp (alg, "EqualOrder") == 0)
  {
    if (NI->Node == 0)
      printf ("Distributor::Distributor: Using '%s ' algorithm\n", alg);
    EqualOrder ();
  }
  else if (strcasecmp (alg, "EqualCells") == 0)
  {
    if (NI->Node == 0) 
    {
      printf ("Distributor::Distributor: The '%s' algorithm does not work in this version, aborting...\n", alg);
      Abort (__FILE__, __LINE__, -11);
    }
//  if (NI->Node == 0)
//    printf ("Distributor::Distributor: Using 'EqualCells' algorithm\n");
//  EqualCells ();
  }
  else if (strcasecmp (alg, "CellSynapse") == 0)
  {
    if (NI->Node == 0) 
    {
      printf ("Distributor::Distributor: The '%s' algorithm does not work in this version, aborting...\n", alg);
      Abort (__FILE__, __LINE__, -11);
    }
//  if (NI->Node == 0)
//    printf ("Distributor::Distributor: Using 'CellSynapse' algorithm\n");
//  CellSynapse ();
  }
  else 
  {
    if (NI->Node == 0) 
    {
      printf ("Distributor::Distributor: Unknown algorithm specified, aborting...\n");
      Abort (__FILE__, __LINE__, -11);
    }
  }
  CellMgr->SetDistFlag ();
}

/*---------------------------------------------------------------------------*/
/* Assigns clusters to nodes by weight of cells + synapses.  Algorithm is    */
/*   For each cluster, assign to node with lowest percentage of weight       */

static void ByCluster ()
{
  CLUSTER *CS;
  double weight, ratio, F, Fmin;
  int i, j, iNode, *LocalIDs;

  if (NI->Node == 0) 
    printf ("Distributor::ByCluster: Synapse factor = %6.3f\n", SYNWEIGHT);

/* First add up the total weights of all the clusters.  As a first cut, the  */
/* weight is 1 unit for the cell itself, plus 0.1 unit for each synapse.     */
/* (Counted on the receiving end of the connection.)                         */

  weight = 0.0;
  for (i = 0; i < CellMgr->nClusters; i++)
  {
    CS = CellMgr->Clusters [i];
    CS->weight = (double) CS->Qty + SYNWEIGHT * (double) CS->nSyn;
    weight += CS->weight;
  }

/* Now split this total weight among the nodes, in proportion to their Bmips */

  ratio = weight / NI->TotalBmips;
  for (i = 0; i < NI->nNodes; i++)
  {
    NI->weight  [i] = ratio * NI->Bmips [i];
    NI->Aweight [i] = 0.0;
  }

/* Simple allocation scheme: clusters are assigned round-robin fashion, to   */
/* make sure that every node gets at least one cluster.                      */

  LocalIDs = (int *) calloc (NI->nNodes, sizeof (int));

/* First step: assign clusters to nodes so that a) all nodes get at least    */
/* one cluster; b) no node gets more than its allocated weight, unless the   */
/* one cluster is more than allocated.                                       */

  iNode = i = 0;
  while (i < CellMgr->nClusters) 
  {
    CS = CellMgr->Clusters [i++];

    if ((NI->nClusters [iNode] > 0) && ((NI->Aweight [iNode] + CS->weight) > NI->weight [iNode]))
    {
      iNode++;
      if (iNode >= NI->nNodes) break;
    }

    CS->Node = iNode;
    NI->Aweight   [iNode] += CS->weight;
    NI->nClusters [iNode]++;
  }

/* There are probably some clusters still not assigned, so go through the    */
/* remainder, and assign them to the nodes that have the least percentage of */
/* assigned weight.                                                          */

  while (i < CellMgr->nClusters) 
  {
    CS = CellMgr->Clusters [i++];

    Fmin = 100000.0;     /* Some large number... */

    for (j = 0; j < NI->nNodes; j++)               /* Find node with lowest fraction. */
    {
      F = NI->Aweight [j] / NI->weight [j];
      if (F < Fmin)
      {
        Fmin = F;
        iNode = j;
      }
    }

    CS->Node = iNode;
    NI->Aweight   [iNode] += CS->weight;
    NI->nClusters [iNode]++;
  }

/* Now one more loop to assign CellIDs and compute quantities... */

  for (i = 0; i < CellMgr->nClusters; i++)
  {
    CS = CellMgr->Clusters [i];

    iNode = CS->Node;
    CS->idx  = LocalIDs [iNode];
    LocalIDs [iNode] += CS->Qty;
    NI->nCells    [iNode] += CS->Qty;
    NI->nSynapses [iNode] += CS->nSyn;

    if (NI->check && (NI->Node == 0))
      printf ("%4d: %3d  (%5d, %5d)\n", i, iNode, CS->idx, CS->Qty);
  }

  CellMgr->nLocalCells = NI->nCells [NI->Node];

  free (LocalIDs);
}

/*---------------------------------------------------------------------------*/
/* This trys to assign an equal number of synapses (and thus memory) to each */
/* node, regardless of cpu power.  For large models, so all memory is used.  */

static void EqualSynapse ()
{
  CLUSTER *CS;
  double weight, ratio, F, Fmin;
  int i, j, iNode, *LocalIDs;

/* First add up the total weights of all the clusters.  As a first cut, the  */
/* weight is 1 unit for the cell itself, plus 0.1 unit for each synapse.     */
/* (Counted on the receiving end of the connection.)                         */

  weight = 0.0;
  for (i = 0; i < CellMgr->nClusters; i++)
  {
    CS = CellMgr->Clusters [i];
    CS->weight = (double) (CS->nSyn * sizeof (Synapse))
               + (double) (CS->nSendTo * sizeof (SENDITEM));
    weight += CS->weight;
  }

/* Now split this total weight among the nodes, in proportion to their Bmips */

  ratio = weight / NI->nNodes;
  for (i = 0; i < NI->nNodes; i++)
  {
    NI->weight  [i] = ratio;
    NI->Aweight [i] = 0.0;
  }

/* Simple allocation scheme: clusters are assigned round-robin fashion, to   */
/* make sure that every node gets at least one cluster.                      */

  LocalIDs = (int *) calloc (NI->nNodes, sizeof (int));

/* First step: assign clusters to nodes so that a) all nodes get at least    */
/* one cluster; b) no node gets more than its allocated weight, unless the   */
/* one cluster is more than allocated.                                       */

  iNode = i = 0;
  while (i < CellMgr->nClusters) 
  {
    CS = CellMgr->Clusters [i++];

    if ((NI->nClusters [iNode] > 0) && ((NI->Aweight [iNode] + CS->weight) > NI->weight [iNode]))
    {
      iNode++;
      if (iNode >= NI->nNodes) break;
    }

    CS->Node = iNode;
    NI->Aweight   [iNode] += CS->weight;
    NI->nClusters [iNode]++;
  }

/* There are probably some clusters still not assigned, so go through the    */
/* remainder, and assign them to the nodes that have the least percentage of */
/* assigned weight.                                                          */

  while (i < CellMgr->nClusters) 
  {
    CS = CellMgr->Clusters [i++];

    Fmin = 100000.0;     /* Some large number... */

    for (j = 0; j < NI->nNodes; j++)               /* Find node with lowest fraction. */
    {
      F = NI->Aweight [j] / NI->weight [j];
      if (F < Fmin)
      {
        Fmin = F;
        iNode = j;
      }
    }

    CS->Node = iNode;
    NI->Aweight   [iNode] += CS->weight;
    NI->nClusters [iNode]++;
  }

/* Now one more loop to assign CellIDs and compute quantities... */

  for (i = 0; i < CellMgr->nClusters; i++)
  {
    CS = CellMgr->Clusters [i];

    iNode = CS->Node;
    CS->idx  = LocalIDs [iNode];
    LocalIDs [iNode] += CS->Qty;
    NI->nCells    [iNode] += CS->Qty;
    NI->nSynapses [iNode] += CS->nSyn;

    if (NI->check && (NI->Node == 0))
      printf ("%4d: %3d  (%5d, %5d)\n", i, iNode, CS->idx, CS->Qty);
  }

  CellMgr->nLocalCells = NI->nCells [NI->Node];

  free (LocalIDs);
}

/*---------------------------------------------------------------------------*/
/* This trys to assign an equal number of synapses (and thus memory) to each */
/* node, regardless of cpu power.  For large models, so all memory is used.  */
/* Sort clusters by size, and assign in that order.                          */

static void EqualOrder ()
{
  CLUSTER **Sorted;
  CLUSTER *CS;
  double weight, ratio, F, Fmin;
  int i, j, iNode, *LocalIDs;

/* First compute the total weights of all the clusters, and add them up.     */
/* Here the weight is memory usage of both synapse and sendto.               */

  weight = 0.0;
  for (i = 0; i < CellMgr->nClusters; i++)
  {
    CS = CellMgr->Clusters [i];
    CS->weight = (double) (CS->nSyn * sizeof (Synapse))
               + (double) (CS->nSendTo * sizeof (SENDITEM));
    weight += CS->weight;
  }

/* Allocate a temporary cluster array, and sort it by weight */

  Sorted = (CLUSTER **) calloc (CellMgr->nClusters, sizeof (CLUSTER *));
  for (i = 0; i < CellMgr->nClusters; i++)
    Sorted [i] = CellMgr->Clusters [i];

  Sort (Sorted, CellMgr->nClusters);
  if (NI->Node == 0)
  {
    for (i = 0; i < CellMgr->nClusters; i++)
      printf ("Sort  %4d = %8d\n", i, Sorted [i]->nSyn);
  }

/* Now split this total weight among the nodes, in proportion to their Bmips */

  ratio = weight / NI->nNodes;
  for (i = 0; i < NI->nNodes; i++)
  {
    NI->weight  [i] = ratio;
    NI->Aweight [i] = 0.0;
  }

/* Simple allocation scheme: clusters are assigned round-robin fashion, to   */
/* make sure that every node gets at least one cluster.                      */

  LocalIDs = (int *) calloc (NI->nNodes, sizeof (int));

/* First step: assign clusters to nodes so that a) all nodes get at least    */
/* one cluster; b) no node gets more than its allocated weight, unless the   */
/* first cluster assigned is more than allocated weight.                     */

  iNode = i = 0;
  while (i < CellMgr->nClusters) 
  {
    CS = Sorted [i++];

    if ((NI->nClusters [iNode] > 0) && ((NI->Aweight [iNode] + CS->weight) > NI->weight [iNode]))
    {
      iNode++;
      if (iNode >= NI->nNodes) break;
    }

    CS->Node = iNode;
    NI->Aweight   [iNode] += CS->weight;
    NI->nClusters [iNode]++;
  }

/* There are probably some clusters still not assigned, so go through the    */
/* remainder, and assign them to the nodes that have the least percentage of */
/* assigned weight.                                                          */

  while (i < CellMgr->nClusters) 
  {
    CS = Sorted [i++];

    Fmin = 100000.0;     /* Some large number... */

    for (j = 0; j < NI->nNodes; j++)               /* Find node with lowest fraction. */
    {
      F = NI->Aweight [j] / NI->weight [j];
      if (F < Fmin)
      {
        Fmin = F;
        iNode = j;
      }
    }

    CS->Node = iNode;
    NI->Aweight   [iNode] += CS->weight;
    NI->nClusters [iNode]++;
  }

/* Now one more loop to assign CellIDs and compute quantities... */

  for (i = 0; i < CellMgr->nClusters; i++)
  {
    CS = CellMgr->Clusters [i];

    iNode = CS->Node;
    CS->idx  = LocalIDs [iNode];
    LocalIDs [iNode] += CS->Qty;
    NI->nCells    [iNode] += CS->Qty;
    NI->nSynapses [iNode] += CS->nSyn;

    if (NI->check && (NI->Node == 0))
      printf ("%4d: %3d  (%5d, %5d)\n", i, iNode, CS->idx, CS->Qty);
  }

  CellMgr->nLocalCells = NI->nCells [NI->Node];

  free (LocalIDs);
}

/*---------------------------------------------------------------------------*/
/* Sorts list of clusters by Col, Layer, Cell.                               */

static void Sort (CLUSTER *array [], int len)
{
  CLUSTER *tmp;
  int i, j, gap;

  for (gap = len / 2; gap > 0; gap /= 2)
  {
    for (i = gap; i < len; i++)
    {
      for (j = i - gap; j >= 0; j -= gap)
      {
        if (Compare (array [j], array [j + gap]) <= 0) break;
        tmp = array [j];
        array [j] = array [j+gap];
        array [j+gap] = tmp;
      }
    }
  }
}

/*------------------------------------------------------------------------*/

static int Compare (CLUSTER *a, CLUSTER *b)
{
  int rc;

  if (a->weight < b->weight)
    rc = 1;
  else if (a->weight > b->weight)
    rc = -1;
  else
    rc = 0;
  return (rc);
}

/*---------------------------------------------------------------------------*/
/* Do checks on load distribution                                            */

void DCheck (ARRAYS *AR, bool check)
{
  CONNECT *CD;
  CLUSTER *CS, *CFrom, *CTo;
  int i, n, nzero, lastnode, iCol1, iLay1, iCel1, iCol2, iLay2, iCel2, nCells, nSynapses;

  printf ("Distributor::Distributor: Loads assigned as follows:\n");
  printf ("   Node   Clusters  Cells  Synapses   Bmips  weight    Assigned\n");

  nzero = nCells = nSynapses = 0;
  for (i = 0; i < NI->nNodes; i++)
  {
    printf ("      %3d: %5d %8d %12d %5.1f %10.1f %10.1f\n", i, NI->nClusters [i],
            NI->nCells [i], NI->nSynapses [i], NI->Bmips [i], NI->weight [i], NI->Aweight [i]);

    if (NI->nClusters [i] == 0) nzero++;
    nCells    += NI->nCells [i];
    nSynapses += NI->nSynapses [i];
  }

  printf ("  Total: %d cells, %d synapses\n", nCells, nSynapses);

  if (nzero > 0) 
    fprintf (stdout, "Warning, %d of %d nodes have no clusters assigned to them\n",
             nzero, NI->nNodes);

  if (check)
  {
    lastnode = n = 0;
    printf ("Cluster assignment by node\n");
    printf ("  Cluster #  Node           Column   Lay  Cell  #cells  weight\n");
               
    for (i = 0; i < CellMgr->nClusters; i++)
    {
      CS = CellMgr->Clusters [i];
  
      if (CS->Node != lastnode)
      {
        printf ("\n");
        lastnode = CS->Node;
        n++;
      }
  
      iCol1 = CS->iCol;
      iLay1 = CS->iLay;
      iCel1 = CS->iCel;
  
      printf ("    %5d, %5d: %16s %4s %4s  %6d %8.1f\n", i, n,
              AR->Columns [iCol1]->L.name, AR->Layers [iLay1]->L.name, AR->Cells [iCel1]->L.name,
              CS->Qty, CS->weight);
    }
  
    printf ("\n");
    printf ("Connect Descriptors\n");
    printf ("  Connect #   From  Col      Lay  Cel         To  Col      Lay  Cel   #connects\n");
               
    for (i = 0; i < CellMgr->nCD; i++)
    {
      CD = &(CellMgr->CDList [i]);
      CFrom = CD->From;
      CTo   = CD->To;
  
      iCol1 = CFrom->iCol;
      iLay1 = CFrom->iLay;
      iCel1 = CFrom->iCel;
  
      iCol2 = CTo->iCol;
      iLay2 = CTo->iLay;
      iCel2 = CTo->iCel;
  
      printf ("    %5d: %16s %4s %4s -> %16s %4s %4s %8d\n", i,
              AR->Columns [iCol1]->L.name, AR->Layers [iLay1]->L.name, AR->Cells [iCel1]->L.name,
              AR->Columns [iCol2]->L.name, AR->Layers [iLay2]->L.name, AR->Cells [iCel2]->L.name,
              CD->nCon);
    }
    printf ("DCheck done\n");
  }
}

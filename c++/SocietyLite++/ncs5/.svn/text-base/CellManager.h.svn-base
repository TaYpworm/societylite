#ifndef CELLMANAGER_H
#define CELLMANAGER_H

/* 04/25/02 - JF: Creates a translation layer between input specs and        */
/* working program.  See extensive comments in .cpp file.                    */

#include <stdio.h>
#include <string.h>

#include "RandomManager.h"
#include "GCList.h"
#include "defines.h"
#include "parse/arrays.h"

class CellManager
{
public:
  bool HasError;
  bool DistDone;             /* Set to true when DistributionManager has */
                             /* assigned cells to nodes.  Some functions */
                             /* can't return valid results before that.  */

  int nLocalCells;           /* number of cells on this node */
  int nGlobalCells;          /* total number of cells = size of GCList  */

  int nClusters;             /* size of list = total number of cell clusters  */
  CLUSTER *CSData;           /* Pointer to actual cs structs - should be private? */
  CLUSTER **Clusters;        /* Pointer to Cell Cluster list */

  int nCD; 
  CONNECT *CDList;           /* The connect descriptor list  */

  int maxCon;                /* Largest number of connections in any descriptor */
  int maxQty;                /* Largest number of Cells in any Cell_Type */

  Random   *RN;

  CellManager ();             // class constructor
  ~CellManager ();            // class destructor
  
  void MakeGCList (ARRAYS *, RandomManager *);
  void CheckCD (T_CONNECT *, int *, ARRAYS *, char *);
  void AddCDToLists (T_CONNECT *, CONNECT *);
  void CSSort (CLUSTER **, int);
  int CScompare (CLUSTER *, CLUSTER *);
  CLUSTER *GetCSItem (int, int, int);
  int FindCSItem (int, int, int);                      //for getting index into list for this item
  void Error1 (char *, int, T_CONNECT *, ARRAYS *);

  void SetDistFlag ();
  int GetFirstCellID (int, int, int);
  int *ListCells (int, int, int, int *, int *);
  int *ListCellsOnNode (int, int, int, int *);

  void FreeCDList ();
  void PrintClusterList (ARRAYS *);

  /**
   * Write Cluster data to a file.  Only the head node needs to do this since
   * this information will be duplicated on all other nodes.  Pointers wihin a CLUSTER
   * object do not
   * need to be followed since they are only used during the brain's first creation.
   * @param out FILE pointer to destination file
   * @return number of bytes written
   */
  int Save( FILE *out );

  /**
   * Restore Cluster data from a File. The Head Node will read the data and send
   * it to all other Nodes using MPI.  Then each node can parse it into the apppropriate
   * structures.  Pointers within a CLUSTER object will be set to NULL since they
   * were only needed when the
   * brain was first created and will not be used again (hopefully).
   * @param in FILE pointer to source file.
   */
  void Load( FILE *in );
  
};

#endif 

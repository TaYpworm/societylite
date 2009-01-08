#define _MAIN_

/* NCS Version 4 - July 2002                           */
#include <stdio.h>

#include "version.h"
#include "Brain.h"
#include "BuildBrain.h"
#include "RandomManager.h"
#include "KillFile.h"
#include "DoStim.h"
#include "DoReport.h"
#include "Memory.h"

#include "parse/arrays.h"
#include "debug.h"
#include "memstat.h"

#include "QQ.h"

#define MEM_KEY KEY_MAIN

/* Here is where the global objects and variables - the Managers - are       */
/* defined.                                                                  */

#include "Managers.h"

NodeInfo *NI;
Brain *TheBrain;
Input *In;
MessageManager *MsgMgr;
MessageBus *MsgBus;
CellManager *CellMgr;
ActiveSynPool *ActiveSynMgr;

int gdbval = 0;           /* These are here for debugging.  gdb needs global */
int *gdbptr = &gdbval;    /* values to use with watches... */

int DB_Node, DB_flag = 0;   /* for debugging, used with dprintf, etc */

void Connector (ARRAYS *, RandomManager *);
void Distributor (char *);
void DCheck (ARRAYS *, bool);
void GetCompileOpts (void);

extern "C" ARRAYS *ParseInput (int, char *, int);

#include <iostream>
using namespace std;

int main (int argc, char *argv [])
{
  ARRAYS *AR;
  RandomManager *RM;        
  double t1, t2, ParseTime, InitTime, ThinkTime, mem;
  int node, nodes, maxdel, rc;
  int QQThink;
  char QQname [128];

/*------------------------------------------------------------------------*/

#ifdef MEM_STATS
  MEMINIT (0);
  fprintf (stdout, "main\n");
#endif

  GetMaxFiles ();

/* MPI initialization must be done first, because 1) MPI adds MPI-specific   */
/* args to the command line, which MPI_Init strips from argv; and 2) the     */
/* node number must be known from the beginning, because other parts of the  */
/* program need to know it.                                                  */

  MPI_Init (&argc, &argv);

  MPI_Comm_rank (MPI_COMM_WORLD, &node);
  MPI_Comm_size (MPI_COMM_WORLD, &nodes);
  DB_Node = node;

/* Creating the NodeInfo object does MPI initialization.  */

  NI = new NodeInfo (node, nodes, argc, argv);

/* Start up the input transmission path */

  In = new Input ();

/* make the kill file */

  MakeKillFile ();

/* Print out some info on sizes of things.  This is for development... */

  if (NI->Node == 0)     
  {
    printf ("Version is %s\n", VERSION);
    GetCompileOpts ();

    printf ("Sizes\n");
    printf ("Synapse       = %4d bytes\n",   sizeof (Synapse));
    printf ("SendTo        = %4d bytes\n",   sizeof (SENDITEM));
    printf ("ACTIVESYN     = %4d bytes\n\n", sizeof (ACTIVESYN));

    printf ("SynDef        = %4d bytes\n",   sizeof (SynapseDef));

    printf ("PACKET        = %4d bytes\n",   sizeof (PACKET));
    printf ("Message       = %4d bytes\n",   sizeof (Message));
    printf ("Msgs/Packet   = %4d \n\n",      MSGS_PER_PACKET);

    printf ("Cell          = %4d bytes\n",   sizeof (Cell));
    printf ("Compartment   = %4d bytes\n",   sizeof (Compartment));
    printf ("Channel       = %4d bytes\n",   sizeof (Channel));
    printf ("NodeInfo      = %4d bytes\n\n", sizeof (NodeInfo));

    printf ("Cluster       = %4d bytes\n",   sizeof (CLUSTER));
    printf ("Connect       = %4d bytes\n\n", sizeof (CONNECT));
  }

/* This is for debugging, to allow time for gdb to attach to the process... */

  if (NI->nSleep > 0)
  {
    fprintf (stderr, "sleeping %d, PID = %d\n", NI->nSleep, getpid ());
    printf ("sleeping %d, PID = %d\n", NI->nSleep, getpid ());
    sleep (NI->nSleep);
    fprintf (stderr, "awake!\n");
  }

/* Start tracing module */

  QQInit (1000000);
  QQThink = QQAddState ("Think");


/* Read & parse the input file.  The Input function returns a pointer to the */
/* ARRAYS structure, which contains all the input data.                      */

  if (NI->Node == 0) printf ("%s parsing input file %s\n", argv [0], NI->input);
  AR = ParseInput (NI->Node, NI->input, 0);

  if (AR == NULL)
  {
    printf ("Main: parser returned with errors, quitting\n");
    fprintf (stderr, "Main: parser returned with errors, quitting\n");
    Abort (__FILE__, __LINE__, -1);
  } 
  //put pointer to AR in NodeInfo as well
  NI->AR = AR;

  ParseTime = MPI_Wtime () - NI->tstart;

  if (NI->Node == 0) 
    printf ("----- Finished parsing input %.3f sec, mem = %8.3f MBytes ----- \n",
            ParseTime, GetMemoryUsed () / 1024.0);

/*---------------------------------------------------------------------------*/
/* The input file has been successfully parsed.  Now create the Brain itself */
/* (as most things are ultimately rooted in it), and other objects needed    */
/* for construction.                                                         */

  NI->FSV = AR->Brain->FSV;        // from user input, Hz
  NI->job = AR->Brain->job;
  if (NI->job == NULL) NI->job = "job";
  if (NI->Node == 0) printf ("Job = '%s'\n", NI->job);
  NI->ConnectRpt = AR->Brain->ConnectRpt;
  NI->SpikeRpt   = AR->Brain->SpikeRpt;

  t1 = MPI_Wtime ();
  TheBrain = new Brain (AR->Brain);
  RM       = new RandomManager (AR);

  if( NI->AR->Brain->loadfile ) //restored a brain need to patch pointers
  {
    AR = NI->AR;
    NI->job = AR->Brain->job;
    if (NI->job == NULL) NI->job = "job";
  }
  
/* The base of the brain has been made, now create.  */
  if( !NI->AR->Brain->loadfile )
  {
    CellMgr = new CellManager ();
    CellMgr->MakeGCList (AR, RM);
  }
  else //brain has already restored the cell manager
  {}
  
  if (CellMgr->HasError)
  {
    if (NI->Node == 0) 
    {
      printf ("Errors in CellManager, can't continue...\n");
      fprintf (stderr, "Errors in CellManager, can't continue...\n");
    }
    Abort (__FILE__, __LINE__, 1); 
  }

  t2 = MPI_Wtime ();
  printf ("Cluster & Connect lists created, %.3f sec, mem = %8.3f MBytes\n",
          t2 - t1, (double) GetMemoryUsed () / 1024.0);

/* Now the clusters can be distributed to the nodes                          */
  if( !AR->Brain->loadfile )
  {
    Distributor (AR->Brain->distribute);
    printf ("Distributor, mem = %8.3f MBytes\n", (double) GetMemoryUsed () / 1024.0);

    if (NI->Node == 0) DCheck (AR, NI->check);
    if (NI->check)
    {
      if (NI->Node == 0) EraseKillFile ();
      MPI_Finalize ();
      exit (1);
    }
  }
  else
    CellMgr->SetDistFlag();

//if (NI->Node == 0) CellMgr->PrintClusterList (AR);

/* Now build the brain: that is, create the cells &c that are on this node   */
  if( !AR->Brain->loadfile )
    BuildBrain (AR, RM);     //need to build brain from scratch

  char nothing[] = "need to remove this";
  if( AR->Brain->loadfile )
    TheBrain->Load( AR->Brain->loadfile, &nothing[0] );

//if (NI->Node == 0) 
//  printf ("nCellList = %d\n", TheBrain->nCellList);

  printf ("BuildBrain finished, mem = %8.3f MBytes\n", GetMemoryUsed () / 1024.0);

  if( !AR->Brain->loadfile ) //this doesn't need to be done if loading a brain
    Connector (AR, RM);

  MPI_Allreduce (&(NI->MaxSynDelay), &maxdel, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
  NI->MaxSynDelay = maxdel;

  if (NI->Node == 0) printf ("NI->MaxSynDelay = %d\n", NI->MaxSynDelay);

  printf ("After Connection, mem = %8.3f MBytes\n", GetMemoryUsed () / 1024.0);

  CellMgr->FreeCDList ();

  printf ("After FreeCDList, mem = %8.3f MBytes\n", GetMemoryUsed () / 1024.0);

  t1 = MPI_Wtime ();
  printf ("main: time for connection & distribution = %f\n", t1 - t2);

  /* The cell position info isn't needed any longer, so free it */


/* Now that the brain is complete, create stimuli & reports from input */
  MakeStimsFromInput (AR, RM);
  MakeReportsFromInput (AR, RM);

/* create the message bus, and initialize it */
  MsgMgr = new MessageManager (NI->Node);
  MsgBus = new MessageBus ();
  ActiveSynMgr = new ActiveSynPool (NI->Node);

  t2 = MPI_Wtime ();
  InitTime = t2 - NI->tstart;

  mem = GetMemoryUsed () / 1024.0;
  fprintf (stderr, "Main: initialization complete (%f sec) calling Brain::DoThink, mem = %6.1f MBytes\n",
           InitTime, mem);
  printf ("Main: initialization complete (%f sec) calling Brain::DoThink, mem = %6.1f MBytes\n",
           InitTime, mem);

/* and finally the brain can start thinking! */

  QQStateOn (QQThink);
  QQBaseTime (0);
  rc = TheBrain->DoThink ();
  QQStateOff (QQThink);

  t2 = MPI_Wtime ();
  ThinkTime = t2 - t1;
  mem = GetMemoryUsed () / 1024.0;
  printf ("main:: thinking finished (%f sec), mem = %8.3f MBytes\n", ThinkTime, mem);
  fprintf (stderr, "Node %d: main:: thinking finished (%f sec), mem = %8.3f MBytes\n",
           NI->Node, ThinkTime, mem);

//  MEMPRINT (1);

  sprintf (QQname, "qqm%d", nodes);
  QQOut (QQname, NI->Node, NI->nNodes);

/* This makes sure all nodes have indeed gotten this far without error.  If  */
/* not, the kill file will be left so the user can kill everything           */

  t1 = AllDone (15.0);

  printf ("Cortical Simulator over --- Have A Nice Day!\n");
  if (NI->Node == 0) printf ("Total run time = %f sec\n", t1);
}

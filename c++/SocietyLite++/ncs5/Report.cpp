/* This object handles collection of data for a report See long explanation  */
/* at end of file.  7/9/02 - JF                                              */

#include <stdlib.h>

#include "Report.h"
#include "Managers.h"
#include "Memory.h"

#include "debug.h"
#include "memstat.h"

#define MEM_KEY  KEY_REPORT

#include "QQ.h"
extern int QQReport;

#include <iostream>

using namespace std;

Report::Report (T_REPORT *iRep, char *header, Random *RN)
{
//printf ("Report::Report: filename = %s\n", iRep->FileName);

  MEMADDOBJ (MEM_KEY);
  
  idx = iRep->L.idx;

  Freq      = iRep->Frequency;
  ReportOn  = iRep->ReportOn;
  nThings   = 0;
  ThingList = NULL;
  RptBuf    = NULL;
  Out       = NULL;
  Started   = Opened = false;
  family = -1;

  iTime       = 0;
  nTimes      = iRep->nTimes;
  TStart      = iRep->Time_Start;
  TStop       = iRep->Time_Stop;
  FreqCounter = 0;
  ThingSize   = sizeof (float);        /* default */
  reportFlag  = iRep->reportFlag;

  GetThingsToReport (iRep, RN);
  if (nThings > 0)
  {
    Out = new Output (iRep, nThings, ThingSize, header);

    //Allocate report buffer and handle special case
    AllocateBuffer();

    if (RptBuf == NULL) MemError ("Report::Report, RptBuf");
  }
  
  //A dynamic report will have start/stop times add via port commands
  //This flag will prevent DoReport from deleting the report due to being out of times
  Dynamic = 0;
  if (nTimes == 0 )
    Dynamic = 1;
  else if( TStart[0] < 0 && TStop[0] < 0 ) //user has made first time pair = -1
    Dynamic = 1;
  
/* The object inserts itself into the Brain's list.  It's always inserted,   */
/* and deleted by the AddReport function if it's not valid.                  */
  this->next = TheBrain->RptList;
  TheBrain->RptList = this;
  if (this->next != NULL) this->next->prev = this;
  this->prev = NULL;
  TheBrain->nReports++;
}

/*------------------------------------------------------------------------*/

Report::~Report ()
{
  MEMFREEOBJ (MEM_KEY);
  if (Out != NULL) 
  {
    if (Started) Out->FinishOutput (RptBuf);
    delete Out;
  }

  if (ThingList != NULL) free (ThingList);
  if (RptBuf    != NULL) free (RptBuf);

  if (this == TheBrain->RptList)    /* Report is first in list */
    TheBrain->RptList = this->next;

  if (this->next != NULL) this->next->prev = this->prev;
  if (this->prev != NULL) this->prev->next = this->next;

  TheBrain->nReports--;
}

/*---------------------------------------------------------------------------*/

void Report::AllocateBuffer()
{
    if( family != -1 )
    {
      RptBuf = (void *) malloc (nThings * ThingSize * getFamilyCount() );
      Out->setItems( getFamilyCount() );
    }
    else if( ReportOn == FIRE_COUNT )
    {
      //only need one element
      RptBuf = (void *) malloc (ThingSize);

      //This case is very special in that this Report object will know
      // that nThings is the number of reporting compartments,
      // but it will just count the number of firings in a timestep
      // and output one value.
      // Therefore, the output object will kepp a different value for
      // nThings
      Out->nThings = 1;
    }
    else
      RptBuf = (void *) malloc (nThings * ThingSize);
}

/*---------------------------------------------------------------------------*/
/* This function gets the list of things that are to be reported.  The       */
/* ThingList is a list of pointers, either to the objects that have the      */
/* values to be reported, or to the values themselves.  This does two        */
/* things: it puts as much overhead up front as possible, and it allows much */
/* more general reporting than the old "report on cell/cmp" model.  In the   */
/* process of building this list, the routine also counts the things,        */
/* computes the buffer space needed, and builds the report header.           */

void Report::GetThingsToReport (T_REPORT *iRep, Random *RN)
{
  Compartment *Cmp;
  Synapse *Syn;
  int i;

  switch (ReportOn)
  {
    case VOLTAGE:         GetCmpPtrs (iRep, RN);
                          for (i = 0; i < nThings; i++)
                          {
                            Cmp = (Compartment *) ThingList [i];
                            ThingList [i] = (void *) &(Cmp->VoltageMembrane);
                          }
                          Func = &Report::GetDblPtrVal;
        break;
    case NET_CURRENT:     GetCmpPtrs (iRep, RN);
                          for (i = 0; i < nThings; i++)
                          {
                            Cmp = (Compartment *) ThingList [i];
                            ThingList [i] = (void *) &(Cmp->NetCurrent);
                          }
                          Func = &Report::GetDblPtrVal;
        break;
    case STIM_CURRENT:    GetCmpPtrs (iRep, RN);
                          for (i = 0; i < nThings; i++)
                          {
                            Cmp = (Compartment *) ThingList [i];
                            ThingList [i] = (void *) &(Cmp->TotalStimulusCurrent);
                          }
                          Func = &Report::GetDblPtrVal;
        break;
    case LEAK_CURRENT:    GetCmpPtrs (iRep, RN);
                          for (i = 0; i < nThings; i++)
                          {
                            Cmp = (Compartment *) ThingList [i];
                            ThingList [i] = (void *) &(Cmp->Current_Leak);
                          }
                          Func = &Report::GetDblPtrVal;
        break;
    case ADJ_CURRENT:     GetCmpPtrs (iRep, RN);
                          for (i = 0; i < nThings; i++)
                          {
                            Cmp = (Compartment *) ThingList [i];
                            ThingList [i] = (void *) &(Cmp->TotalAdjacentCompartmentCurrent);
                          }
                          Func = &Report::GetDblPtrVal;
        break;
    case SYN_CURRENT:     GetCmpPtrs (iRep, RN);
                          for (i = 0; i < nThings; i++)
                          {
                            Cmp = (Compartment *) ThingList [i];
                            ThingList [i] = (void *) &(Cmp->TotalSynapseCurrent);
                          }
                          Func = &Report::GetDblPtrVal;
        break;
    case CHANNEL_CURRENT: GetCmpPtrs (iRep, RN);
                          for (i = 0; i < nThings; i++)
                          {
                            Cmp = (Compartment *) ThingList [i];
                            ThingList [i] = (void *) &(Cmp->TotalChannelCurrent);
                          }

                          Func = &Report::GetDblPtrVal;
        break;
    case CHANNEL_RPT:     GetChanPtrs (iRep, RN);
                          Func = &Report::GetChanValues;
        break;
    case SYNAPSE_USE:     GetSynPtrs (iRep, RN);
                          for (i = 0; i < nThings; i++)
                          {
                            Syn = (Synapse *) ThingList [i];
                            ThingList [i] = (void *) &(Syn->USE);
                          }
                          Func = &Report::GetFltPtrVal;
        break;
    case SYNAPSE_RSE:     GetSynPtrs (iRep, RN);
                          for (i = 0; i < nThings; i++)
                          {
                            Syn = (Synapse *) ThingList [i];
                            ThingList [i] = (void *) &(Syn->RSE_New);
                          }
                          Func = &Report::GetFltPtrVal;
        break;
    case SYNAPSE_UF:      GetSynPtrs( iRep, RN );                     //report on facilitation modifier
                          for( i=0; i<nThings; i++ )
                          {
                            Syn = (Synapse *) ThingList [i];
                            ThingList [i] = (void *) &(Syn->USE_New);
                          }
                          Func = &Report::GetFltPtrVal;
                          break;
    case SYNAPSE_SA:
                          GetSynPtrs( iRep, RN );

                          Func = &Report::GetSynapseAugmentation;

                          //confirm Augmentation object exists
                          for( i=0; i<nThings; i++ )
                          {
                            Syn = (Synapse *) ThingList [i];
                            if( !Syn->synAugmentation )
                              Func = &Report::EmptyFunc;
                          }
                          break;
     case SYNAPSE_CA:
                          GetSynPtrs( iRep, RN );

                          Func = &Report::GetSynapseAugmentation;
                          for( i=0; i<nThings; i++ ) //confirm SA object exists
                          {
                            Syn = (Synapse *) ThingList [i];
                            if( !Syn->synAugmentation )
                              Func = &Report::EmptyFunc;
                          }
                          break;
     case FIRE_COUNT:
                          GetCmpPtrs (iRep, RN);

                          //find the spikeshape the compartment uses,
                          //determine the maximum value (i.e. the peak of the spike)
                          if( nThings > 0 )
                          {
                            spikeMax = 0;  //The spike shape should peak
                                               //well above zero for biological realism
                            for (i = 0; i < ((Compartment*) ThingList[0])->nSpikeShape; i++)
                            {
                              if( ((Compartment*)ThingList[0])->SpikeShape[i] > spikeMax )
                                spikeMax = ((Compartment*)ThingList[0])->SpikeShape[i];
                            }
                          }
                          for (i = 0; i < nThings; i++)
                          {
                            Cmp = (Compartment *) ThingList [i];
                            ThingList [i] = (void *) &(Cmp->VoltageMembrane);
                          }
                          Func = &Report::GetFireCount;
                          break;
     default:              Func = &Report::EmptyFunc;
        break;
  }
}

//------------------------------------------------------------------

int Report::getFamilyCount()
{
  int nitems = 0;

  switch( family )
  {
        case FAM_NA:
            break;
        case FAM_CA:
            break;
        case FAM_KM:
                        nitems = 5;
            break;
        case FAM_KA:    
                        nitems = 8;
            break;
        case FAM_KDR:
            break;
        case FAM_KNICD:
            break;
        case FAM_KAHP:  
                        nitems = 7;
            break;
  }
  return nitems;
}

/*---------------------------------------------------------------------------*/
/* Returns an array containing the addresses of the compartments that this   */
/* report will report on, or NULL if the cell cluster is not on this node.   */

void Report::GetCmpPtrs (T_REPORT *iRep, Random *RN)
{
  Cell *Cel;
  double prob;
  int i, j, nCells, *CellIDs;

/* First step finds the cells.  If reporting probability is < 1, cells are   */
/* deleted from list according to random test.                               */

  CellIDs = CellMgr->ListCellsOnNode (iRep->Column, iRep->Layer, iRep->Cell, &nCells);

  if (CellIDs != NULL)
  {
    if (iRep->Prob < 1.0)           /* not all cells will be reported */
    {
        if( iRep->Seed == SELECT_FRONT )      //just get the front cells
            nCells = (int) (nCells*iRep->Prob);
        else                                  //randomly select cells
        {
            for (i = j = 0; i < nCells; i++)
            {                                    
                prob = RN->Rand ();
                if (prob <= iRep->Prob)     /* remove cell if prob > specified */
                CellIDs [j++] = CellIDs [i];
            }  
            nCells = j;
        }
    }

/* Now get the specific compartment within each Cell, and put its address in */
/* the ThingList.                                                            */
    if (nCells > 0)
    {
      nThings   = nCells;
      ThingList = (void **) malloc (nThings * sizeof (void *));
      if (ThingList == NULL) 
        MemError ("Report::GetCmpPtrs");

      for (i = 0; i < nThings; i++)
      {
        Cel = TheBrain->Cells [CellIDs [i]];
        ThingList [i] = (void *) Cel->Compartments [iRep->Cmp];
      }
    }
    free (CellIDs);
  }  

}

/*---------------------------------------------------------------------------*/
/* Returns an array containing the addresses of the channels that this       */
/* report will report on, or NULL if the cell cluster is not on this node.   */

void Report::GetChanPtrs (T_REPORT *iRep, Random *RN)
{
  Cell *Cel;
  Compartment *Cmp;
  double prob;
  int i, j, iChan, nCells, *CellIDs;

  CellIDs = CellMgr->ListCellsOnNode (iRep->Column, iRep->Layer, iRep->Cell, &nCells);

  if (nCells > 0)
  {
    if (iRep->Prob < 1.0)           /* not all cells will be reported */
    {
        if( iRep->Seed == SELECT_FRONT )        //just want the first cells
            nCells = (int) (nCells*iRep->Prob);
        else                                    //randomly select cells
        {
            for (i = j = 0; i < nCells; i++)
            {
                prob = RN->Rand ();
                if (prob <= iRep->Prob)     /* remove cell if prob > specified */
                    CellIDs [j++] = CellIDs [i];
            }
            if (nCells > j) nCells = j;
        }
    }

/* Now get the address of the specific channel within the specified          */
/* compartment of each Cell, and put its address in the ThingList.  First    */
/* test one cell (since they're all identical), to find the index of the     */
/* particular channel needed.  This must be done by checking the index in    */
/* the definition, since a compartment may have several different channels   */
/* of the same family.                                                       */

    if (nCells > 0)
    {
      Cmp   = TheBrain->Cells [CellIDs [0]]->Compartments [0];
      iChan = -1;
      for (i = 0; i < Cmp->nChannels; i++)
      {
        if (Cmp->vChannels [i]->source->L.idx == iRep->Thing)
        {
          family = Cmp->vChannels [i]->family;
          iChan  = i;
          break;
        }
      }
      if (iChan < 0)
      {
        printf ("Report::GetChanPtrs: Error in '%s' channel '%s' not found\n",
                iRep->L.name, iRep->Name);
        fprintf (stderr, "Report::GetChanPtrs: Error in '%s', channel '%s' not found\n",
                iRep->L.name, iRep->Name);
      }
      else
      {
        ThingSize = sizeof (float) * Cmp->vChannels [iChan]->GetNumItems ();
  
        nThings   = nCells;
        ThingList = (void **) malloc (nThings * sizeof (void *));
        if (ThingList == NULL) 
          MemError ("Report::GetChanPtrs");
  
        for (i = 0; i < nThings; i++)
        {
          Cel = TheBrain->Cells [CellIDs [i]];
          ThingList [i] = (void *) Cel->Compartments [iRep->Cmp];
        }
      }
    }
  }
}

/*---------------------------------------------------------------------------*/
/* Returns an array containing the addresses of the synapses that this       */
/* report will report on, or NULL if the cell cluster is not on this node.   */

void Report::GetSynPtrs (T_REPORT *iRep, Random *RN)
{
    Compartment *Cmp;
    int i, j, nCells, nSyn, *CellIDs;

    /* Start by getting a list of the cells on the receiving end of the synapse */

    CellIDs = CellMgr->ListCellsOnNode (iRep->Column, iRep->Layer, iRep->Cell, &nCells);

    if( nCells <= 0 )
        return;

    if (iRep->Prob < 1.0)           /* not all cells will be reported */
    {
        if( iRep->Seed == SELECT_FRONT )          //just select front cells
            nCells = (int) (nCells*iRep->Prob);
        else                                      //select cells randomly
        {
            for (i = j = 0; i < nCells; i++)
                if (RN->Rand() <= iRep->Prob)
                    CellIDs [j++] = CellIDs [i];  //overwrite pointers
            if (nCells > j) nCells = j;
        }
    }

    if( nCells <= 0 ) //Probability eliminated all cells - free list and return
    {
        free( CellIDs );
        return;
    }

      //since we are averging synapses, we need to know how many belong to each cell
    if( reportFlag & AVERAGE_SYN )
      synapseCounts = (int*) malloc ( sizeof(int) * nCells );

    /* Now go through all the synapses for each selected cell, checking for ones that     */
    /* match the specified SYNAPSE name */

    nSyn = 0;

    for (i = 0; i < nCells; i++)     /* First count the number */
    {
      Cmp = TheBrain->Cells [CellIDs [i]]->Compartments [iRep->Cmp]; //should not assume cmp 0!
      if( reportFlag & AVERAGE_SYN )
        synapseCounts[i] = 0;

      for (j = 0; j < Cmp->nSynapse; j++)
      {
        if (Cmp->SynapseList [j]->SynDef->source->L.idx == iRep->Thing)
        {
          nSyn++;
          if( reportFlag & AVERAGE_SYN )
            synapseCounts[i]++;
        }
      }
    }

    if( nSyn == 0 )
    {
      free( CellIDs );
      free( synapseCounts );
      return;
    }

    /* Now fill in the ThingList pointers and header info */
    if (nSyn > 0)
    {
        nThings = nSyn;
        ThingList = (void **) malloc (nThings * sizeof (void *));
        if (ThingList == NULL)
            MemError ("Report::GetSynPtrs");

        nSyn = 0;
        for (i = 0; i < nCells; i++)
        {
            Cmp = TheBrain->Cells [CellIDs [i]]->Compartments [iRep->Cmp];

            //go through compartment's synapses, pushing all matching synapses onto the list
            for (j = 0; j < Cmp->nSynapse; j++)
                if (Cmp->SynapseList[j]->SynDef->source->L.idx == iRep->Thing )
                {
                    ThingList [nSyn] = (void *) Cmp->SynapseList[j];
                    nSyn++;
                }
        }
    }

    if( reportFlag & AVERAGE_SYN )
      nThings = nCells;

    free (CellIDs);
}

/*---------------------------------------------------------------------------*/
/* This function is called to do the reporting.  It's called once per        */
/* timestep for each active report.  It checks whether the timestep is       */
/* within one of the report's active periods, and if so, calls functions to  */
/* flush previous output, collect the data for this cycle, and start         */
/* outputting it.  Actual output is done by another module, but will         */
/* conceptually be asynchronous, e.g.  MPI non-blocking sends.               */

bool Report::DoReport (int TimeStep)  
{
  bool done;
  currentTime = TimeStep;

  QQStateOn (QQReport);
  done = false;
  if ((TimeStep >= TStart [iTime]) && (TimeStep <= TStop [iTime]))
  {
    if (TimeStep == TStart [iTime]) 
    {
      Out->OpenFile ("a");
    }

    if (FreqCounter == 0)
    {
      if (Started) 
        Out->FinishOutput (RptBuf);    /* Finish previous output */

      (this->*Func)();                        /* Get stuff to output */

      Out->StartOutput (TimeStep, RptBuf);    /* Start the output */
  
      Started = true;
      FreqCounter = Freq;
    }
    FreqCounter--;
    if (TimeStep == TStop [iTime]) 
    {
      iTime++;
      if (iTime >= nTimes) done = true;
      if (Started) Out->FinishOutput (RptBuf);   /* Pause in reporting, so flush MPI send */
      Started = false;
      Out->CloseFile ();
    }
  }
  QQStateOff (QQReport);
  return (done);
}   

/*---------------------------------------------------------------------------*/
/* The following routines are the ones that actually collect the data.  The  */
/* appropriate one for the report will be assigned to the func pointer.      */

/* This is used for most reports.  Just copies double values from pointers   */

void Report::GetDblPtrVal ()
{
  float *ptr;
  int i;

  ptr = (float *) RptBuf;
  for (i = 0; i < nThings; i++)
  {
    *ptr = (float) *((double *) ThingList [i]);
    ptr++;
  }
}

/*---------------------------------------------------------------------------*/
/* Synapse values are stored as floats if the USE_FLOAT flag is set, so need */
/* additional function.                                                      */

void Report::GetFltPtrVal ()
{
  float *ptr;
  float sum = 0.0;
  int i;

  ptr = (float *) RptBuf;

  if( reportFlag & AVERAGE_SYN )
  {
    for( i=0; i<nThings; i++ )
    {
      for( int j=0; j<synapseCounts[i]; j++ )
      {
        sum += *((float *) ThingList [i]);
      }
      *ptr = sum/synapseCounts[i];
      sum = 0.0;
      ptr++;
    }
  }
  else
  {
    for (i = 0; i < nThings; i++)
    {
#ifdef USE_FLOAT
      *ptr = *((float *) ThingList [i]);
#else
      *ptr = (float) *((double *) ThingList [i]);
#endif
      ptr++;
    }
  }
}

//------------------------------------------------------------------------------
/* Special function to make sure syanpse updates internal values if no spikes have arrived */

void Report::GetSynapseAugmentation ()
{
  //When this function is called, it will see if the synapse has updated itself
  //(updates happen when a spike arrives).  If not, it will update the synapse
  //so decay patterns can be observed.

  float *ptr = (float*) RptBuf;
  float sum = 0.0;
  int index = 0;

  if( reportFlag & AVERAGE_SYN )
  {
    for( int j=0; j<nThings; j++ )
    {
      for( int i=0; i<synapseCounts[j]; i++, index++ )
      {
        ((Synapse *) ThingList[index])->synAugmentation->update( currentTime );
        if( ReportOn == SYNAPSE_SA )
          sum += (float) ((Synapse*) ThingList[index])->synAugmentation->augmentation;
        else if( ReportOn == SYNAPSE_CA )
          sum += (float) ((Synapse*) ThingList[index])->synAugmentation->Calcium;
      }

      *ptr = sum / synapseCounts[j];
      ptr++;
      sum = 0.0;
    }
  }
  else
  {
    for( int i=0; i<nThings; i++ )
    {
      ((Synapse *) ThingList[i])->synAugmentation->update( currentTime );

      if( ReportOn == SYNAPSE_SA )
        *ptr = (float) ((Synapse*) ThingList[i])->synAugmentation->augmentation;
      else if( ReportOn == SYNAPSE_CA )
        *ptr = (float) ((Synapse*) ThingList[i])->synAugmentation->Calcium;
      else
        *ptr = 0.0;

      ptr++;
    }
  }
}

/*---------------------------------------------------------------------------*/
/* This has to go through a layer of indirection, because what's reported    */
/* for the channels isn't simple values, but new values computed from the    */
/* channel state.                                                            */

void Report::GetChanValues ()
{
  Compartment *Cmp;
  float *ptr;
  int i, nitems;

  ptr = (float *) RptBuf;
  for (i = 0; i < nThings; i++)
  {
    Cmp = (Compartment *) ThingList [i];
    Cmp->ReportChannel (family, ptr, &nitems);
    ptr += nitems;
  }
}

/*---------------------------------------------------------------------------*/


void Report::GetFireCount()
{
  //go through the compartment Membrane Voltages, counting all compartments
  // that have successfully fired
  float* ptr = (float*) RptBuf;
  *ptr = 0;
  for( int i=0; i<nThings; i++ )
    if( *((double*) ThingList[i]) == spikeMax )
      (*ptr)++;
}

/*---------------------------------------------------------------------------*/

void Report::EmptyFunc ()
{
  printf ("Report::EmptyFunc: Error, EmptyFunc called!\n");
}

/*-----------------------------------------------------------------------------

This module handles the gathering of data for a report.  A report may be
specified in the input file, or may be created dynamically.  (Report
creation and deletion are managed at a higher level, in DoReport.cpp.)
For a given report, a report object will be created on the node that has
the cells (or other objects) that are being reported on.  (They will be
on only one node.)

The Report constructor determines how much data is reported at each
timestep, and allocates a buffer to hold it.  It then collects the report
header information into the buffer, and calls Out->MakeRpt, passing in
the buffer, filename, and so on.  The constructor also adds the report
object into the Brain's list.

The actual reporting is handled by the DoReport function (not the same as
DoReport object!), which is (conceptually) called at each timestep.  It
determines if the timestep matches the time interval(s) and frequency for
the report.  If so, the Out->CompleteOutput function is called (to allow
e.g.  AIO functions to be used), then the report data is collected into
the buffer which is passed to Out->Output, and the checked to see if the
report is complete.  If so, the done flag set to true and returned to the
caller.

Thus at the end of the DoReport function, the buffer is full of data.  The
Output module has at least until the function is entered at the next
timestep to do the actually writing.  With luck, something like AIO will
allow this to be done in the background.

Data collection is now handled through function pointers, in order to
eliminate overhead and make the reporting as generic as possible.  Notice
a bit of trickiness here: for most reports, the report creation sets the
ThingList value to the address of the variable within the object, thus
saving the overhead of repeated indirection each time the reporting
function is called.  However, this can't be done for e.g.  channel
reports, as the reported values are computed 'on the fly'.  For these,
the ThingList contains a pointer to the object, and the reporting
function calls a function within the object to compute the required
values.  The function pointer method allows either way to be used.

To create a new type of report, add code to the "GetThingsToReport"
function that specifies what to report, computes how big the report
buffer should be, and creates any header for the report.  Then add a
function that collects the report data into RptBuf, and set the function
pointer to that function.

The design also allows for reports to be created "on the fly", if some
means of specifying them is added.

Note one important design difference from the old code: That allowed a
report object to report on a list of different things (supposedly, I
don't know that it ever worked).  For simplicity, a report now reports on
one "thing", though that thing may have multiple items per cell.  For
example, it would be easy to create a new report type that outputs both
compartment voltage and current, if that was ever wanted.

-----------------------------------------------------------------------------*/

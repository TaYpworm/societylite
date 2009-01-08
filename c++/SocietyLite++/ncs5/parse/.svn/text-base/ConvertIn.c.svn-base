/* After all the input structures have been read in, this goes through the   */
/* list, converting the linked lists of input to pointer arrays, changing    */
/* names of things to symbol table indexes, and otherwise converting         */
/* human-readable input values into actual values that will be used in the   */
/* program.                                                                  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../defines.h"
#include "../InitStruct.h"
#include "input.h"
#include "list.h"
#include "SymTree.h"
#include "arrays.h"
#include "proto.h"
#include "ReadFile.h"
#include "share.c"

extern TMP_INPUT *TIN;       /* This must be global so parse.y can access it */

ARRAYS *ConvertIn (TREENODE *SymTree)
{
  ARRAYS *AR;
  T_BRAIN *brain;

//if (TIN->Node == 0) printf ("\nparse/ConvertIn: starting conversion\n");

  TIN->nDataErr = 0;

  AR = (ARRAYS *) malloc (sizeof (ARRAYS));

  //This function is also used when loading a brain that has previously run.
  // When this load happens, the structures are not defined in the input file
  // but will be restored later.  (Why later? why havn't they been restored already?)
  // one issue is indexing.  I don't want to load everything then have parser objects
  // like symTree try to alter the indexing that was in use before.
  // Also, if the user attempts to leave the column/layer/etc. definitions in the
  // input file that is getting restored, I want them read, but ignored rather than
  // trying to sort out duplicates.

  brain     = TIN->Brain;
  AR->Brain = TIN->Brain;
  if( !AR->Brain->loadfile )  //Column are not defined in a loaded Brain
    AR->Brain->Columns  = ConvertList (brain->ColumnNames, brain->nColumns,  SymTree, "Column");
  AR->Brain->StInject = ConvertList (brain->StInjNames,  brain->nStInject, SymTree, "Stimulus Inject");
  AR->Brain->Reports  = ConvertList (brain->ReportNames, brain->nReports,  SymTree, "Report");

  if( !AR->Brain->loadfile )
  {
    ConvertCsh   (SymTree, AR);  
    ConvertCol   (SymTree, AR);  
    ConvertLsh   (SymTree, AR);  
    ConvertLay   (SymTree, AR);  
    ConvertCell  (SymTree, AR);  
    ConvertCmp   (SymTree, AR);  
    ConvertChan  (SymTree, AR);  

    ConvertSynFD (SymTree, AR);          /* Synapse components must be */
    ConvertSynLearn (SymTree, AR);       /* converted before synapses  */
    ConvertSynPSG  (SymTree, AR);
    ConvertSynData (SymTree, AR);
    ConvertSynAugmentation (SymTree, AR);
    ConvertSyn   (SymTree, AR);

    ConvertSpike (SymTree, AR);  
  }

  //These will always be converted (although names are looked up later)
  ConvertStim  (SymTree, AR);
  ConvertSti   (SymTree, AR);  
  ConvertRpt   (SymTree, AR);  

  if( !AR->Brain->loadfile )
  {
    ConvertConnect (AR, SymTree);
    ConvertRecurrent( AR, SymTree );
  
    CellCount (AR);
  }

  if (TIN->nDataErr > 0)
    printerr ("Parser: %d Input errors found.\n", TIN->nDataErr);

  return (AR);
}

/*----------------------------------------------------------------------------*/

void ConvertCsh (TREENODE *SymTree, ARRAYS *AR)
{
  T_CSHELL *csh;
  int i, idx;

  AR->nCsh = TIN->nCsh;
  if (TIN->nCsh <= 0)
  {
    printerr ("ConvertCsh: No column shells defined\n");
    AR->Csh = NULL;
    TIN->nDataErr++;
  }
  else
  {
    AR->Csh = (T_CSHELL **) calloc (AR->nCsh, sizeof (T_CSHELL *));
    csh = TIN->Csh;
    for (i = 0; i < AR->nCsh; i++)
    {
      idx = csh->L.idx;
      AR->Csh [idx] = csh;
      csh = csh->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertCol (TREENODE *SymTree, ARRAYS *AR)
{
  T_COLUMN *col;
  int i, idx;

  AR->nColumns = TIN->nColumn;
  if (TIN->nColumn <= 0)
  {
    printerr ("ConvertCol: No columns defined\n");
    AR->Columns = NULL;
    TIN->nDataErr++;
  }
  else
  {
    AR->Columns = (T_COLUMN **) calloc (AR->nColumns, sizeof (T_COLUMN *));
    col = TIN->Column;
    for (i = 0; i < AR->nColumns; i++)
    {
      idx = col->L.idx;
      AR->Columns [idx] = col;

      col->CShell = getName (&(col->L), col->shellName, SymTree, "Column Shell");
      col->Layers = ConvertList (col->LayerNames, col->nLayers, SymTree, "Layer");

      col = col->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertLsh (TREENODE *SymTree, ARRAYS *AR)
{
  T_LSHELL *lsh;
  int i, idx;

  AR->nLsh = TIN->nLsh;
  if (AR->nLsh <= 0)
  {
    printerr ("ConvertLsh: No layer shells defined\n");
    AR->Lsh = NULL;
    TIN->nDataErr++;
  }
  else
  {
    AR->Lsh = (T_LSHELL **) calloc (TIN->nLsh, sizeof (T_LSHELL*));
    lsh = TIN->Lsh;
    for (i = 0; i < AR->nLsh; i++)
    {
      idx = lsh->L.idx;
      AR->Lsh [idx] = lsh;
      lsh = lsh->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertLay (TREENODE *SymTree, ARRAYS *AR)
{
  T_LAYER *lay;
  int i, idx;

  AR->nLayers = TIN->nLayer;
  if (AR->nLayers <= 0)
  {
    printerr ("ConvertLay: No layers defined\n");
    AR->Layers = NULL;
    TIN->nDataErr++;
  }
  else
  {
    AR->Layers = (T_LAYER **) calloc (TIN->nLayer, sizeof (T_LAYER *));
    lay = TIN->Layer;
    for (i = 0; i < AR->nLayers; i++)
    {
      idx = lay->L.idx;
      AR->Layers [idx] = lay;
  
      lay->LShell = getName (&(lay->L), lay->shellName, SymTree, "Layer Shell");
  
      lay->CellTypes  = (int *) calloc (lay->nCellTypes, sizeof (int));
      lay->CellQty    = (int *) calloc (lay->nCellTypes, sizeof (int));
      ConvertCellList (lay->CellNames, lay->nCellTypes, SymTree, lay->CellTypes, lay->CellQty);

      lay = lay->L.next;
    }
  }
}

/*---------------------------------------------------------------------------*/

void ConvertCell (TREENODE *SymTree, ARRAYS *AR)
{
  T_CELL *cell;
  LIST *cmp;
  int i, j, idx;

  AR->nCells = TIN->nCell;
  if (AR->nCells <= 0)
  {
    printerr ("ConvertCell: No cells defined\n");
    AR->Cells = NULL;
    TIN->nDataErr++;
  }
  else
  {
    AR->Cells = (T_CELL **) calloc (AR->nCells, sizeof (T_CELL *));
    cell = TIN->Cell;
    
    for (i = 0; i < AR->nCells; i++)
    {
      idx = cell->L.idx;
      AR->Cells [idx] = cell;
  
      if (cell->nCmp <= 0)
      {
        printerr ("%s:%d: Error, Cell '%s' has no compartments\n",
                   cell->L.file, cell->L.line, cell->L.name);
        TIN->nParseErr++;
      }
      else
      {
        cell->Cmp    = (int *)    calloc (cell->nCmp, sizeof (int));
        cell->Labels = (char **)  calloc (cell->nCmp, sizeof (char *));
        cell->Rpos   = (double *) calloc (cell->nCmp, sizeof (double));
        cell->Xpos   = (double *) calloc (cell->nCmp, sizeof (double));
        cell->Ypos   = (double *) calloc (cell->nCmp, sizeof (double));
        cell->Zpos   = (double *) calloc (cell->nCmp, sizeof (double));
  
        cmp = cell->CmpNames;
        for (j = 0; j < cell->nCmp; j++)
        {
          cell->Cmp    [j] = getName (&(cmp->L), cmp->L.name, SymTree, "Compartment");
          cell->Labels [j] = cmp->label;
          cell->Rpos   [j] = 0;
          cell->Xpos   [j] = cmp->x;
          cell->Ypos   [j] = cmp->y;
          cell->Zpos   [j] = cmp->z;
          cmp = cmp->L.next;
        }
      }
      cell = cell->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertCmp (TREENODE *SymTree, ARRAYS *AR)
{
  T_CMP *cmp;
  int i, idx;

  AR->nCmp = TIN->nCmp;
  if (AR->nCmp <= 0)
  {
    printerr ("Error: No compartments defined in input\n");
    AR->Cmp = NULL;
    TIN->nDataErr++;
  }
  else
  {
    AR->Cmp = (T_CMP **) calloc (AR->nCmp, sizeof (T_CMP *));
    cmp = TIN->Cmp;
  
    for (i = 0; i < AR->nCmp; i++)
    {
      idx  = cmp->L.idx;
      AR->Cmp [idx] = cmp;
  
      cmp->Active = FALSE;
      if ((cmp->SpikeName != NULL) /*&& (cmp->Spike_HW [0] != 0.0)*/ && (cmp->Threshold [0]))  
        //spike halfwidth (Spike_HW) is no longer used, so it should not affect "active"
      {
        cmp->Active = TRUE;
        cmp->Spike  = getName (&(cmp->L), cmp->SpikeName, SymTree, "Spike");
      }
      cmp->Channels = ConvertList (cmp->ChannelNames, cmp->nChannels, SymTree, "Channel");
      cmp = cmp->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertChan (TREENODE *SymTree, ARRAYS *AR)
{
  T_CHANNEL *chan;
  int i, idx;

  AR->Channel  = NULL;
  AR->nChannel = TIN->nChannel;
  if (AR->nChannel > 0)
  {
    AR->Channel = (T_CHANNEL **) calloc (AR->nChannel, sizeof (T_CHANNEL *));

    chan = TIN->Channel;
    
    for (i = 0; i < AR->nChannel; i++)
    {
      idx = chan->L.idx;
      AR->Channel [idx] = chan;
  
      if (chan->family == FAM_KA)
      {
        if (chan->nValM != chan->nVoltM + 1)
        {
          printerr ("%s:%d: Error in Channel Ka '%s', nValM (%d) != NvolM (%d) + 1\n",
                  chan->L.file, chan->L.line, chan->L.name, chan->nValM, chan->nVoltM);
          TIN->nDataErr++;
        }
  
        if (chan->nValH != chan->nVoltH + 1)
        {
          printerr ("%s:%d: Error in Channel Ka '%s', nValM (%d) != NvolM (%d) + 1\n",
                  chan->L.file, chan->L.line, chan->L.name, chan->nValH, chan->nVoltH);
          TIN->nDataErr++;
        }
      }
      chan = chan->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertSyn (TREENODE *SymTree, ARRAYS *AR)
{
  T_SYNAPSE *syn;
  T_SYNDATA *sdt;
  int i, idx, DataIdx;

  AR->Synapse  = NULL;
  AR->nSynapse = TIN->nSynapse;
  if (AR->nSynapse > 0)
  {
    AR->Synapse = (T_SYNAPSE **) calloc (AR->nSynapse, sizeof (T_SYNAPSE *));
    syn = TIN->Synapse;
    
    for (i = 0; i < AR->nSynapse; i++)
    {
      idx = syn->L.idx;
      AR->Synapse [idx] = syn;
  
      if ((syn->SfdIdx = getName (&(syn->L), syn->SfdName, SymTree, "SYN_FD")) < 0)
      {
        printerr ("%s:%d: Error, SYN_FD '%s' not defined\n",
                  syn->L.file, syn->L.line, syn->SfdName);
        TIN->nDataErr++;
      }
      if ((syn->LearnIdx = getName (&(syn->L), syn->LearnName, SymTree, "SYN_LEARN")) < 0)
      {
        printerr ("%s:%d: Error, SYN_LEARN '%s' not defined\n",
                  syn->L.file, syn->L.line, syn->LearnName);
        TIN->nDataErr++;
      }

/* The Synapse Data element is deprecated.  If one is specified, give a      */
/* warning, and copy the vars to the synapse                                 */


      if (syn->DataName != NULL)
      {
        if ((DataIdx = getName (&(syn->L), syn->DataName, SymTree, "SYN_DATA")) < 0)
        {
          printerr ("%s:%d: Error, SYN_DATA '%s' specified, but not defined\n",
                    syn->L.file, syn->L.line, syn->DataName);
          TIN->nDataErr++;
        }
        else
        {
          sdt = AR->SynData [DataIdx];
          syn->MaxG[0] = sdt->MaxG[0];
          syn->MaxG[1] = sdt->MaxG[1];
          syn->SynDelay [0] = sdt->SynDelay [0];
          syn->SynDelay [1] = sdt->SynDelay [1];
          syn->SynRever [0] = sdt->SynRever [0];
          syn->SynRever [1] = sdt->SynRever [1];
        }
      }
      if ((syn->PsgIdx = getName (&(syn->L), syn->PsgName,SymTree, "SYN_PSG")) < 0)
      {
        printerr ("%s:%d: Error, SYN_PSG '%s' not defined\n",
                  syn->L.file, syn->L.line, syn->PsgName);
        TIN->nDataErr++;
      }
      if( syn->AugmentationName != NULL )
      {
        if( (syn->AugmentationIdx = getName( &syn->L, syn->AugmentationName, SymTree, "SYN_AUGMENTATION" )) < 0 )
        {
            printerr ("%s:%d: Error, SYN_AUGMENTATION '%s' not defined\n",
                  syn->L.file, syn->L.line, syn->AugmentationName);
            TIN->nDataErr++;
        }
      }

      if ((syn->SynDelay [0] < 0.0) || (syn->SynDelay [1] <= 0.0) ||
          (syn->SynDelay [1] < syn->SynDelay [0]))
      {
        printerr ("%s:%d: Error, SYNAPSE DELAY range error, values are %f, %f\n",
                  syn->L.file, syn->L.line, syn->SynDelay [0], syn->SynDelay [1]);
        TIN->nDataErr++;
      }
      syn = syn->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertSynLearn (TREENODE *SymTree, ARRAYS *AR)
{
  T_SYNLEARN *sln;
  int i, idx;

  AR->SynLearn  = NULL;
  AR->nSynLearn = TIN->nSynLearn;
  if (AR->nSynLearn > 0)
  {
    AR->SynLearn = (T_SYNLEARN **) calloc (AR->nSynLearn, sizeof (T_SYNLEARN *));
    sln = TIN->SynLearn;
    
    for (i = 0; i < AR->nSynLearn; i++)
    {
      idx = sln->L.idx;
      AR->SynLearn [idx] = sln;
      sln = sln->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertSynFD (TREENODE *SymTree, ARRAYS *AR)
{
  T_SYN_FD *sfd;
  int i, idx;

  AR->SynFD  = NULL;
  AR->nSynFD = TIN->nSynFD;
  if (AR->nSynFD > 0)
  {
    AR->SynFD = (T_SYN_FD **) calloc (AR->nSynFD, sizeof (T_SYN_FD *));

    sfd = TIN->SynFD;
    
    for (i = 0; i < AR->nSynFD; i++)
    {
      idx = sfd->L.idx;
      AR->SynFD [idx] = sfd;
  
      sfd = sfd->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertSynPSG (TREENODE *SymTree, ARRAYS *AR)
{
  T_SYNPSG *sp;
  int i, idx, nvals;

  AR->SynPSG  = NULL;
  AR->nSynPSG = TIN->nSynPSG;
  if (AR->nSynPSG > 0)
  {
    AR->SynPSG = (T_SYNPSG **) calloc (AR->nSynPSG, sizeof (T_SYNPSG *));

    sp = TIN->SynPSG;
    for (i = 0; i < AR->nSynPSG; i++)
    {
      idx = sp->L.idx;
      AR->SynPSG [idx] = sp;
  
      sp->PSG = ReadFPNFile (sp->File, TIN->Node, &nvals, 1);
      if (sp->PSG == NULL)
      {
        printerr ("%s:%d: Error, can't open PSG file '%s'\n", 
                  sp->L.file, sp->L.line, sp->File);
        TIN->nDataErr++;
      }
      else
      {
        sp->nPSG = nvals;
      }
  
      sp = sp->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertSynData (TREENODE *SymTree, ARRAYS *AR)
{
  T_SYNDATA *sdt;
  int i, idx;

  AR->SynData  = NULL;
  AR->nSynData = TIN->nSynData;
  if (AR->nSynData > 0)
  {
    AR->SynData = (T_SYNDATA **) calloc (TIN->nSynData, sizeof (T_SYNDATA *));
    sdt = TIN->SynData;
    
    for (i = 0; i < AR->nSynData; i++)
    {
      idx = sdt->L.idx;
      AR->SynData [idx] = sdt;
  
      sdt = sdt->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertSynAugmentation (TREENODE *SymTree, ARRAYS *AR)
{
  T_SYNAUGMENTATION *saugmentation;
  int i, idx;

  AR->SynAugmentation  = NULL;
  AR->nSynAugmentation = TIN->nSynAugmentation;
  if (AR->nSynAugmentation > 0)
  {
    AR->SynAugmentation = (T_SYNAUGMENTATION **) calloc (AR->nSynAugmentation, sizeof (T_SYNAUGMENTATION *));

    saugmentation = TIN->SynAugmentation;
    
    for (i = 0; i < AR->nSynAugmentation; i++)
    {
      idx = saugmentation->L.idx;
      AR->SynAugmentation [idx] = saugmentation;

      if( saugmentation->MaxSA[0] < 1.0 )
      {
        printerr ( "%s:%d: Error, Max Augmentation may not be less than 1.0 ( '%f' input )\n",
                  saugmentation->L.file, saugmentation->L.line, saugmentation->MaxSA[0] );
        TIN->nDataErr++;
      }

      saugmentation = saugmentation->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertSpike (TREENODE *SymTree, ARRAYS *AR)
{
  T_SPIKE *spike;
  int i, idx;

  AR->Spike = NULL;
  AR->nSpike = TIN->nSpike;
  if (AR->nSpike > 0)
  {
    AR->Spike = (T_SPIKE **) calloc (AR->nSpike, sizeof (T_SPIKE  *));
    spike = TIN->Spike;
    
    for (i = 0; i < AR->nSpike; i++)
    {
      idx = spike->L.idx;
      AR->Spike [idx] = spike;
  
      spike = spike->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertStim (TREENODE *SymTree, ARRAYS *AR)
{
  T_STIMULUS *stim;
  int i, j, idx;

  AR->Stimulus = NULL;
  AR->nStimulus = TIN->nStimulus;
  if (AR->nStimulus > 0)
  {
    AR->Stimulus = (T_STIMULUS **) calloc (AR->nStimulus, sizeof (T_STIMULUS *));
    stim = TIN->Stimulus;
    
    for (i = 0; i < AR->nStimulus; i++)
    {
      idx  = stim->L.idx;
      AR->Stimulus [idx] = stim;

      if (stim->nTStart != stim->nTStop)
      {
        printerr ("%s:%d: Error, number of start & stop times differ (%d - %d\n",
                   stim->L.file, stim->L.line, stim->nTStart, stim->nTStop);
      }
      stim->nTimes = stim->nTStart;
      stim->Time_Start = (int *) calloc (2 * stim->nTimes, sizeof (int));
      stim->Time_Stop  = stim->Time_Start + stim->nTimes;
  
  /* Convert input start & stop times, which are real numbers (seconds) to     */
  /* integer tick counts.                                                      */
  
  // RPD:  had to add the + EPSILON during the float->double conversion to avoid having
  // end of stimulus change.  This was a weird problem.  Apparently the double representation
  // of, say, "1400" was very slightly *less* than the integer 1400 while the float representation
  // was very slight *more*.  This resulted in different integer values for stimulus end.
  
      for (j = 0; j < stim->nTimes; j++)
      {
        stim->Time_Start [j] = (int) (stim->TStart [j] * TIN->Brain->FSV + EPSILON);
        stim->Time_Stop  [j] = (int) (stim->TStop  [j] * TIN->Brain->FSV + EPSILON);
      }

      stim = stim->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertSti (TREENODE *SymTree, ARRAYS *AR)
{
  T_STINJECT *st;
  int i, idx;

  AR->StInject = NULL;
  AR->nStInject = TIN->nStInject;
  if (AR->nStInject > 0)
  {
    AR->StInject = (T_STINJECT **) calloc (AR->nStInject, sizeof (T_STINJECT *));
    st = TIN->StInject;
    
    for (i = 0; i < AR->nStInject; i++)
    {
      idx  = st->L.idx;
      AR->StInject [idx] = st;
 
      if( !AR->Brain->loadfile )  //Search for names as long as not loading a brain
      {
        st->STIM_TYPE = getName (&(st->L), st->StimName, SymTree, "Stimulus");
        st->Column    = getName (&(st->L), st->ColName,  SymTree, "Column");
        st->Layer     = getName (&(st->L), st->LayName,  SymTree, "Layer");
        st->Cell      = getName (&(st->L), st->CellName, SymTree, "Cell");
        st->Cmp       = getCmpIdx (st->CellName, st->CmpName, SymTree, &(st->L));
      }
  
      st = st->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void ConvertRpt (TREENODE *SymTree, ARRAYS *AR)
{
  T_REPORT *rpt;
  int i, j, idx;

  AR->Reports = NULL;
  AR->nReports = TIN->nReport;
  if (AR->nReports > 0)
  {
    AR->Reports = (T_REPORT **) calloc (AR->nReports, sizeof (T_REPORT *));
    rpt = TIN->Report;
    
    for (i = 0; i < AR->nReports; i++)
    {
      idx  = rpt->L.idx;
      AR->Reports [idx] = rpt;
  
      rpt->Column = getName (&(rpt->L), rpt->ColName,  SymTree, "Column");
      rpt->Layer  = getName (&(rpt->L), rpt->LayName,  SymTree, "Layer");
      rpt->Cell   = getName (&(rpt->L), rpt->CellName, SymTree, "Cell");
      rpt->Cmp    = getCmpIdx (rpt->CellName, rpt->CmpName, SymTree, &(rpt->L));
  
      rpt->Thing  = -1;
      if (rpt->ReportOn == SYNAPSE_USE || rpt->ReportOn == SYNAPSE_RSE ||
          rpt->ReportOn == SYNAPSE_UF  || rpt->ReportOn == SYNAPSE_SA ||
          rpt->ReportOn == SYNAPSE_CA )
        rpt->Thing = getName (&(rpt->L), rpt->Name, SymTree, "Synapse");
      else if( rpt->ReportOn == FIRE_COUNT)
	rpt->reportFlag |= INTEGER_CAST;      
      else if (rpt->ReportOn == CHANNEL_RPT)
      {
        rpt->Thing = getName (&(rpt->L), rpt->Name, SymTree, "Channel");
//      printf ("parse/ConvertIn.c, Channel rpt, name = '%s', thing = %d\n", rpt->Name, rpt->Thing);
      }
  
      if (rpt->nTStart != rpt->nTStop)
      {
        printerr ("%s:%d: Error, number of start & stop times differ (%d - %d\n",
                  rpt->L.file, rpt->L.line, rpt->nTStart, rpt->nTStop);
      }
      rpt->nTimes     = rpt->nTStart;
      rpt->Time_Start = (int *) calloc (2 * rpt->nTimes, sizeof (int));
      rpt->Time_Stop  = rpt->Time_Start + rpt->nTimes;
  
      for (j = 0; j < rpt->nTimes; j++)
      {
        rpt->Time_Start [j] = (int) (rpt->TStart [j] * TIN->Brain->FSV + EPSILON);
        rpt->Time_Stop  [j] = (int) (rpt->TStop  [j] * TIN->Brain->FSV + EPSILON);
      }
  
      rpt = rpt->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/
/* Converts linked list of names to integer array of symbol table indexes     */

int *ConvertList (LIST *List, int num, TREENODE *SymTree, char *tag)
{
  int i, index, *array;

  if (num <= 0)
    array = NULL;
  else
  {
    array = (int *) calloc (num, sizeof (int));
  
    for (i = 0; i < num; i++)
    {
      index = getTreeIndex (List->L.name, SymTree);
      if (index < 0)
      {
        printerr ("%s:%d: Error, %s '%s' is used, but not defined\n",
                  List->L.file, List->L.line, tag, List->L.name);
        TIN->nDataErr++;
      }
      array [i] = index;
      List = List->L.next;
    }
  }
  return (array);
}

/*----------------------------------------------------------------------------*/

void ConvertCellList (LIST *List, int num, TREENODE *SymTree, int *type, int *qty)
{
  int i, index;

  for (i = 0; i < num; i++)
  {
    index = getTreeIndex (List->L.name, SymTree);
    if (index < 0)
    {
      printerr ("%s:%d: Error, Cell type '%s' is used, but not defined\n",
                List->L.file, List->L.line, List->L.name);
      TIN->nDataErr++;
    }
    type [i] = index;
    qty  [i] = List->Qty;
    List = List->L.next;
  }
}

/*---------------------------------------------------------------------------*/
/* After the first phase of conversion, need to go through and convert the   */
/* connection lists.  This can't be done in the main conversion, because the */
/* indexes aren't fully determined until all the Columns, Layers, and cells  */
/* have been converted.  In the process, the original linked lists of        */
/* connect structs are converted to arrays, though the same pointer in the   */
/* T_* struct is used.                                                       */

void ConvertConnect (ARRAYS *AR, TREENODE *SymTree)
{
  T_BRAIN   *Brain;
  T_COLUMN  *Col;
  T_LAYER   *Lay;
  T_CELL    *Cel;
  T_CONNECT *con;
  T_CMPCONNECT *cmpc;
  int i, j, k, n, idx, iCol, iLay, iCel;

  Brain = AR->Brain;
//printf ("ConvertConnect: Brain, %d connects\n", Brain->nConnect);
  if (Brain->nConnect > 0)
  {
    con = Brain->CnList;
    Brain->Connect = (T_CONNECT **) calloc (Brain->nConnect, sizeof (T_CONNECT *));

    for (i = 0; i < Brain->nConnect; i++)      
    {
      idx = con->L.idx;
      Brain->Connect [idx] = con;

      con->FromCol  = getName (&(con->L), con->fromColName,  SymTree, "Column");
      con->FromLay  = getName (&(con->L), con->fromLayName,  SymTree, "Layer");
      con->FromCell = getName (&(con->L), con->fromCellName, SymTree, "Cell");
      con->FromCmp  = getCmpIdx (con->fromCellName, con->fromCmpName, SymTree, &(con->L));
  
      con->ToCol    = getName (&(con->L), con->toColName,  SymTree, "Column");
      con->ToLay    = getName (&(con->L), con->toLayName,  SymTree, "Layer");
      con->ToCell   = getName (&(con->L), con->toCellName, SymTree, "Cell");
      con->ToCmp    = getCmpIdx (con->toCellName, con->toCmpName, SymTree, &(con->L));
  
      con->SynType  = getName (&(con->L), con->SynName,   SymTree, "Synapse");
  
      con = con->L.next;
    }
  }

//printf ("\n %d columns\n", Brain->nColumns);
  for (i = 0; i < Brain->nColumns; i++)
  {
    iCol = Brain->Columns [i];
    Col = AR->Columns [iCol];

    if (Col->nConnect > 0)
    {
//    printf ("%4d: %s, %d connects\n", iCol, Col->L.name, Col->nConnect);
      con = Col->CnList;
      Col->Connect = (T_CONNECT **) calloc (Col->nConnect, sizeof (T_CONNECT *));
  
      for (j = 0; j < Col->nConnect; j++)      /* Do for each brain-level connect */
      {
        idx = con->L.idx;
        Col->Connect [idx] = con;

        con->FromCol  = con->ToCol = iCol;
  
        con->FromLay  = getName (&(con->L), con->fromLayName,  SymTree, "Layer");
        con->FromCell = getName (&(con->L), con->fromCellName, SymTree, "Cell");
        con->FromCmp  = getCmpIdx (con->fromCellName, con->fromCmpName, SymTree, &(con->L));
  
        con->ToLay    = getName (&(con->L), con->toLayName,  SymTree, "Layer");
        con->ToCell   = getName (&(con->L), con->toCellName, SymTree, "Cell");
        con->ToCmp    = getCmpIdx (con->toCellName, con->toCmpName, SymTree, &(con->L));
  
        con->SynType  = getName (&(con->L), con->SynName, SymTree, "Synapse");
  
        con = con->L.next;
      }
    }

//  printf ("\n   %d layers\n", Col->nLayers);
    for (j = 0; j < Col->nLayers; j++)
    {
      iLay = Col->Layers [j];
      Lay = AR->Layers [iLay];

      if (Lay->nConnect > 0)
      {
//      printf ("%8d: %s, %d connects\n", iLay, Lay->L.name, Lay->nConnect);
        con = Lay->CnList;
        Lay->Connect = (T_CONNECT **) calloc (Lay->nConnect, sizeof (T_CONNECT *));

        for (k = 0; k < Lay->nConnect; k++) 
        {
          idx = con->L.idx;
          Lay->Connect [idx] = con;
//        printf ("        %d: idx = %d, con = %x\n", k, idx, con);

          con->FromCol  = con->ToCol  = iCol;
          con->FromLay  = con->ToLay  = iLay;
  
          con->FromCell = getName (&(con->L), con->fromCellName, SymTree, "Cell");
          con->FromCmp  = getCmpIdx (con->fromCellName, con->fromCmpName, SymTree, &(con->L));
  
          con->ToCell   = getName (&(con->L), con->toCellName, SymTree, "Cell");
          con->ToCmp    = getCmpIdx (con->toCellName,   con->toCmpName,   SymTree, &(con->L));
  
          con->SynType  = getName (&(con->L), con->SynName, SymTree, "Synapse");
//        printf ("        (%d, %d, %d) -> (%d, %d, %d)\n", iCol, iLay, con->FromCell,
//                                                          iCol, iLay, con->ToCell);
          con = con->L.next;
        }
      }

      if (Lay->nCellTypes > 0)                  //Handle Cells
      {
        for (k = 0; k < Lay->nCellTypes; k++)
        {
          iCel = Lay->CellTypes [k];
          Cel  = AR->Cells [iCel];
          if (Cel->nConnect > 0)                //Compartment connections present
          {
            cmpc = Cel->CnList;
            Cel->Connect = (T_CMPCONNECT **) calloc (Cel->nConnect, sizeof (T_CMPCONNECT *));
            for (n = 0; n < Cel->nConnect; n++)
            {
              //idx = cmpc->L.idx;
              idx = n;
              Cel->Connect [idx] = cmpc;

              cmpc->FromCmp = getCmpIdx (Cel->L.name, cmpc->fromCmpName, SymTree, &(cmpc->L));
              cmpc->ToCmp   = getCmpIdx (Cel->L.name, cmpc->toCmpName,   SymTree, &(cmpc->L));
  
              cmpc = cmpc->L.next;
            }

            //If a Cell includes any compartment connections, they will
            //be configured in this function
            buildCellCmpConnects( Cel );
          } 
        }
      }
    }
  }
}

//--------------------------------------------------------------------------

int matchCheck( T_CONNECT *iCon, int aIndex[4], int bIndex[4], int synapseIndex )
{
    //For column and layer, if the aIndex value is -1, it can be ignored
    if( aIndex[0] > -1 )
    {
        if( iCon->FromCol != aIndex[0] || iCon->ToCol != bIndex[0] )
            return 0;
    }
    if( aIndex[1] > -1 )
    {
        if( iCon->FromLay != aIndex[1] || iCon->ToLay != bIndex[1] )
            return 0;
    }
    
    //cell, compartment, and synapse are always checked
    if( iCon->FromCell == aIndex[2] && iCon->FromCmp == aIndex[3] &&
        iCon->ToCell == bIndex[2] && iCon->ToCmp == bIndex[3] &&
        iCon->SynType == synapseIndex 
      )
        return 1;
    return 0;
}

//--------------------------------------------------------------------------

void gatherRecurrentInfo( int aIndex[4], int bIndex[4], int *synapseIndex, T_RECURRENT *con, TREENODE *SymTree, int depth )
{
    if( depth > 3 )  //need column info
    {
        aIndex[0] = getName( &(con->L), con->colNameA, SymTree, "column" );
        bIndex[0] = getName( &(con->L), con->colNameB, SymTree, "column" );
    }
    else
        aIndex[0] = bIndex[0] = -1;
    
    if( depth > 2 )  //need layer info
    {
        aIndex[1] = getName( &(con->L), con->layNameA, SymTree, "layer" );
        bIndex[1] = getName( &(con->L), con->layNameB, SymTree, "layer" );
    }
    else
        aIndex[1] = bIndex[1] = -1;
        
    //always need cell, compartment, and synapse info
    aIndex[2] = getName( &(con->L), con->cellNameA, SymTree, "cell" );
    aIndex[3] = getCmpIdx( con->cellNameA, con->cmpNameA, SymTree, &(con->L) );
    bIndex[2] = getName( &(con->L), con->cellNameB, SymTree, "cell" );
    bIndex[3] = getCmpIdx( con->cellNameB, con->cmpNameB, SymTree, &(con->L) );
    *synapseIndex  = getName (&(con->L), con->synName, SymTree, "Synapse");
}

//--------------------------------------------------------------------------

void outputRecurrenceError( T_RECURRENT *con, T_CONNECT *conAtoB, T_CONNECT *conBtoA )
{
    if( !conAtoB )
    {
        printerr ("%s:%d: Error: Failed to create recurrent connection.\n", con->L.file, con->L.line );
        printerr ("Could not find regular connection from group A (%s %s %s %s) to group B (%s %s %s %s)\n",
                    con->colNameA, con->layNameA, con->cellNameA, con->cmpNameA, 
                    con->colNameB, con->layNameB, con->cellNameB, con->cmpNameB );
        printerr ("using synapse %s\n", con->synName );
        TIN->nDataErr++;
    }
    if( !conBtoA )
    {
        printerr ("%s:%d: Error: Failed to create recurrent connection.\n", con->L.file, con->L.line );
        printerr ("Could not find regular connection from group B (%s %s %s %s) to group A (%s %s %s %s)\n",
                    con->colNameB, con->layNameB, con->cellNameB, con->cmpNameB, 
                    con->colNameA, con->layNameA, con->cellNameA, con->cmpNameA );
        printerr ("using synapse %s\n", con->synName );
        TIN->nDataErr++;
    }
}

//--------------------------------------------------------------------------

void setupRecurrence( T_RECURRENT *con, T_CONNECT *conAtoB, T_CONNECT *conBtoA )
{
    //be able to access info about conBtoA from within conAtoB
    conAtoB->recurrentConnection = conBtoA;
    conAtoB->recurrentProbability[0] = con->PrecurrenceAtoB;
    conAtoB->recurrentProbability[1] = con->PrecurrenceBtoA;

    //disable conBtoA if they are not the same connection
    //i.e. Don't disable a self-reciprocal cell group 
    if( conAtoB != conBtoA )
        conBtoA->disabled = 1;
}
              
//--------------------------------------------------------------------------

void ConvertRecurrent( ARRAYS *AR, TREENODE *SymTree )
{
    int i, j, k, n, indexCol, indexLay;
    int aIndex[4], bIndex[4];
    int synapseIndex;
    T_RECURRENT *con;
    T_COLUMN *Col; T_LAYER *Lay; T_BRAIN *Brain;
    T_CONNECT *conAtoB, *conBtoA;

    //go through the recurrent connections for each container

    Brain = AR->Brain;

    //Brain level
    con = Brain->recurrentList;

    for( k=0; k<Brain->nRecurrent; k++ )
    {
        gatherRecurrentInfo( aIndex, bIndex, &synapseIndex, con, SymTree, 4 );
        
        //are there connections matching?
        conAtoB = conBtoA = NULL;
        for( n=0; n<Brain->nConnect; n++ )
        {
            if( matchCheck( Brain->Connect[n], aIndex, bIndex, synapseIndex ))
                conAtoB = Brain->Connect[n];
            if( matchCheck( Brain->Connect[n], bIndex, aIndex, synapseIndex ))
                conBtoA = Brain->Connect[n];
        }

        if( conAtoB && conBtoA )
            setupRecurrence( con, conAtoB, conBtoA );
        else                                            //display errors if necessary
            outputRecurrenceError( con, conAtoB, conBtoA );
        
        con = con->L.next;
    }

    //for each column
    for (i = 0; i < Brain->nColumns; i++)
    {
        indexCol = Brain->Columns [i];
        Col = AR->Columns [indexCol];

        con = Col->recurrentList;

        for( k=0; k<Col->nRecurrent; k++ )
        {
            gatherRecurrentInfo( aIndex, bIndex, &synapseIndex, con, SymTree, 3 );

            //are there connections matching?
            conAtoB = conBtoA = NULL;
            for( n=0; n<Col->nConnect; n++ )
            {
                if( matchCheck( Col->Connect[n], aIndex, bIndex, synapseIndex ))
                    conAtoB = Col->Connect[n];
                if( matchCheck( Col->Connect[n], bIndex, aIndex, synapseIndex ))
                    conBtoA = Col->Connect[n];
            }

            if( conAtoB && conBtoA )
                setupRecurrence( con, conAtoB, conBtoA );
            else        //display errors if necessary
                outputRecurrenceError( con, conAtoB, conBtoA );

            con = con->L.next;
        }
        
        //for each layer
        for (j = 0; j < Col->nLayers; j++)
        {
            indexLay = Col->Layers [j];
            Lay = AR->Layers [indexLay];

            //has this layer been handled during a previous conversion?
            //does it have any recurrent connections at all?
            if( Lay->recurrentHandled || Lay->nRecurrent == 0)
                continue;
            
            Lay->recurrentHandled = 1;
            con = Lay->recurrentList;

            for( k=0; k<Lay->nRecurrent; k++ )
            {
                gatherRecurrentInfo( aIndex, bIndex, &synapseIndex, con, SymTree, 2 );

                //are there connections matching?
                conAtoB = conBtoA = NULL;
                for( n=0; n<Lay->nConnect; n++ )
                {
                    if( matchCheck( Lay->Connect[n], aIndex, bIndex, synapseIndex ))
                        conAtoB = Lay->Connect[n];
                    if( matchCheck( Lay->Connect[n], aIndex, bIndex, synapseIndex ))
                        conBtoA = Lay->Connect[n];
                }

                if( conAtoB && conBtoA )
                    setupRecurrence( con, conAtoB, conBtoA );
                else        //display errors if necessary
                    outputRecurrenceError( con, conAtoB, conBtoA );
                
                con = con->L.next;
            }
        }
    }
}

/*---------------------------------------------------------------------------*/
/* This needs to do a two-stage lookup, since the cmp isn't specified by its */
/* name, but by the label it was given in the cell definition.  This is done */
/* so that a cell can have multiple instances of the same type of            */
/* compartment.  Each instance (that is, each COMPARTMENT statement in the   */
/* CELL definition) has a label that must be different for each compartment  */
/* in that cell, though the same label may be used for compartments          */
/* different cells.)                                                         */

/* This routine first does a tree search to find the cell definition         */
/* matching CellName, then (since a cell isn't expected to have many         */
/* compartments yet) a linear search through the cell's list of compartments */
/* to match the label.                                                       */

int getCmpIdx (char *CellName, char *label, TREENODE *SymTree, LOCATOR *L)
{
  TREENODE *node;
  T_CELL *Cel;
  int i, idx;

  idx = -1;
  node = findTree (CellName, SymTree);         /* Find the cell's entry in tree... */
  if (node == NULL)
  {
    printerr ("%s:%d: Error: Cell name '%s' not defined.\n",
              L->file, L->line, CellName);
    TIN->nDataErr++;
  }
  else
  {
    Cel = (T_CELL *) node->data;           
    for (i = 0; i < Cel->nCmp; i++)
    {
      if (strcmp (label, Cel->Labels [i]) == 0)
      {
        idx = i;
        break;
      }
    }
  }
  if( idx == -1 ) //did not find Cell or Compartment
  {
    printf( "%s:%d: Error, could not find compartment '%s' in cell '%s'\n",
              L->file, L->line, label, CellName);
    fprintf ( stderr, "%s:%d: Error, could not find compartment '%s' in cell '%s'\n",
              L->file, L->line, label, CellName);
    TIN->nDataErr++;
  }

  return (idx);                         
}

/*----------------------------------------------------------------------------*/
/* Provides an error-reporting layer for name lookups                         */

int getName (LOCATOR *L, char *name, TREENODE *SymTree, char *tag)
{
  int idx;

  idx = getTreeIndex (name, SymTree);
  if (idx < 0) 
  {
    printerr ("%s:%d: Error, %s '%s' is used in input file, but is not defined\n",
              L->file, L->line, tag, name);
    TIN->nDataErr++;
  }
  return (idx);  
}

/*----------------------------------------------------------------------------*/

void CellCount (ARRAYS *AR)
{
  T_BRAIN  *Brain;
  T_COLUMN *Col;
  T_LAYER  *Lay;
  T_CELL   *Cel;
  T_CMP    *Cmp;
  int i, j, k, n, nCell, nCmp;


  AR->LayerCount = AR->CellCount = AR->ClusterCount = AR->CmpCount =
  AR->ChanCount  = nCell = nCmp = 0;

  Brain = AR->Brain;
//printf ("CellCount: Brain has %d columns\n", Brain->nColumns);
  for (i = 0; i < Brain->nColumns; i++)
  {
    Col = AR->Columns [Brain->Columns [i]];
//  printf ("CellCount:   Column %2d has %d layers\n", i, Col->nLayers);
    for (j = 0; j < Col->nLayers; j++)
    {
      Lay = AR->Layers [Col->Layers [j]];
      AR->LayerCount++;
//    printf ("CellCount:     Layer %2d has %d cell types\n", i, Lay->nCellTypes);
      for (k = 0; k < Lay->nCellTypes; k++)
      {
        nCell = Lay->CellQty [k];
        AR->CellCount += nCell;
        AR->ClusterCount++;
        Cel = AR->Cells [Lay->CellTypes [k]];
        nCmp = nCell * Cel->nCmp;
        AR->CmpCount += nCmp;
        for (n = 0; n < Cel->nCmp; n++)
        {
          Cmp = AR->Cmp [Cel->Cmp [n]];
          AR->ChanCount += nCell * Cmp->nChannels;
        }
      }
//    printf ("CellCount:     AR->ClusterCount = %d\n", AR->ClusterCount);
    }
  }

  printerr ("cells = %d, clusters = %d, cmps = %d, chans = %d\n",
            AR->CellCount, AR->ClusterCount, AR->CmpCount, AR->ChanCount);
}


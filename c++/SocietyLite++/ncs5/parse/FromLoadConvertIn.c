/* When a brain is being loaded from input, the only data members that
 * should be new or have new data are: the brain, reports, stimuli
 * This will prepare ARRAYS with those structures, and than let
 * the main program extrart the originals from the save file
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../defines.h"
#include "../InitStruct.h"
#include "input.h"
#include "list.h"
#include "SymTree.h"
#include "arrays.h"

extern TMP_INPUT *TIN;       /* This must be global so parse.y can access it */

//function prototypes - these functions are used instead of the normal ones to
//prevent them from trying to find the brain objects they would target
//e.g. Column, Layer, Cell, Cmp
//These objects won't be available until after the rest of loading is complete
void fromLoadConvertStim (TREENODE *SymTree, ARRAYS *AR);
void fromLoadConvertSti (TREENODE *SymTree, ARRAYS *AR);
void fromLoadConvertRpt (TREENODE *SymTree, ARRAYS *AR);

ARRAYS *fromLoadConvertIn (TREENODE *SymTree)
{
  ARRAYS *AR;
  T_BRAIN *brain;

  TIN->nDataErr = 0;

  AR = (ARRAYS *) calloc (sizeof (ARRAYS),1);

  brain     = TIN->Brain;
  AR->Brain = TIN->Brain;
  AR->Brain->StInject = ConvertList (brain->StInjNames,  brain->nStInject, SymTree, "Stimulus Inject");
  AR->Brain->Reports  = ConvertList (brain->ReportNames, brain->nReports,  SymTree, "Report");

  fromLoadConvertStim  (SymTree, AR);
  fromLoadConvertSti   (SymTree, AR);
  fromLoadConvertRpt   (SymTree, AR);

  AR->LayerCount = AR->CellCount = AR->ClusterCount = AR->CmpCount = AR->ChanCount = 0;

  //CellCount (AR);
  if (TIN->nDataErr > 0)
  {
    printerr ("Parser: %d Input errors found.\n", TIN->nDataErr);
  }
  return (AR);
}

/*----------------------------------------------------------------------------*/

void fromLoadConvertStim (TREENODE *SymTree, ARRAYS *AR)
{
  T_STIMULUS *stim;
  int i, j, idx;

  AR->Stimulus = NULL;
  AR->nStimulus = TIN->nStimulus;
  if (AR->nStimulus > 0)
  {
    //allocate space in ARRAYS struct for array of pointers
    AR->Stimulus = (T_STIMULUS **) calloc (AR->nStimulus, sizeof (T_STIMULUS *));
    stim = TIN->Stimulus;
    
    //put a pointer in the array for each stimulus
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

void fromLoadConvertSti (TREENODE *SymTree, ARRAYS *AR)
{
  T_STINJECT *st;
  int i, idx;

  AR->StInject = NULL;
  AR->nStInject = TIN->nStInject;
  if (AR->nStInject > 0)
  {
    //allocate space
    AR->StInject = (T_STINJECT **) calloc (AR->nStInject, sizeof (T_STINJECT *));
    st = TIN->StInject;
    
    for (i = 0; i < AR->nStInject; i++)
    {
      idx  = st->L.idx;
      AR->StInject [idx] = st;

/*    //don't get names yet - do that after all structures have been loaded
      st->STIM_TYPE = getName (&(st->L), st->StimName, SymTree, "Stimulus");
      st->Column    = getName (&(st->L), st->ColName,  SymTree, "Column");
      st->Layer     = getName (&(st->L), st->LayName,  SymTree, "Layer");
      st->Cell      = getName (&(st->L), st->CellName, SymTree, "Cell");
      st->Cmp       = getCmpIdx (st->CellName, st->CmpName, SymTree, &(st->L));
*/  
      st = st->L.next;
    }
  }
}

/*----------------------------------------------------------------------------*/

void fromLoadConvertRpt (TREENODE *SymTree, ARRAYS *AR)
{
  T_REPORT *rpt;
  int i, j, idx;

  AR->Reports = NULL;
  AR->nReports = TIN->nReport;
  if (AR->nReports > 0)
  {
    //allocate memory
    AR->Reports = (T_REPORT **) calloc (AR->nReports, sizeof (T_REPORT *));
    rpt = TIN->Report;
    
    for (i = 0; i < AR->nReports; i++)
    {
      idx  = rpt->L.idx;
      AR->Reports [idx] = rpt;
 /*   //don't get names yet - do that after all structures have been loaded
      rpt->Column = getName (&(rpt->L), rpt->ColName,  SymTree, "Column");
      rpt->Layer  = getName (&(rpt->L), rpt->LayName,  SymTree, "Layer");
      rpt->Cell   = getName (&(rpt->L), rpt->CellName, SymTree, "Cell");
      rpt->Cmp    = getCmpIdx (rpt->CellName, rpt->CmpName, SymTree, &(rpt->L));
 */ 
      rpt->Thing  = -1;
 /*
      if (rpt->ReportOn == SYNAPSE_USE)
        rpt->Thing = getName (&(rpt->L), rpt->Name, SymTree, "Synapse");
      else if (rpt->ReportOn == CHANNEL_RPT)
      {
        rpt->Thing = getName (&(rpt->L), rpt->Name, SymTree, "Channel");
//      printf ("parse/ConvertIn.c, Channel rpt, name = '%s', thing = %d\n", rpt->Name, rpt->Thing);
      }
*/
      //verify and convert times
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

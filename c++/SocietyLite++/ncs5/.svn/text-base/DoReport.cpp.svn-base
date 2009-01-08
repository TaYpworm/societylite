/* This module handles processing of Report objects.  See documentation at   */
/* end of file.                                                              */

#include "defines.h"

#include "DoReport.h"
#include "Managers.h"
#include "debug.h"
#include "memstat.h"

/*---------------------------------------------------------------------------*/
/* Cycle through the Brain's report list, doing reports if the time & such   */
/* are correct.                                                              */

void DoReports (int TimeStep)
{
  Report *R, *Rnext;
  bool RptDone;

  R = TheBrain->RptList;
  while (R != NULL)
  {
    Rnext = R->next;
    RptDone = R->DoReport (TimeStep);

    //If the report is finished, and the user will not dynamically add new start/stop time, delete it
    if (RptDone && !R->Dynamic)
      delete R;             /* Report finished, so delete from list */

    R = Rnext;
  }
}

/*---------------------------------------------------------------------------*/
/* Create a report from a T_REPORT struct, and add it to the Brain's list    */

void AddReport (T_REPORT *iRep, int idx, char *ASCIIhdr, RandomManager *RM)
{
  Report *R;
  Random *RN;

  RN = RM->GetReportRand (idx);

  R = new Report (iRep, ASCIIhdr, RN);

  if (R->Out == NULL) delete R;
}

/*---------------------------------------------------------------------------*/
/* Cycles through the input, creating specified reports.  Note that much of  */
/* the report's text ASCIIhdr must be created outside the Report object, since */
/* dynamically-created reports may not have access to the information in the */
/* AR struct.                                                                */

void MakeReportsFromInput (ARRAYS *AR, RandomManager *RM)
{
  T_BRAIN *iBrain;
  T_REPORT *iRep;
  int i;
  char ASCIIhdr [1024], *hptr, *title;

  iBrain = AR->Brain;
  printf ("DoReport::MakeReportsFromInput: nReports = %d\n", iBrain->nReports);

  title = iBrain->L.name;
  if (title == NULL) title = "No name";

  for (i = 0; i < iBrain->nReports; i++)    
  {
    iRep = AR->Reports [iBrain->Reports [i]];

    sprintf (ASCIIhdr, "%-126s\n%-31s\n%-31s\n", title, iRep->L.name,
                       NameDefine (iRep->ReportOn));
    hptr = ASCIIhdr + strlen (ASCIIhdr);

    /*sprintf (hptr, "%-31s\n%-31s\n%-31s\n%-31s\n",
             AR->Columns [iRep->Column]->L.name, AR->Layers [iRep->Layer]->L.name,
             AR->Cells [iRep->Cell]->L.name, AR->Cmp [iRep->Cmp]->L.name);
    */
    
    //For Compartment, I have made it use both the type name and the cell label,
    // putting both in the original 31 spaces allocated for the compartment
    sprintf (hptr, "%-31s\n%-31s\n%-31s\n%-16s%-15s\n",
             AR->Columns [iRep->Column]->L.name, AR->Layers [iRep->Layer]->L.name,
             AR->Cells [iRep->Cell]->L.name,
             AR->Cells [iRep->Cell]->Labels[iRep->Cmp], //label name
             AR->Cmp [ AR->Cells [iRep->Cell]->Cmp[iRep->Cmp] ]->L.name);   //type name

    if (iRep->ReportOn == SYNAPSE_USE)
    {
      hptr = hptr + strlen (hptr);
      sprintf (hptr, "%-31s\n", AR->Synapse [iRep->Thing]->L.name);
    }
    else if (iRep->ReportOn == CHANNEL_RPT)
    {
      hptr = hptr + strlen (hptr);
      sprintf (hptr, "%-31s\n", AR->Channel [iRep->Thing]->L.name);
      printf ("DoReport::MakeReportsFromInput: channel rpt, name = %s\n", hptr);
    }

    AddReport (iRep, i, ASCIIhdr, RM);
  } 
}

/*-----------------------------------------------------------------------------

This module handles creating and processing reports.  Reports can (potentially) 
be created either at startup, from info in the input file, or dynamically at run 
time.  (This last isn't inplemented yet, but the code is being re-written from 
the old MakeObject.cpp version to allow for the possibility.)

A linked list of Report objects is kept in the Brain, so that new ones can be 
added, and old ones deleted when they have finished.  A Report contains a 
pointer to the RptInfo struct, which holds information relating to data 
collection & output.  The mechanics of this is handled by Output.cpp.

This code should be very similar to CreateStim.cpp, just going in the opposite 
direction.  7/9/02 - JF

-----------------------------------------------------------------------------*/

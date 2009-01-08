/* Time-stamp: <21 Nov 2001 14:33:08 jamesf> */

/* This dumps a report of the information contained in the ARRAYS structure. */
/* Mostly for debugging, but might be a useful option for the real program.  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../InitStruct.h"
#include "arrays.h"

void dump (FILE *out, ARRAYS *AR, int flags)
{
  T_CSHELL *csh;
  T_CELL   *cel;
  T_REPORT *rpt;
  int i;

  fprintf (out, "\n\nElements defined in ARRAYS:\n");

  fprintf (out, "%5d Column Shells\n", AR->nCsh);
  fprintf (out, "%5d Columns\n", AR->nColumns);
  fprintf (out, "%5d Layer Shells\n", AR->nLsh);
  fprintf (out, "%5d Layers\n", AR->nLayers);
  fprintf (out, "%5d Compartments\n", AR->nCmp);
  fprintf (out, "%5d Cells\n", AR->nCells);
  fprintf (out, "%5d Channels\n", AR->nChannel);
  fprintf (out, "%5d Synapses\n", AR->nSynapse);
  fprintf (out, "%5d Syn FD\n", AR->nSynFD);
  fprintf (out, "%5d Syn Learn\n", AR->nSynLearn);
  fprintf (out, "%5d Syn Data\n", AR->nSynData);
  fprintf (out, "%5d Spikes\n", AR->nSpike);
  fprintf (out, "%5d Stimuli\n", AR->nStimulus);
  fprintf (out, "%5d Stimulus Injects\n", AR->nStInject);
  fprintf (out, "%5d Reports\n", AR->nReports);

  fprintf (out, "\n\nBrain contains\n", AR->nCsh);
  fprintf (out, "    %5d Columns\n", AR->Brain->nColumns);
  fprintf (out, "    %5d BrainConnect elements\n", AR->Brain->nConnect);
  fprintf (out, "    %5d Stimulus Injects\n", AR->Brain->nStInject);
  fprintf (out, "    %5d Reports\n", AR->Brain->nReports);

  fprintf (out, "\n\nColumn Shells (%d):\n", AR->nCsh);
  for (i = 0; i < AR->nCsh; i++)
  {
    csh = AR->Csh [i];
    fprintf (out, "    %5d: %16s at(%5.2f, %5.2f)\n", i, csh->L.name, csh->x, csh->y);
  }

  fprintf (out, "\n\nCells (%d):\n", AR->nCells);
  for (i = 0; i < AR->nCells; i++)
  {
    cel = AR->Cells [i];
    fprintf (out, "    %5d: %16s\n", i, cel->L.name);
             
  }

  fprintf (out, "\n\nReports (%d):\n", AR->nReports);
  for (i = 0; i < AR->nReports; i++)
  {
    rpt = AR->Reports [i];
    fprintf (out, "    %3d: %s -> %s\n", i, rpt->L.name, rpt->FileName);
  }
}

/*----------------------------------------------------------------------------*/
/* Print brain definition in hierarchical format                              */

void PrintBrain (FILE *out, ARRAYS *AR)
{
  T_BRAIN *Br;
  T_COLUMN *col;
  T_LAYER *lay;
  T_CELL *cel;
  int i, j, k;

  fprintf (out, "\n--------------------------------------------------------------------------\n");
  fprintf (out, "Brain structure is defined as:\n");

  Br = AR->Brain;
  fprintf (out, "Brain %s, %d Columns, %d Stim Injects, %d Reports, %d Connects\n",
           Br->L.name, Br->nColumns, Br->nStInject, Br->nReports, Br->nConnect);

  for (i = 0; i < Br->nColumns; i++)
  {
    col = AR->Columns [i];
    fprintf (out, "  Column %2d: %s, %d layers, %d connects\n", 
             i, col->L.name, col->nLayers, col->nConnect);
    for (j = 0; j < col->nLayers; j++)
    {
      lay = AR->Layers [col->Layers [j]];
      fprintf (out, "    Layer %2d: %s, %d cell types, %d connects\n", 
               j, lay->L.name, lay->nCellTypes, lay->nConnect);
      for (k = 0; k < lay->nCellTypes; k++)
      {
        cel = AR->Cells [lay->CellTypes [k]];
        fprintf (out, "      CellType %2d: %s, %d cells, %d connects\n", 
                 k, cel->L.name, lay->CellQty [k], cel->nConnect);
      }
    }
  }
}

/*----------------------------------------------------------------------------*/
/* Print brain connections in hierarchical format                            */

void PrintConnect (FILE *out, ARRAYS *AR)
{
  T_BRAIN *Br;
  T_COLUMN *fCol, *tCol;
  T_LAYER *fLay, *tLay;
  T_CELL *fCel, *tCel;
  T_CONNECT *cc;
  int i, j, k;

  fprintf (out, "\n--------------------------------------------------------------------------\n");
  fprintf (out, "Brain connection structure defined\n\n");

  Br = AR->Brain;
  fprintf (out, "%6d colunns\n", Br->nColumns);
  fprintf (out, "%6d brain-level connects\n\n", Br->nConnect);

  for (i = 0; i < Br->nConnect; i++)
  {
    cc = Br->Connect [i];
    fCol  = AR->Columns [cc->FromCol];
    fLay  = AR->Layers  [cc->FromLay];
    fCel  = AR->Cells   [cc->FromCell];
    fprintf (out, "  %2d: %2d, %2d, %2d (%s, %s, %s)\n", i, cc->FromCol, cc->FromLay, cc->FromCell,
             fCol->L.name, fLay->L.name, fCel->L.name);
    tCol = AR->Columns [cc->ToCol];
    tLay = AR->Layers  [cc->ToLay];
    tCel = AR->Cells   [cc->ToCell];
    fprintf (out, "      %2d, %2d, %2d (%s, %s, %s)\n", cc->ToCol, cc->ToLay, cc->ToCell,
             tCol->L.name, tLay->L.name, tCel->L.name);
  }

  printf ("\nColumn level connects\n");
  for (i = 0; i < Br->nColumns; i++)     
  {   
    fCol = tCol = AR->Columns [Br->Columns [i]];
    fprintf (out, "%2d: (%s), %6d column-column connects\n",
                  i, fCol->L.name, fCol->nConnect);

    for (j = 0; j < fCol->nConnect; j++)
    {
      cc = fCol->Connect [j];
      fLay = AR->Layers [cc->FromLay];
      fCel = AR->Cells  [cc->FromCell];
      fprintf (out, "  %2d: %2d, %2d, %2d (%s, %s, %s)\n", j, cc->ToCol, cc->ToLay, cc->ToCell,
               fCol->L.name, fLay->L.name, fCel->L.name);
      tLay = AR->Layers [cc->ToLay];
      tCel = AR->Cells  [cc->ToCell];
      fprintf (out, "      %2d, %2d, %2d (%s, %s, %s)\n", cc->ToCol, cc->ToLay, cc->ToCell,
               tCol->L.name, tLay->L.name, tCel->L.name);
    }

    printf ("\nLayer level connects\n");
    for (j = 0; j < fCol->nLayers; j++)
    {   
      fLay = tLay = AR->Layers [fCol->Layers [j]];
      fprintf (out, "%2d: Layer %s, %6d layer-layer connects\n",
                    j, fLay->L.name, fLay->nConnect);
  
      for (k = 0; k < fLay->nConnect; k++)
      {
        cc = fLay->Connect [k];
        fCel = AR->Cells [cc->FromCell];
        fprintf (out, "  %2d: %2d, %2d, %2d (%s, %s, %s)\n", j, cc->ToCol, cc->ToLay, cc->ToCell,
                 fCol->L.name, fLay->L.name, fCel->L.name); fflush (stdout);
        tCel = AR->Cells [cc->ToCell];
        fprintf (out, "      %2d, %2d, %2d (%s, %s, %s)\n", cc->ToCol, cc->ToLay, cc->ToCell,
                 tCol->L.name, tLay->L.name, tCel->L.name); fflush (stdout);
      }
    }
  }
}

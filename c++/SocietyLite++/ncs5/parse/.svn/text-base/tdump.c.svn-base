#include <stdio.h>
#include "../InitStruct.h"
#include "arrays.h"

void tdump (ARRAYS *AR)
{
  T_COLUMN *Col;
  T_LAYER  *Lay;
  T_CELL   *Cel;
  int i, j;

/*printf ("TDump: %d columns\n", AR->nColumns);
  for (i = 0; i < AR->nColumns; i++)          
  {
    Col = &AR->Columns [i];
    printf ("  Column %d (%s)\n", Col->idx, Col->name);
  }
  printf ("\n");


  printf ("TDump: %d layers\n", AR->nLayers);
  for (j = 0; j < AR->nLayers; j++)      
  {
    Lay = &AR->Layers [j];
    printf ("    Layer %d (%s)\n", Lay->idx, Lay->name);
  }
  printf ("\n");*/

  printf ("TDump: %d cells\n", AR->nCells);
  for (i = 0; i < AR->nCells; i++)     
  {   
    Cel = &(AR->Cells [i]);
    printf ("      Cell %d (%x)\n", Cel->idx, AR->Cells [i].name);
    printf ("      Cell %d (%s)\n", Cel->idx, AR->Cells [i].name);
  }
  printf ("TDump done\n");
}

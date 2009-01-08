/* Functions used in creating elements of input file */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../defines.h"
#include "../InitStruct.h"
#include "input.h"
#include "tree.h"
#include "parse.h"

extern TMP_INPUT *TIN;
extern int debug;

void *allocElem (void *, size_t, int);


/*--------------------------------------------------------------------------*/

T_BRAIN *makebrain ()
{
  T_BRAIN *p;

  if (TIN->nBrain > 0)
  {
    TIN->nParseErr++;
    print2err ("%s:%d: Error, multiple BRAIN statements\n", TIN->file, TIN->line);
  }

  p = (T_BRAIN *) allocElem ((void *) TIN->Brain, sizeof (T_BRAIN), TK_BRAIN);

  TIN->Brain = p;
  p->L.idx   = TIN->nBrain++;

  p->job = p->distribute = NULL;
  p->ConnectRpt = p->SpikeRpt  = FALSE;

  p->nColumns    = p->nStInject  = p->nReports    = p->nConnect = 0;
  p->ColumnNames = p->StInjNames = p->ReportNames = NULL;
  p->Columns     = NULL;
  p->StInject    = NULL;
  p->Reports     = NULL;
  p->CnList      = NULL;
  p->Port        = 0;
  p->HostPort    = 0;
  p->Seed        = 0;
  p->flag        = 0;
  p->loadfile = p->savefile = NULL;
  p->nRecurrent  = 0;
  p->recurrentList = NULL;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_CSHELL *makecsh ()
{
  T_CSHELL *p;

  p = (T_CSHELL *) allocElem ((void *) TIN->Csh, sizeof (T_CSHELL), TK_CSHELL);
  
  TIN->Csh = p;
  p->L.idx = TIN->nCsh++;

  p->x = p->y = p->width = p->height = 0;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_COLUMN *makecolumn ()
{
  T_COLUMN *p;

  p = (T_COLUMN *) allocElem ((void *) TIN->Column, sizeof (T_COLUMN), TK_COLUMN);

  TIN->Column = p;
  p->L.idx = TIN->nColumn++;

  p->shellName = NULL;
  p->nLayers    = p->nConnect = 0;
  p->LayerNames = NULL;
  p->Layers     = NULL;
  p->CnList     = NULL;
  p->nRecurrent = 0;
  p->recurrentList = NULL;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_LSHELL *makelsh ()
{
  T_LSHELL *p;

  p = (T_LSHELL *) allocElem ((void *) TIN->Lsh, sizeof (T_LSHELL), TK_LSHELL);
  
  TIN->Lsh = p;
  p->L.idx = TIN->nLsh++;

  p->Upper = p->Lower = 0;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_LAYER *makelayer ()
{
  T_LAYER *p;

  p = (T_LAYER *) allocElem ((void *) TIN->Layer, sizeof (T_LAYER), TK_LAYER);

  TIN->Layer = p;
  p->L.idx   = TIN->nLayer++;

  p->nCellTypes = p->nConnect = 0;
  p->shellName  = NULL;
  p->CellNames  = NULL;
  p->CellTypes  = NULL;
  p->CellQty    = NULL;
  p->CnList     = NULL;
  p->recurrentList = NULL;
  p->nRecurrent = 0;
  p->recurrentHandled = 0;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_CELL *makecell ()
{
  T_CELL *p;

  p = (T_CELL *) allocElem ((void *) TIN->Cell, sizeof (T_CELL), TK_CELL);

  TIN->Cell = p;
  p->L.idx  = TIN->nCell++;

  p->nCmp    = p->nConnect = 0;
  p->Labels  = NULL;
  p->Cmp     = NULL;
  p->CnList  = NULL;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_CMP *makecmp ()
{
  T_CMP *p;

  p = (T_CMP *) allocElem ((void *) TIN->Cmp, sizeof (T_CMP), TK_CMP);

  TIN->Cmp = p;
  p->L.idx = TIN->nCmp++;

  p->Seed         = 0;
  p->nChannels    = 0;
  p->ChannelNames = NULL;
  p->Channels     = NULL;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_CHANNEL *makechan (char *family)
{
  T_CHANNEL *p;

  p = (T_CHANNEL*) allocElem ((void *) TIN->Channel, sizeof (T_CHANNEL), TK_CHANNEL);

  TIN->Channel = p;
  p->L.idx  = TIN->nChannel++;

  p->family = FamilyCode (family);
  p->Seed = 0;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_SYNAPSE *makesynapse ()
{
  T_SYNAPSE *p;

  p = (T_SYNAPSE *) allocElem ((void *) TIN->Synapse, sizeof (T_SYNAPSE), TK_SYNAPSE);

  TIN->Synapse = p;
  p->L.idx = TIN->nSynapse++;

  p->SfdName = p->LearnName = p->PsgName = p->DataName = p->AugmentationName = NULL;
  p->AugmentationIdx = -1;

  //zero out some elements which are not commonly used.
  p->InitRSE[0] = p->InitRSE[1] = 1;
  p->Seed = 0;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_SYN_FD *makesyn_fd ()
{
  T_SYN_FD *p;

  p = (T_SYN_FD *) allocElem ((void *) TIN->SynFD, sizeof (T_SYN_FD), TK_SYN_FD);

  TIN->SynFD = p;
  p->L.idx   = TIN->nSynFD++;

  p->SFD = INVALID;
  p->Seed = 0;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_SYNLEARN *makesyn_learn ()
{
  T_SYNLEARN *p;

  p = (T_SYNLEARN *) allocElem ((void *) TIN->SynLearn, sizeof (T_SYNLEARN), TK_SYN_LEARN);

  TIN->SynLearn = p;
  p->L.idx = TIN->nSynLearn++;

  p->Learning = INVALID;
  p->Seed = 0;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_SYNDATA *makesyn_data ()
{
  T_SYNDATA *p;

  p = (T_SYNDATA *) allocElem ((void *) TIN->SynData, sizeof (T_SYNDATA), TK_SYN_DATA);

  TIN->SynData = p;
  p->L.idx = TIN->nSynData++;
  p->Seed = 0;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_SYNPSG *makesyn_psg ()
{
  T_SYNPSG *p;

  p = (T_SYNPSG *) allocElem ((void *) TIN->SynPSG, sizeof (T_SYNPSG), TK_SYN_PSG);

  TIN->SynPSG = p;
  p->L.idx = TIN->nSynPSG++;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_SYNAUGMENTATION *makesyn_augmentation ()
{
  T_SYNAUGMENTATION *p;

  p = (T_SYNAUGMENTATION *) allocElem( (void*) TIN->SynAugmentation, sizeof( T_SYNAUGMENTATION), TK_SYN_AUGMENTATION );

  TIN->SynAugmentation = p;
  p->L.idx = TIN->nSynAugmentation++;
  p->SA_delay[0] = 0.5;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_SPIKE *makespike ()
{
  T_SPIKE *p;

  p = (T_SPIKE *) allocElem ((void *) TIN->Spike, sizeof (T_SPIKE), TK_SPIKE);
  
  TIN->Spike = p;
  p->L.idx   = TIN->nSpike++;

  p->nVoltages = 0;
  p->Voltages  = NULL;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_STIMULUS *makestim ()
{
  T_STIMULUS *p;

  p = (T_STIMULUS *) allocElem ((void *) TIN->Stimulus, sizeof (T_STIMULUS), TK_STIMULUS);

  TIN->Stimulus = p;
  p->L.idx = TIN->nStimulus++;

  p->Time_Start    = p->Time_Stop = NULL;
  p->FileName      = NULL;
  p->Port          = 0;
  p->MODE          = p->PATTERN = p->TIMING = INVALID;
  p->CellsPerFreq  = p->nFreqs = 0;
  p->DynRange [0]  = p->DynRange [1] = 0.0;
  p->Seed = 0;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_STINJECT *makesti ()
{
  T_STINJECT *p;

  p = (T_STINJECT *) allocElem ((void *) TIN->StInject, sizeof (T_STINJECT), TK_ST_INJECT);

  TIN->StInject = p;
  p->L.idx = TIN->nStInject++;

  p->StimName = p->ColName = p->LayName = p->CellName = p->CmpName = NULL;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_REPORT *makereport ()
{
  T_REPORT *p;

  p = (T_REPORT *) allocElem ((void *) TIN->Report, sizeof (T_REPORT), TK_REPORT);

  TIN->Report = p;
  p->L.idx = TIN->nReport++;

  p->Port   = 0;
  p->Time_Start = p->Time_Stop = NULL;
  p->ColName = p->LayName = p->CellName = p->CmpName = p->Name = p->FileName = NULL;
  p->ReportOn = INVALID;
  p->Seed = 0;
  p->reportFlag = 0;

  return (p);
}

/*---------------------------------------------------------------------------*/
/* Allocate space for a brain element, giving error message on failure       */

void *allocElem (void *next, size_t bytes, int kind)
{
  T_ANYTHING *p;

  p = (T_ANYTHING *) malloc (bytes);
  if (p == NULL)
  {
    fprintf (stderr, "Input: Allocation of %s struct failed at input line %d\n", 
             TK2name (kind), TIN->line);
    exit (-1);
  }

  p->L.next = next;
  p->L.file = TIN->file;
  p->L.line = TIN->line;
  p->L.kind = kind;
  p->L.name = NULL;
  TIN->nElements++;

  return (p);
}

/*---------------------------------------------------------------------------*/
/* Allocates space for array of double, and copies values to it, returning   */
/* pointer to allocated space                                                */

double *allocVlist (int nval, double *values)
{
  double *vlist;
  int i;

  vlist = calloc (nval, sizeof (double));
  for (i = 0; i < nval; i++)
    vlist [i] = values [i];
  return (vlist);
}

/*---------------------------------------------------------------------------*/
/* Same as allocVlist, but values are stored in reverse order, so calling    */
/* code can use index == 0 test for end                                      */

double *allocRVlist (int nval, double *values)
{
  double *vlist;
  int i;

  vlist = calloc (nval, sizeof (double));
  for (i = 0; i < nval; i++)
    vlist [nval - i - 1] = values [i];
  return (vlist);
}

/*--------------------------------------------------------------------------*/

char *TK2name (int token)
{
  char *str; 

  switch (token)
  {
    case TK_BRAIN:        str = "BRAIN";
        break;
    case TK_CSHELL:       str = "COLUMN_SHELL";
        break;
    case TK_COLUMN:       str = "COLUMN";
        break;
    case TK_DATA_LABEL:   str = "DATA_LABEL";
        break;
    case TK_LSHELL:       str = "LAYER_SHELL";
        break;
    case TK_LAYER:        str = "LAYER";
        break;
    case TK_CMP:          str = "COMPARTMENT";
        break;
    case TK_CELL:         str = "CELL";
        break;
    case TK_CHANNEL:      str = "CHANNEL";
        break;
    case TK_SYNAPSE:      str = "SYNAPSE";
        break;
    case TK_SYN_FD:       str = "SYN_FACIL_DEPRESS";
        break;
    case TK_SYN_PSG:      str = "SYN_PSG";
        break;
    case TK_SYN_DATA:     str = "SYN_DATA";
        break;
    case TK_SYN_LEARN:    str = "SYN_LEARNING";
        break;
    case TK_SYN_AUGMENTATION: str = "SYN_AUGMENTATION";
        break;
    case TK_SPIKE:        str = "SPIKESHAPE";
        break;
    case TK_STIMULUS:     str = "STIMULUS";
        break;
    case TK_ST_INJECT:    str = "STIMULUS_INJECT";
        break;
    case TK_REPORT:       str = "REPORT";
        break;
    case TK_Km:           str = "Km";
        break;
    case TK_Kahp:         str = "Kahp";
        break;
    case TK_Ka:           str = "Ka";
        break;
    case TK_COLUMN_TYPE:  str = "COLUMN_TYPE";
        break;
    case TK_LAYER_TYPE:   str = "LAYER_TYPE";
        break;
    case TK_CELL_TYPE:    str = "CELL_TYPE";
        break;
    case TK_INTERACTIVE:  str = "INTERACTIVE";
        break;
    case TK_IGNORE_EMPTY: str = "IGNORE_EMPTY";
        break;
    case TK_SAVE_SYN:     str = "SAVE_SYN";
        break;
    default:              str = "UNKNOWN";
        break;
  }
  return (str);
}

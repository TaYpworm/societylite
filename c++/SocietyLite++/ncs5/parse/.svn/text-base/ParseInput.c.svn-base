/* This function is the top level of the ncs input parser.  It is basically */
/* a shell which allocates a root node for the symbol tree, opens the input */
/* file, and then calls the routine defined by parse.y to do the parsing.   */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "input.h"
#include "SymTree.h"
#include "arrays.h"
#include "proto.h"
#include "ReadFile.h"
#include "FromLoadConvertIn.c"

//#define MEM_KEY KEY_INPUT

TMP_INPUT *TIN;       /* This must be global so parse.y can access it */
char *currentFile = NULL;

ARRAYS *ParseInput (int node, char *filename, int output)
{
  TREENODE *SymTree;
  ARRAYS *AR;
  int nbytes;
  char *fbuf;

  MakeTIN (node);
  TIN->file = filename;
  TIN->line = 1;

  currentFile = strdup (filename);  //I want to know the name of the file that starts it all

/* This reads the entire input file into memory, using MPI to transfer so as */
/* to avoid NFS file contention problems on startup.  The scanner then reads */
/* from the buffer instead of from file.                                     */

  fbuf = ReadFile (filename, node, &nbytes);

//printf ("Node %d, ReadFile returns %d bytes\n", node, nbytes);

  printerr ("Parsing file...\n");

  scanner (fbuf, nbytes);
  free (fbuf);

  print2err ("Input scanned, %d elements, %d syntax errors found\n",
              TIN->nElements, TIN->nParseErr);

  if (TIN->nParseErr > 0)
  {
    print2err ("Parsing failed, %d parse errors found.\n", TIN->nParseErr);
    return (NULL);
  }

  if (TIN->nBrain == 0)
  {
    print2err ("Fatal Error: no BRAIN structure found in input\n");
    return (NULL);
  }

  SymTree = makeSymTree (TIN);

  if (SymTree == NULL)
  {
    print2err ("Fatal error: parser failed to create symbol table\n");
    return (NULL);
  }

  if (TIN->Node == 0) PrintInfo (TIN, stdout);

/* Add an option here to check all the input files which are */
/* named in the brain definition file?                       */

  if ((output > 0) && (TIN->Node == 0))
  {
    printf ("\n\nNames Defined in Input\n");
    TreePrint (SymTree);
  }

//printf ("Node %3d: Calling ConvertIn\n", node); fflush (stdout);
  AR = NULL;
  if( !TIN->Brain->loadfile )
    AR = ConvertIn (SymTree);
  else //if loading a brain, use a special converter that affects only the brain, reports, and stimulus
    AR = fromLoadConvertIn (SymTree);

//printf ("Node %3d: ConvertIn done\n", node); fflush (stdout);
  if (TIN->nDataErr > 0)
  {
    print2err ("Parsing failed, %d data errors found.\n", TIN->nDataErr);
    return (NULL);
  }

  if (AR != NULL)
  {
    if (TIN->Node == 0)
    {
      printf ("LayerCount = %d\n", AR->LayerCount); 
      printf ("CellCount  = %d\n", AR->CellCount); 

//    PrintBrain (stdout, AR);

//    dump (stdout, AR, flags);
//    PrintConnect (stdout, AR);
    }
  }

  free (TIN);
  return (AR);
}

/*---------------------------------------------------------------------------*/
/* Allocates & initializes temporary input struct                            */

void MakeTIN (int node)
{
  TIN = (TMP_INPUT *) malloc (sizeof (TMP_INPUT));
  if (TIN == NULL)
  {
    fprintf (stderr, "error allocating base struct quitting\n");
    exit (-1);
  }

  TIN->Node       = node;
  TIN->nParseErr  = TIN->nDataErr = TIN->line = 0;
  
  TIN->nElements = TIN->nBrain   = TIN->nCsh      = TIN->nColumn   =
  TIN->nLsh      = TIN->nLayer   = TIN->nCmp      = TIN->nCell     =
  TIN->nChannel  = TIN->nSynapse = TIN->nSynLearn = TIN->nSynFD    = 
  TIN->nSynPSG   = TIN->nSynData = TIN->nSpike    = TIN->nStimulus =
  TIN->nStInject = TIN->nReport  = TIN->nSynAugmentation  = 0;

  TIN->Brain    = NULL;
  TIN->Csh      = NULL;
  TIN->Column   = NULL;
  TIN->Lsh      = NULL;
  TIN->Layer    = NULL;
  TIN->Cell     = NULL;
  TIN->Cmp      = NULL;
  TIN->Channel  = NULL;
  TIN->Synapse  = NULL;
  TIN->SynLearn = NULL;
  TIN->SynFD    = NULL;
  TIN->SynPSG   = NULL;
  TIN->SynData  = NULL;
  TIN->Stimulus = NULL;
  TIN->StInject = NULL;
  TIN->Spike    = NULL;
  TIN->Report   = NULL;
  TIN->SynAugmentation = NULL;
}

/*--------------------------------------------------------------------------*/
/* Prints info on the number of elements of each type found                 */

PrintInfo (TMP_INPUT *TIN, FILE *out)
{
  fprintf (out, "Elements defined in input file:\n");

  fprintf (out, "    %5d Brain\n",            TIN->nBrain);
  fprintf (out, "    %5d Column Shells\n",    TIN->nCsh);
  fprintf (out, "    %5d Columns\n",          TIN->nColumn);
  fprintf (out, "    %5d Layer Shells\n",     TIN->nLsh);
  fprintf (out, "    %5d Layers\n",           TIN->nLayer);
  fprintf (out, "    %5d Compartments\n",     TIN->nCmp);
  fprintf (out, "    %5d Cells\n",            TIN->nCell);
  fprintf (out, "    %5d Channels\n",         TIN->nChannel);
  fprintf (out, "    %5d Synapses\n",         TIN->nSynapse);
  fprintf (out, "    %5d Syn_FD\n",           TIN->nSynFD);
  fprintf (out, "    %5d Syn_PSG\n",          TIN->nSynPSG);
  fprintf (out, "    %5d Syn Learn\n",        TIN->nSynLearn);
  fprintf (out, "    %5d Syn Data\n",         TIN->nSynData);
  fprintf (out, "    %5d Spikes\n",           TIN->nSpike);
  fprintf (out, "    %5d Stimuli\n",          TIN->nStimulus);
  fprintf (out, "    %5d Stimulus Injects\n", TIN->nStInject);
  fprintf (out, "    %5d Reports\n",          TIN->nReport);
}

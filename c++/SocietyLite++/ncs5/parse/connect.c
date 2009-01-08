/* Functions used in creating connection elements of brain structures */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../InitStruct.h"
#include "input.h"

extern TMP_INPUT *TIN;       /* This must be global so parse.y can access it */

/*--------------------------------------------------------------------------*/
//connection for simple probability of connection

T_CONNECT *makeConnect (T_CONNECT *Con, int *idx,
                        char *fromCol, char *fromLay, char *fromCel, char *fromCmp,
                        char *toCol,   char *toLay,   char *toCel,   char *toCmp,
                        char *SynName, double prob, double speed)
{
  T_CONNECT *p;

  p = (T_CONNECT *) malloc (sizeof (T_CONNECT));
  if (p == NULL)
  {
    fprintf (stderr, "Allocation failed at input line %d\n", TIN->line);
    exit (-1);
  }

  p->L.next   = Con;
  p->L.idx    = (*idx)++;
  p->L.file   = TIN->file;
  p->L.line   = TIN->line;

  p->fromColName  = (fromCol == NULL) ? NULL : strdup (fromCol);
  p->fromLayName  = (fromLay == NULL) ? NULL : strdup (fromLay);
  p->fromCellName = (fromCel == NULL) ? NULL : strdup (fromCel);
  p->fromCmpName  = (fromCmp == NULL) ? NULL : strdup (fromCmp);

  p->toColName    = (fromCol == NULL) ? NULL : strdup (toCol);
  p->toLayName    = (fromLay == NULL) ? NULL : strdup (toLay);
  p->toCellName   = (fromCel == NULL) ? NULL : strdup (toCel);
  p->toCmpName    = (fromCmp == NULL) ? NULL : strdup (toCmp);

  p->SynName   = strdup (SynName);
  p->Prob      = prob;
  p->speed     = speed;
  p->step      = 0;
  p->recurrentProbability[0] = p->recurrentProbability[1] = 0;
  p->disabled = 0;
  p->recurrentConnection = NULL;

  return (p);
}

/*--------------------------------------------------------------------------*/
//connection for using distance based probability of connection

T_CONNECT *makeDecayingConnect (T_CONNECT *Con, int *idx,
                        char *fromCol, char *fromLay, char *fromCel, char *fromCmp,
                        char *toCol,   char *toLay,   char *toCel,   char *toCmp,
                        char *SynName, double maxprob, double distanceStep, double speed)
{
    T_CONNECT *p = makeConnect( Con, idx, fromCol, fromLay, fromCel, fromCmp,
                                toCol, toLay, toCel, toCmp, SynName, maxprob, speed );
    
    p->step = distanceStep;
    
    //should I use p->prob for both methods?
    // another variable would waste space, but make definition clearer.
    return (p);
}

/*--------------------------------------------------------------------------*/

T_RECURRENT *makeRecurrentConnect (T_RECURRENT *Con, int *idx,
                        char *fromCol, char *fromLay, char *fromCel, char *fromCmp,
                        char *toCol,   char *toLay,   char *toCel,   char *toCmp,
                        char *synName, double AtoB, double BtoA )
{
  T_RECURRENT *p;

  p = (T_RECURRENT *) malloc (sizeof (T_RECURRENT));
  if (p == NULL)
  {
    fprintf (stderr, "Allocation failed at input line %d\n", TIN->line);
    exit (-1);
  }

  p->L.next   = Con;
  p->L.idx    = (*idx)++;
  p->L.file   = TIN->file;
  p->L.line   = TIN->line;

  p->colNameA  = (fromCol == NULL) ? NULL : strdup (fromCol);
  p->layNameA  = (fromLay == NULL) ? NULL : strdup (fromLay);
  p->cellNameA = (fromCel == NULL) ? NULL : strdup (fromCel);
  p->cmpNameA  = (fromCmp == NULL) ? NULL : strdup (fromCmp);

  p->colNameB    = (fromCol == NULL) ? NULL : strdup (toCol);
  p->layNameB    = (fromLay == NULL) ? NULL : strdup (toLay);
  p->cellNameB   = (fromCel == NULL) ? NULL : strdup (toCel);
  p->cmpNameB    = (fromCmp == NULL) ? NULL : strdup (toCmp);

  p->synName = (synName==NULL) ? NULL :  strdup(synName);
  p->PrecurrenceAtoB = AtoB;
  p->PrecurrenceBtoA = BtoA;

  return (p);
}

/*--------------------------------------------------------------------------*/

T_CMPCONNECT *makeCmpConn (T_CMPCONNECT *Con, 
                            char *fromCmp, char *toCmp, double speed, double G, double retroG)
{
  T_CMPCONNECT *p;

  p = (T_CMPCONNECT *) malloc (sizeof (T_CMPCONNECT));
  if (p == NULL)
  {
    fprintf (stderr, "Allocation failed at input line %d\n", TIN->line);
    exit (-1);
  }

  p->L.next   = Con;
  p->L.file   = TIN->file;
  p->L.line   = TIN->line;

  p->fromCmpName = strdup (fromCmp);
  p->toCmpName   = strdup (toCmp);
  p->G           = G;
  p->retroG      = retroG;
  p->Speed       = speed;

  return (p);
}


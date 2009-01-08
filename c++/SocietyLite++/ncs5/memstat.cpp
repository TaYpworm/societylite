/* Routines to enable tracking of memory allocation.  There are two forms.   */
/* For tracking of object creation & deletion, the macros MEMADDOBJ and      */
/* MEMFREEOBJ must be called in the constructor & destructor.  For *alloc    */
/* calls, the only action needed is to include the memstat.h file.  The      */
/* macros have no effect unless the MEM_STATS flag is set.                   */

#include <stdlib.h>
#include <stdio.h>

#include <map>

#ifdef MEM_STATS

#define MEM_SELF
#include "memstat.h"
#include "memstruct.h"
#include "debug.h"

static int MemTotal = 0;     /* Total memory allocated by these calls */

/* Each allocated chunk of memory is classified under one of these */

static char *MemNames [] = {"Brain",      "BuildBrain",  "Cell",      "CellManager",
                            "Channel",    "Compartment", "CmpDelay",  "Column",
                            "ConnectMgr", "DistMgr",     "DoReport",  "DoStim",
                            "File",       "InitMgr",     "Input",     "KillFile",
                            "Layer",      "LearnTable",  "main",      "Message",
                            "MessageBus", "MessageMgr",  "Port",      "Random",
                            "RandomMgr",  "Report",      "Stimulus",  "Synapse",
                            "SynDelay",   "SynLearn",    "SynPSG",
                            "SynRSE",     "Output",      "Other"};

static MEMITEM *MemItems;


typedef std::map <void *, ALLOC *, std::less <void *> > MMAP;
static MMAP MM;


void MemInit ()
{
  MEMITEM *mem;
  int i, nkeys;

  nkeys = KEY_OTHER + 1;
  printf ("MemInit: nkeys = %d\n", nkeys);

  MemItems = (MEMITEM *) calloc (nkeys, sizeof (MEMITEM));

  for (i = 0; i < nkeys; i++)
  {
    mem = &(MemItems [i]);
    mem->key = i;
    mem->name = MemNames [i];
    mem->size = mem->count = mem->ndelete = mem->total = mem->objMax = mem->nalloc
              = mem->nfree = mem->alloc = mem->memMax = mem->total = mem->max = 0;
  }
}

/*---------------------------------------------------------------------------*/

void MemAddObj (int key, void *adr, int size)
{
  MEMITEM *mem;

  mem = &(MemItems [key]);
  if (mem->count == 0) mem->size = size;
  mem->count++;

  mem->objMem += size;
  if (mem->objMem > mem->objMax) mem->objMax = mem->objMem;

  mem->total += size;
  if (mem->total > mem->max) mem->max = mem->total;

  MemTotal += size;
}

/*---------------------------------------------------------------------------*/

void MemFreeObj (int key, void *adr)
{
  MEMITEM *mem;

  mem = &(MemItems [key]);
  mem->objMem -= mem->size;
  mem->total -= mem->size;
  mem->ndelete++;

  MemTotal -= mem->size;
}

/*---------------------------------------------------------------------------*/

void *MemMalloc (int key, int size)
{
  MEMITEM *mem;
  ALLOC *A;
  void *p;

  mem = &(MemItems [key]);

  mem->nalloc++;
  mem->alloc += size;
  if (mem->alloc > mem->memMax) mem->memMax = mem->alloc;

  mem->total += size;
  if (mem->total > mem->max) mem->max = mem->total;

  MemTotal += size;

  p = malloc (size);
//if (key == KEY_CONNECT) printf ("MemMalloc Connect, %d bytes at %x\n", size, p);

  if (MM.find (p) != MM.end ())
    printf ("memstat::MemMalloc: error, address %x already allocated\n", p);
  else
  {
    A = (ALLOC *) malloc (sizeof (ALLOC));
    A->size = size;
    A->key  = key;
    MM.insert (MMAP::value_type (p, A));
  }

  return (p);
}

/*---------------------------------------------------------------------------*/

void *MemCalloc (int key, int num, int size)
{
  MEMITEM *mem;
  ALLOC *A;
  int total;
  void *p;

//printf ("memstat::MemCalloc: key %s, size %d\n", MemKeyName (key), size);

  mem = &(MemItems [key]);
  total = num * size;
//if (key == KEY_OUTPUT) printf ("MemCalloc Output, size = %d, num = %d, total = %d\n", size, num, total);

  mem->nalloc++;
  mem->alloc += total;
  if (mem->alloc > mem->memMax) mem->memMax = mem->alloc;

  mem->total += total;
  if (mem->total > mem->max) mem->max = mem->total;

  MemTotal += total;

  p = calloc (num, size);

  if (MM.find (p) != MM.end ())
    printf ("memstat::MemCalloc: error, address %x already allocated\n", p);
  else
  {
    A = (ALLOC *) malloc (sizeof (ALLOC));
    A->size = total;
    A->key  = key;
    MM.insert (MMAP::value_type (p, A));
  }

  return (p);
}

/*---------------------------------------------------------------------------*/

void *MemRealloc (int key, void *adr, int size)
{
  MEMITEM *mem;
  MMAP::const_iterator item;
  ALLOC *A;
  int total;
  void *p;

//if (key == KEY_OUTPUT) printf ("MemRealloc Output, size = %d\n", size);
  item = MM.find (adr);
  if (item == MM.end ())     /* adr not in list, so same as alloc */
  {
    A = (ALLOC *) malloc (sizeof (ALLOC));
    A->key  = key;
    mem = &(MemItems [key]);
//  printf ("memstat::MemRealloc: creating new %s, %d bytes\n", MemKeyName (key), size);
    MM.insert (MMAP::value_type (p, A));
  }
  else                       /* adr already in list, so delete and adjust mem  */
  {
    A = (ALLOC *) item->second;
//  MM.erase (adr);

    mem = &(MemItems [A->key]);
//  printf ("memstat::MemRealloc: %s, was %d bytes, new = %d, alloc = %d, total = %d\n",
//          MemKeyName (key), A->size, size, mem->alloc, mem->total);
    mem->alloc -= A->size;
    mem->total -= A->size;
    MemTotal   -= A->size;
//  printf ("memstat::MemRealloc: subtract %d, alloc = %d, total = %d\n",
//          A->size, mem->alloc, mem->total);
  }

  A->size = size;

  mem->nalloc++;
  mem->alloc += size;
  if (mem->alloc > mem->memMax) mem->memMax = mem->alloc;

  mem->total += size;
  if (mem->total > mem->max) mem->max = mem->total;

//printf ("memstat::MemRealloc: add      %d, alloc = %d, total = %d\n",
//        size, mem->alloc, mem->total);

  MemTotal += size;

  p = realloc (adr, size);

//if (size > 0) printf ("memstat::MemRealloc: inserted new at %x\n", p);
  if (size > 0) MM.insert (MMAP::value_type (p, A));

  return (p);
}

/*---------------------------------------------------------------------------*/

void MemFree (void *adr)
{
  MEMITEM *mem;
  MMAP::const_iterator item;
  ALLOC *A;

//printf ("MemFree %x\n", adr);
  item = MM.find (adr);
  if (item != MM.end ())     /* adr found, delete and adjust mem  */
  {
    A = (ALLOC *) item->second;
//  printf ("memstat::MemFree: subtracting %d bytes at %x (%s) \n",
//          A->size, item->first, MemKeyName (A->key));
    mem = &(MemItems [A->key]);

    mem->alloc -= A->size;
    mem->total -= A->size;
    MemTotal   -= A->size;
    free (A);
    MM.erase (adr);
  }
  else
    printf ("memstat::MemFree: Error, freeing mem at %x that wasn't recorded\n", adr);

  free (adr);
}

/*---------------------------------------------------------------------------*/

void MemPrint ()
{
  MEMITEM *mem;
  int i, nkeys, objKb, objMax, allKb, allMax, total, mmax;

  nkeys = KEY_OTHER + 1;
  printf ("Memory Allocation:  Total Allocated = %d KBytes\n", (MemTotal / 1024) + 1);
  printf ("                  Object    Number    Number      Object       Alloc       Total         Max\n");
  printf ("    Item           Size    Created   Deleted        KB          KB           Kb           KB\n");
  for (i = 0; i < nkeys; i++)
  {
    mem = &(MemItems [i]);
    objKb  = (mem->objMem == 0) ? 0 : (mem->objMem / 1024) + 1;
    objMax = (mem->objMax == 0) ? 0 : (mem->objMax / 1024) + 1;
    allKb  = (mem->alloc  == 0) ? 0 : (mem->alloc  / 1024) + 1;
    allMax = (mem->memMax == 0) ? 0 : (mem->memMax / 1024) + 1;
    total  = (mem->total  == 0) ? 0 : (mem->total  / 1024) + 1;
    mmax   = (mem->max    == 0) ? 0 : (mem->max    / 1024) + 1;
    if (mem->max > 0)
      printf ("  %-12s    %6d  %8d  %8d  %10d  %10d  %10d  %10d\n", mem->name, mem->size,
              mem->count, mem->ndelete, objKb, allKb, total, mmax);
  }
}

/*---------------------------------------------------------------------------*/

char *MemKeyName (int key)
{
  char *name;

  name = MemNames [key];
  return (name);
}
#endif

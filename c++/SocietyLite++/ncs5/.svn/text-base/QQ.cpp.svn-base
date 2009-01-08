#define _QQSource

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include "QQ.h"
#include "debug.h"

static QQBase *qq;
static QQItem *qqPtr, *qqTop, *qqSave;

void QQInit (int size)
{
  printf ("QQInit\n");
  qq = (QQBase *) malloc (sizeof (QQBase));

  qq->table = (QQItem *) calloc (size, sizeof (QQItem));
  if (qq->table == NULL)
  {
    printf ("QQInit: error, could not allocate %d table entries\n", size);
    exit (0);
  }

  qqPtr = qqSave = qq->table;
  qqTop = qqPtr + size;

  qq->record  = TRUE;

  qq->keysize = MAXKEYS;
  qq->keys = (QQKey *) calloc (MAXKEYS, sizeof (QQKey));

  qq->nkeys = 1;
  qq->keys [0].Key = 0;
  qq->keys [0].Name = (char *) calloc (KEYLEN, sizeof (char));
  strncpy (qq->keys [0].Name, "Empty", KEYLEN);

  rdtsc (&(qq->TBase));
}

/*---------------------------------------------------------------------------*/

void QQBaseTime (int dummy)
{
  rdtsc (&(qq->TBase));
}

/*---------------------------------------------------------------------------*/

int QQAddState (char *name)
{
  return (QQAddName (name, QQ_STATE));
}

/*---------------------------------------------------------------------------*/

int QQAddMark (char *name)
{
  return (QQAddName (name, QQ_MARK));
}

/*---------------------------------------------------------------------------*/

int QQAddCount (char *name)
{
  return (QQAddName (name, QQ_COUNT));
}

/*---------------------------------------------------------------------------*/

int QQAddValue (char *name)
{
  return (QQAddName (name, QQ_VALUE));
}

/*---------------------------------------------------------------------------*/

static int QQAddName (char *name, int type)
{
  int i, rc, key;
  char *tmp;

//printf ("QQAddName, nkeys = %d\n", qq->nkeys);
  if (qq->nkeys > MAXKEYS)
  {
    printf ("QQAddName: Error, key table is full\n");
    return (0);
  }

  rc  = -1;
  for (i = 0; i < qq->nkeys; i++)
  {
    if ((rc = strncmp (name, qq->keys [i].Name, KEYLEN-1)) == 0)
    {
      printf ("QQAddName: Error, key '%s' already in table\n", name);
      break;
    }
  }

  if ((rc != 0) && (qq->nkeys < qq->keysize))
  {
    key = qq->nkeys;
    qq->keys [key].Key  = key;
    qq->keys [key].Type = type;
    qq->keys [key].Name = (char *) calloc (KEYLEN, sizeof (char));
    strncpy (qq->keys [key].Name, name, KEYLEN);
    qq->nkeys++;
  }
  return (key);
}

/*---------------------------------------------------------------------------*/

void QQStateOn (int key)
{
  if (qqPtr < qqTop) 
  {
    qqPtr->Key       = key;
    qqPtr->Val.State = QQ_ON;
    rdtsc (&(qqPtr->Time));
    qqPtr++;
  }
}

/*---------------------------------------------------------------------------*/

void QQStateOff (int key)
{
  if (qqPtr < qqTop) 
  {
    qqPtr->Key       = key;
    qqPtr->Val.State = QQ_OFF;
    rdtsc (&(qqPtr->Time));
    qqPtr++;
  }
}

/*---------------------------------------------------------------------------*/

void QQMark (int key)
{
  if (qqPtr < qqTop) 
  {
    qqPtr->Key = key;
    rdtsc (&(qqPtr->Time));
    qqPtr++;
  }
}

/*---------------------------------------------------------------------------*/

void QQCount (int key, int count)
{
  if (qqPtr < qqTop) 
  {
    qqPtr->Key       = key;
    qqPtr->Val.Count = count;
    rdtsc (&(qqPtr->Time));
    qqPtr++;
  }
}

/*---------------------------------------------------------------------------*/

void QQValue (int key, double value)
{
  if (qqPtr < qqTop) 
  {
    qqPtr->Key       = key;
    qqPtr->Val.Value = value;
    rdtsc (&(qqPtr->Time));
    qqPtr++;
  }
}

/*---------------------------------------------------------------------------*/

void QQRecord (int state)
{
  if (state == QQ_OFF)
  {
    if (qq->record)            /* If recording is currently on, save pointer */
    {
      qqSave = qqPtr;
      qqPtr = qqTop;           /* and set it above top to disable recording */
    }
    qq->record = FALSE;
  }
  else                         /* To turn back on, simply restore pointer */
  {
    qqPtr = qqSave;
    qq->record = TRUE;
  }
}

/*---------------------------------------------------------------------------*/

#include <mpi.h>

void QQOut (char *name, int node, int nnodes)
{
  FILE *out, *proc;
  MPI_Status status;
  QQItem *q;
  QQNode qn, *qnlist;
  double MHz;
  unsigned long long tbase;
  int i, size, len, tag, nkeys, num, nsend, bytes, count, total;
  bool found;
  char fname [128], line [256], *s, *buf;

  tag = 8952;
  if (qq->record) qqSave = qqPtr;

  qn.offset = 0;
  qn.TBase  = qq->TBase;
  qn.nItems = qqSave - qq->table;
  printf ("QQOut: nitems = %d\n", qn.nItems);

/* Get the node's MHz and send to root */

  proc = fopen ("/proc/cpuinfo", "r");

  found = false;
  while (!found && (fgets (line, 255, proc) != NULL))
  {
    if (strncmp (line, "cpu MHz", 7) == 0)
    {
      num = 8;
      s = line + num;
      while (!isdigit (*s) && (num < 255))
      {
        s++;
        num++;
      }

      sscanf (s, "%lf", &(qn.MHz));
      printf ("Read MHz = %f\n", qn.MHz);
      found = true;
    }
  }
  fclose (proc);


  if (node == 0)
  {
    qnlist = (QQNode *) calloc (nnodes, sizeof (QQNode));

    out = fopen (name, "w");

    size = KEYLEN;
    fwrite (&(qq->nkeys), sizeof (int), 1, out);
    fwrite (&size,        sizeof (int), 1, out);
    for (i = 0; i < qq->nkeys; i++)
    {
      fwrite (&(qq->keys [i].Key),  sizeof (int),       1, out);
      fwrite (&(qq->keys [i].Type), sizeof (int),       1, out);
      fwrite (qq->keys [i].Name,    sizeof (char), KEYLEN, out);
    }
  }
  else
  {
    qnlist  = NULL;
  }

  len = sizeof (QQNode);
  MPI_Gather (&qn, sizeof (QQNode), MPI_BYTE, qnlist, sizeof (QQNode), MPI_BYTE, 0, MPI_COMM_WORLD);
  printf ("gather done\n");

  if (node == 0)
  {
    fwrite (&nnodes, sizeof (int), 1, out);

    qnlist [0].offset = ftell (out) + nnodes * sizeof (QQNode);
    for (i = 1; i < nnodes; i++)
    {
      qnlist [i].offset += qnlist [i-1].offset + qnlist [i-1].nItems * sizeof (QQItem);
    }

    total = 0;
    for (i = 0; i < nnodes; i++)
    {
      printf ("QQOut %2d: offset %8d, nItems %8d, MHz = %8.2f\n",
              i, qnlist [i].offset, qnlist [i].nItems, qnlist [i].MHz);

      total += qnlist [i].nItems * sizeof (QQItem);
    }

    fwrite (qnlist, sizeof (QQNode), nnodes, out);

    fseek (out, qnlist [0].offset, 0);
    fwrite (qq->table, sizeof (QQItem), qnlist [0].nItems, out);
    count = qnlist [0].nItems * sizeof (QQItem);
    printf ("count = %d, total = %d\n", count, total);

    size = 1000 * sizeof (QQItem);
    buf = (char *) calloc (size, sizeof (char));
    while (count < total)
    {
      MPI_Recv (buf, size, MPI_BYTE, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
      printf ("Received\n");
      MPI_Get_count (&status, MPI_BYTE, &bytes);
      fseek (out, qnlist [status.MPI_SOURCE].offset, 0);
      fwrite (buf, sizeof (char), bytes, out);
      qnlist [status.MPI_SOURCE].offset += bytes;

      count += bytes;
      printf ("count = %d, total = %d, bytes = %d\n", count, total, bytes);
    }
    fclose (out);
  }
  else
  {
    q = qq->table;
    nsend = qn.nItems;
    while (nsend > 0)
    {
      num = (nsend < 1000) ? nsend : 1000;
      nsend -= num;
      MPI_Send (q, num * sizeof (QQItem), MPI_BYTE, 0, tag, MPI_COMM_WORLD);
      printf ("Send %d items, %d bytes\n", num, num * sizeof (QQItem));
      q += num;
    }
  }
}

/*---------------------------------------------------------------------------*/

//void QQOut (char *name, int node)
//{
//  FILE *out;
//  QQItem *q;
//  int i, size;
//  char fname [128];
//
//  sprintf (fname, "%s.%d", name, node);
//  out = fopen (fname, "w");
//
//  size = KEYLEN;
//  fwrite (&(qq->nkeys), sizeof (int), 1, out);
//  fwrite (&size,        sizeof (int), 1, out);
//  for (i = 0; i < qq->nkeys; i++)
//  {
//    fwrite (&(qq->keys [i].Key),  sizeof (int),       1, out);
//    fwrite (&(qq->keys [i].Type), sizeof (int),       1, out);
//    fwrite (qq->keys [i].Name,    sizeof (char), KEYLEN, out);
//  }
//
//  if (qq->record)
//    qqSave = qqPtr;
//  size = qqSave - qq->table;
//
//  fwrite (&(qq->TBase), sizeof (unsigned long long), 1, out);
//  fwrite (&size,        sizeof (int), 1, out);
//  fwrite (qq->table,    sizeof (QQItem), size, out);
//  fclose (out);
//}

/*---------------------------------------------------------------------------

QQ Output File Format (parallel)


  nkeys (int) - Number of keys in table, assumes table the same on all nodes.
  keylen (int) - Length of key name string

  key table - for each key:
    key # (int) - Index of key (0...nkeys-1, 0 is reserved)
    key type (int) -
    key name (keylen * char) -

  nnodes (int) - number of nodes 

  node table - for each node
    offset (size_t) - index in file at which data for the node starts
    size (int) - number of entries for node
    Tbase (unsigned long long) - Base tick count for node
    MHz (double) - frequency of node, in MHz (from /proc/cpuinfo)

  data - for each node
    item (size * QQItem) - data entries

---------------------------------------------------------------------------*/

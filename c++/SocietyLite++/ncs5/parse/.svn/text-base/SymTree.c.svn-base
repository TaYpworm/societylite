/* Takes a pointer to the input structure, and creates a symbol table tree */
/* from it.  The names are checked for uniqueness, etc.  Each node in the  */
/* tree contains a pointer back to the element from which it was defined,  */
/* so it can be used for lookup.                                           */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "SymTree.h"

#define NKINDS 18      /* The number of different kinds of elements */

static int ndup;       /* Number of duplicate names found, error if not 0 */
static int nerr;       

/*--------------------------------------------------------------------------
This routine makes a tree of all the element names in the input file.  It 
checks to see whether any duplicates are found (an error!)
--------------------------------------------------------------------------*/

TREENODE *makeSymTree (TMP_INPUT *TIN)
{
  TREENODE *Root, *dup;
  T_ANYTHING *A, *Adup, *ALL [NKINDS];
  int i, rc, nkinds;

/*  Because the element structs all begin with the same members, and all we
    want to access here are the common members, we can cast them all to a 
    universal type, and so make the code a lot simpler */

  nkinds = 0;
  ALL [nkinds++] = (T_ANYTHING *) TIN->Csh;
  ALL [nkinds++] = (T_ANYTHING *) TIN->Column;
  ALL [nkinds++] = (T_ANYTHING *) TIN->Lsh;
  ALL [nkinds++] = (T_ANYTHING *) TIN->Layer;
  ALL [nkinds++] = (T_ANYTHING *) TIN->Cmp;
  ALL [nkinds++] = (T_ANYTHING *) TIN->Cell;
  ALL [nkinds++] = (T_ANYTHING *) TIN->Channel;
  ALL [nkinds++] = (T_ANYTHING *) TIN->Synapse;
  ALL [nkinds++] = (T_ANYTHING *) TIN->SynFD;
  ALL [nkinds++] = (T_ANYTHING *) TIN->SynPSG;
  ALL [nkinds++] = (T_ANYTHING *) TIN->SynLearn;
  ALL [nkinds++] = (T_ANYTHING *) TIN->SynData;
  ALL [nkinds++] = (T_ANYTHING *) TIN->SynAugmentation;
  ALL [nkinds++] = (T_ANYTHING *) TIN->Spike;
  ALL [nkinds++] = (T_ANYTHING *) TIN->Stimulus;
  ALL [nkinds++] = (T_ANYTHING *) TIN->StInject;
  ALL [nkinds++] = (T_ANYTHING *) TIN->Report;

  ndup = nerr = 0;
  Root = NULL;

/* Make the root node from the single BRAIN element */

  if (TIN->Node == 0) 
    printf ("Creating symbol table for brain '%s'\n", TIN->Brain->L.name);

  A = (T_ANYTHING *) TIN->Brain;
  Root = makeTreeNode (TIN->Brain->L.name, (void *) A);
  if (Root == NULL)
  {
    printf ("Error creating Root node\n");
    return (NULL);
  }

/* Insert the names of all the elements into the symbol table tree */

  if (TIN->Node == 0) printf ("Adding names to symbol table...\n");

  for (i = 0; i < nkinds; i++)
  {
    A = ALL [i];
    while (A != NULL) 
    {
      rc = insertTree (Root, A->L.name, (void *) A);
      if (!rc)
      {
        dup = findTree (A->L.name, Root);
        Adup = (T_ANYTHING *) dup->data;
        if (TIN->Node == 0) 
          printf ("Duplicate name '%s' at line %d, first defined at line %d\n",
                  A->L.name, A->L.line, Adup->L.line);
        ndup++;
      }
      A = (T_ANYTHING *) A->L.next;
    }
  }

  if (ndup > 0)
  {
    if (TIN->Node == 0) 
      fprintf (stderr, "Error: %d duplicate names found\n", ndup);
    return (NULL);
  }
  return (Root);
}

/*--------------------------------------------------------------------------*/
/* Prints a header for the symbol table list, then calls printree           */

TreePrint (TREENODE *node)
{
  printf ("Line #     Element Type     Index     Name\n\n");
  printtree (node);
}

/*--------------------------------------------------------------------------*/

printtree (TREENODE *node)
{
  T_ANYTHING *data;

  if (node->left != NULL)
    printtree (node->left);
  data = (T_ANYTHING *) node->data;
  printf ("%6d: %-20s %5d - %s\n", data->L.line, TK2name (data->L.kind), data->L.idx, node->key);
  if (node->right != NULL)
    printtree (node->right);
}

/*--------------------------------------------------------------------------*/
/* Return the name table index of name */

int getTreeIndex (char *name, TREENODE *root)
{
  TREENODE *node;
  T_ANYTHING *A;
  int index;

  index = -1;
  if (name != NULL)
  {
    node = findTree (name, root);
    if (node != NULL)
    {
      A = (T_ANYTHING *) node->data;
      index = A->L.idx;
    }
  }
  return (index);
}

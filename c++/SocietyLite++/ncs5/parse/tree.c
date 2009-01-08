/* Low-level routines for implementing a generic binary tree.  The tree is    */
/* keyed on a character string (char *), and may hold pointers to structures  */
/* of any type, cast to void *                                                */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tree.h"
//#include "../memstat.h"

//#define MEM_KEY KEY_INPUT


/*--------------------------------------------------------------------------*/

insertTree (TREENODE *node, char *name, void *A)
{
  int cmp, duplicate, inserted;

  duplicate = inserted = FALSE;
  while (!inserted && (node != NULL))
  {
    cmp = strcasecmp (name, node->key);            /* First do case-insensitive comparison */
    if (cmp == 0) cmp = strcmp (name, node->key);  /* Only if those match is case used     */
  
    if (cmp == 0)
    {
      duplicate = inserted = TRUE;
    }
    else if (cmp < 0)
    {
      if (node->left == NULL)
      {
        node->left = makeTreeNode (name, (void *) A);
        inserted = TRUE;
      }
      else
        node = node->left;
    }
    else 
    {
      if (node->right == NULL)
      {
        node->right = makeTreeNode (name, (void *) A);
        inserted = TRUE;
      }
      else  
        node = node->right;
    }
  }
  return (!duplicate);
}

/*--------------------------------------------------------------------------*/

TREENODE *makeTreeNode (char *name, void *A)
{
  TREENODE *node; 

  node = (TREENODE *) malloc (sizeof (TREENODE));
  node->left = node->right = NULL;
  node->key  = name;
  node->data = A;

  return (node);
}

/*--------------------------------------------------------------------------*/
/* Return pointer to node corresponding to name, or NULL if not found       */

TREENODE *findTree (char *name, TREENODE *node)
{
  int cmp, found;

  found = FALSE;
  while (!found && (node != NULL))
  {
    cmp = strcasecmp (name, node->key);            /* First do case-insensitive comparison */
    if (cmp == 0) cmp = strcmp (name, node->key);  /* Only if those match is case used     */
  
    if (cmp == 0)
    {
      found = TRUE;
    }
    else if (cmp < 0)
    {
      node = node->left;
    }
    else 
    {
      node = node->right;
    }
  }
  return (node);
}

/*--------------------------------------------------------------------------*/
/* Returns an array of pointers to the data structures in the tree.  The    */
/* number of nodes in the tree must be passed as an argument (usually they  */
/* will be counted as the tree is created).                                 */

void *TreeToList (TREENODE *Root, int nNodes)
{
  void **list;
  int i;

/*printf ("In TreeToList, nNodes = %d\n", nNodes);  fflush (stdout);*/
  list = (void **) calloc (nNodes, sizeof (void *));

/*printf ("calling treelist...\n");  fflush (stdout);*/
  i = 0;
  treelist (Root, list, &i);

  return (list);
}

/*--------------------------------------------------------------------------*/

void treelist (TREENODE *node, void **list, int *i)
{
  if (node->left != NULL)
    treelist (node->left, list, i);
  list [*i] = node->data;
/*printf ("  treelist, list [%d] = %x\n", *i, node->data);  fflush (stdout);*/
  (*i)++;
  if (node->right != NULL)
    treelist (node->right, list, i);
}

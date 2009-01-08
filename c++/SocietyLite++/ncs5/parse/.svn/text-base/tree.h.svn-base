/* Define tree node structure for name table */

#ifndef TREE_DEF
#define TREE_DEF 1

#define TRUE  1
#define FALSE 0

typedef struct Treenode
{
  struct Treenode *left;
  struct Treenode *right;
  char *key;     /* Pointer to string that serves as key */
  void *data;    /* Pointer to the data for this node */
} TREENODE;

TREENODE *findTree (char *, TREENODE *);
TREENODE *makeTreeNode (char *, void *);
int insertTree (TREENODE *, char *, void *);
void *TreeToList (TREENODE *, int);
void treelist (TREENODE *, void **, int *);

#endif

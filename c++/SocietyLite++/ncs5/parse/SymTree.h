/* */
/* Define tree node structure for name table */

#ifndef SYMTREE_DEF
#define SYMTREE_DEF 1

#include "input.h"
#include "tree.h"

TREENODE *makeSymTree (TMP_INPUT *);
int getTreeIndex (char *, TREENODE *);

#endif

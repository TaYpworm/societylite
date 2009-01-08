/* Functions used in managing lists of input items */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "input.h"
#include "list.h"

extern TMP_INPUT *TIN;       /* This must be global so parse.y can access it */


/*---------------------------------------------------------------------------*/
/* Creates & fills in a list entry.  Sets the Next pointer to the existing   */
/* list, and returns a pointer to the new entry, which is now the head of    */
/* the list.                                                                 */

LIST *AddName (LIST *List, char *name, int Qty)
{
  LIST *Item;

  Item = (LIST *) malloc (sizeof (LIST));

  Item->L.next = List;
  Item->L.name = strdup (name);
  Item->L.file = TIN->file;
  Item->L.line = TIN->line;
  Item->Qty  = Qty;
  Item->label = NULL;

//printf ("AddName: name '%s'\n", Item->name);
  return (Item);
}

/*---------------------------------------------------------------------------*/
/* Does the same for a compartment list entry.                               */

LIST *AddCmp (LIST *List, char *name, char *label, double x, double y, double z)
{
  LIST *Item;

  Item = (LIST *) malloc (sizeof (LIST));

  Item->L.next  = List;
  Item->L.name  = strdup (name);
  Item->L.file  = TIN->file;
  Item->L.line  = TIN->line;
  Item->label = strdup (label);
  Item->x     = x;
  Item->y     = y;
  Item->z     = z;

//printf ("AddName: name '%s'\n", Item->name);
  return (Item);
}

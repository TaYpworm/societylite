/* Structures used to hold lists of names of things that are part of a brain */
/* element.  For instance, a column would have a lIST of the Layers, each    */
/* layer a list of cells, and so on.  These lists are only used in parsing:  */
/* the names are then converted into integer indexes for efficiency.         */

#ifndef LIST_DEF
#define LIST_DEF 1

#include "../Locator.h"

/**
 * Object to be used as node in a linked list.
 * The Locator contained in this object is used for the actual linking since
 * that locator contains a pointer to the next LIST object.
 */
struct LIST
{
          ///General information about this node and links to the next node
  LOCATOR L;
          ///Used by T_LAYER to track how many instances of a particular T_CELL to create
  int Qty;
          ///Used by T_CELL to assign a unique label to a T_CMP
  char *label;
          ///Used by T_CELL to assign a T_CMP to an x-coordinate within a cell
  double x;
          ///Used by T_CELL to assign a T_CMP to an y-coordinate within a cell
  double y;
          ///Used by T_CELL to assign a T_CMP to an z-coordinate within a cell
  double z;
};
typedef struct LIST LIST;

#endif

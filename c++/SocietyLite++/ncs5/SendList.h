#ifndef SENDLIST_H
#define SENDLIST_H

/**
 * This structure holds the information that specifies the destination of a
 * synapse firing.  Each compartment has a list of these
 */
struct SENDITEM
{
          ///Destination Node
  int Node;
          ///Memory address of object to receive msg
  void *Addr;
          ///Ticks from now to deliver msg - must be at least 1!
  int Delay;
};
typedef struct SENDITEM SENDITEM;

#endif 

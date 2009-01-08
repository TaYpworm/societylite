#ifndef LOCATOR_H
#define LOCATOR_H

/**
 * This struct is a sub-structure of all elements that are read from the
 * input.  It holds the name, file & line info for error reporting, and a
 * linked list pointer.  It allows for e.g.  common lookup & error reporting
 * functions.
 */
struct LOCATOR
{
  ///Flag for type of structure?
  int kind;
  ///Name of item.  Assigned from input file, usually by TYPE keyword.
  char *name;
  ///Index number = count as read in.
  int idx;
  ///Linked-list pointer
  void *next;

  ///Future: file name where the object was defined
  char *file;
  ///The line of the input file where the object was defined
  int line;
};
typedef struct LOCATOR LOCATOR;

#endif 

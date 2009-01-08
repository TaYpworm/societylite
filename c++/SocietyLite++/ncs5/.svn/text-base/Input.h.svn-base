#ifndef INPUT_H
#define INPUT_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

#include "Brain.h"
#include "Stimulus.h"
#include "InputInfo.h"
#include "defines.h"
#include "parse/arrays.h"


class Input
{
 public:

  MPI_Comm InputComm;       /* Separate MPI group & communicator for messages */
  MPI_Group Group;             
  

  Input ();
  ~Input ();

  FILE *getAutoPort( char *name);
  void *MakeInput (char *, int, int);
  void Read (void *, void *);
  void FreeInput (void *);
};

#endif 

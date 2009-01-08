#ifndef CELL_H
#define CELL_H

/* The Cell object is simply a container for the compartments that make up   */
/* the cell.  It just holds a list of those compartments, and loops through  */
/* them at each timestep.                                                    */

#include "Compartment.h"
#include <vector>
#include <queue>
#include <deque>

using namespace std;

//Doc++ include statement
//@Include: Compartment.h

using namespace std;

class Compartment;
class Synapse;

/**
 * The Cell object is simply a container for the compartments that make up
 * the cell.  It just holds a list of those compartments, and loops through 
 * them at each timestep.
 */
class Cell
{
 public:
          ///The identifier of the cell on this machine: 0...nCells - 1. Node & CellID form a unique identifier
  int CellID;
          ///This is index into the ARRAYS struct of the definition from which this object was created
  int idx;
          ///Number of compartments that compose this cell
  int nCompartments;
          ///Array of pointers to this cell's compartments
  Compartment **Compartments;

  //Functions

  Cell();
  Cell (int, int, int, int); 
  ~Cell ();      
        
  void DoProcessCell (int); 

  /**
   * Write cell Data to a file. This is only the first half of the save
   * since connection info between compartments must be saved later.
   * This function saves data about the individual cell and its compartments as
   * well as synapse data.
   */
  int Save (FILE *out);
  /**
   * Finish writing cell data to a file. This is the second half of saving cell
   * data. It is called after all individual cell and compartments have been written.
   * This is important to the loading of a brain state since memory addresses will need
   * to be assigned and that cannot be done until all memory has been allocated.
   * @param out File pointer to destination
   * @return number of bytes written
   */
  int SaveConnections( vector <Synapse *> &collection, int targetNode ); //FILE *out );
  void CellReset (FILE *, char *);
  /**
   * Load Cell data from a file. This is only the first half of loading since connection
   * info for this cell cannot be loaded until all cells have been loaded.
   * @param in input file stream
   * @param nothing dummy string for moving pointers to
   */
  void Load (FILE *in, char *nothing);
  /**
   * Finish loading cell data. This function is called after all initial cell data
   * has been loaded.  This is important because the connection data this function loads
   * cannot be used until all memory has been allocated.  With all objects allocated,
   * pointers to synapse memory addresses can be properly assigned
   * @param collection STL queue with synapse memory addresses.
   * @param targetNode the Node ID which the synapse memory addresses are from
   */
  void LoadConnections( deque<Synapse *> &collection, int targetNode );
};

#endif

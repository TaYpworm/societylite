#ifndef BRAIN_H
#define BRAIN_H

#include "defines.h"
#include "Cell.h"
#include "Stimulus.h"
#include "Report.h"
#include "SynapseDef.h"
#include <vector>
#include <mpi.h>

using namespace std;

class Cell;
class Stimulus;
class Report;
class SynapseDEF;

//Includes for doc++
//@Include: Cell.h
//@Include: NodeInfo.h
//@Include: CellManager.h
//@Include: Connector.h
//@Include: MessageBus.h
//@Include: Report.h
//@Include: Random.h

/**
 * This is the core of the brain.  It contains lists of the objects, such as
 * Cells, that the brain contains (which in turn may contain other lists of
 * their internal components).  The code is basically a loop that runs
 * through the Stimulus, Cell, and Report lists every timestep, and calls
 * them to do whatever they do.
 */
class Brain
{
 public:
          ///Simulation parameters parsed from the input file to control the simulation behavior
  int flag;
          ///total time to run the program - seconds - input from user
  double TotalTime;
          /// Ticks to run.  Note: TimeStep values must be int, since negative values are used as flags
  int CycleCount;
          ///Current timestep of the DoThink loop
  int TimeStep;
          ///Time to do Rich's status reports?
  int ReportTime;
          ///Actual time in seconds when the program started the main loop - not used
  int StartTime;
          ///Number of neurons on this node
  int nCells;
          ///List of pointers referencing all Cells on this node
  Cell **Cells;
          ///Cell Coordinates.  Only used if synaptic distances turned on.  Can be freed to save memory
  double *CellPosition[3];
  
/* Reports and Stimulii are now kept as linked lists, so that they may be    */
/* added & deleted as desired.                                               */
          ///Number of stimulus objects currently defined
  int nStimulus;
          ///Linked list of Stimulus objects
  Stimulus *StimList;
          ///Number of report objects currently defined
  int nReports; 
          ///Linked List of Reports objects
  Report *RptList;

/* synapse definitions */

          ///Number of Synapse Definitions
  int nSynapseDef;
          ///List of pointers to Synapse Definitions
  SynapseDef **SynapseDefs;

  /* variables below do not need to be saved */

          ///If set to true, the state of all brain objects should be written to a file at end of timestep
  bool SaveFlag;
          ///If set to true, the state of all brain objects should be read and reset based on file
  bool ResetFlag;
          ///Automatically save brain state to file when it reaches this time
  double SaveTime;
          ///Name of file to save to when automatically saving
  char *SaveFile;
          ///port for automatically connecting reports/stims/and the brain
  int HostPort;
          ///port for receiving outside commands
  int Port;
          ///hostname where this Brain will recv commands from
  char *HostName;
          ///socket descriptor opened if taking runtime commands
  int sd;
          ///FILE Pointer to use instead of sd
  FILE *input;
          ///Pointer to hold pending commands meant for later timesteps
  char *pendingCommands;
          ///length of pendingCommands
  int pendingLength;
          ///Communicator for distributing runtime commands
  MPI_Comm commandComm;
          ///Group for distributing runtime commands
  MPI_Group commandGroup;

  //------------------------ functions --------------------------------------

  Brain (char *, char *);  // Constructor for loading from Brain file
  Brain (T_BRAIN *);        
  ~Brain (); 

  int DoThink ();

  /**
   * Save the current state of the brain to a file. Recursively saves data
   * from each subobject of the brain - cells, compartments, channels, etc.
   * does not save reports or stimulus; these will be defined new when the
   * brain is loaded.
   */
  void Save (char *);
  /**
   * Restore data that was kept in the arrays structure. This should be done
   * first and separately, so that the data can be ready and available for
   * the rest of loading that will be done later.
   * @param filename name of save file containing all ncs data
   */
  void LoadArrays( char *filename );
  /**
   * Load a brain and its last state from a file. Recursively creates and loads data
   * into substructures - cells, compartments, channels, etc.
   * @param filename name of save file containing all ncs data
   * @param nothing character pointer to be used as a place holder, will eventually be removed when it is no longer needed
   */
  void Load (char *filename, char *nothing );
  void Reset (char *, char *);

  /**
   * Checks for any runtime commands and distributes them among all nodes.
   * @return integer to indicate result of command receipt and any subsequent execution
   */
  int CommandCheck();

   /**
   * Takes the command buffer and breaks it up into individual words
   * (separated by spaces and/or tabs) and calls CommandExec on individual
   * commands (seperated by semi-colon).  If commands meant for different
   * timesteps are read in at the same time, they will be separated by newlines.
   * @param commandBuffer charater string containing the received command
   * @return result code of the last executed command
   */
  int CommandParse( char *commandBuffer );

  /**
   * When a command is parsed, this function determines which other
   * function to call that will perform the required action.
   * @param commandBuffer charater string containing the received command
   * @return result code of the executed command
   */
  int CommandExec( vector<char *> &result );
  
  //Runtime Commands

  /**
   * Changes the Hebbian learning mode of the specified Synapse
   *  Format
   *   SetHebbian <syanpse type> < NONE | BOTH | +HEBBIAN | -HEBBIAN >
   * @param result all words parsed from command buffer.  Should contain information
   *  to locate synapse and the new mode it should be set.
   * @return 0 on success, -1 on failure
   */
  int CommandHebbian( vector<char*> &result );

  /**
   * User has requested the simulator to terminate.  All reports will be flushed and
   * closed and ncs will break out of the DoThink loop so that it can shut down properly.  Hopefully.
   * @param result all words parsed from command buffer.  Not needed in this function, but passed for consistency
   * @return integer value 15 to notify the DoThink loop it should call break.
   */
  int CommandExit( vector<char*> &result );

  /**
   * User wants to add another start/end pair to an existing stimulus.
   * format:
   *  APPENDSTIM [-flags] <stim type> <TStart> [TStop]
   *  flags are optional, but must all be included as the second item.  If no flags
   *  are needed, then the stim type can go immediatly after the command.  Currently
   *  implemented flags include:
   *    a : Automatic Time Stop.  If this flag is specified, then the TStop argument
   *        should not be sent because the stimulus will be made to last 2.5 milliseconds.
   *    t : Timesteps will be sent.  Instead of taking TStart (and TStop, if applicable ) and
   *        multiplying by FSV, the values sent are used directly.
   *    s : Seconds will be sent (default).  After receiving TStart (and TStop, if applicable )
   *        they will be multiplied by FSV to calculate the appropriate TimeStep.
   * Multiple stim types can be affected if desired by following the final parameter with
   * white space (i.e. space or tab) then another set of parameters.
   * ex. APPENDSTIM -a Stim1 3.4 Stim2 3.4
   * ex. APPENDSTIM -t Stim1 34000 35000 Stim2 34000 37000
   * @param result all words parsed from command buffer. Should contain flags, name of
   *  stimuli and start/end time pairs (in seconds by default)
   * @return 0 on success, -1 on failure
   */
  int CommandStim( vector<char*> &result );
  
    /**
   * User wants to add another start/end pair to an existing report.
   * format:
   *  APPENDREPORT [-flags] <report type> <TStart> [TStop]
   *  flags are optional, but must all be included as the second item.  If no flags
   *  are needed, then the stim type can go immediatly after the command.  Currently
   *  implemented flags include:
   *    a : Automatic Time Stop.  If this flag is specified, then the TStop argument
   *        should not be sent because the report will be made to last 2.5 milliseconds.
   *    t : Timesteps will be sent.  Instead of taking TStart (and TStop, if applicable ) and
   *        multiplying by FSV, the values sent are used directly.
   *    s : Seconds will be sent (default).  After receiving TStart (and TStop, if applicable )
   *        they will be multiplied by FSV to calculate the appropriate TimeStep.
   * @param result all words parsed from command buffer. Should contain flags, name of
   *  report and start/end time pairs (in seconds)
   * @return 0 on success, -1 on failure
   */
  int CommandReport( vector<char*> &result );

  /**
   * Port Command to instruct brain state to be written at current timestep.
   * Format:
   *  SAVE [filename]
   * filename is optional as long as a save file was given in the input file already.
   * When a filename is confirmed to exist, either in the command or from
   * the input file, the Save function is called.
   * @param result all words parsed from command buffer. Should contain just "SAVE" and a filename if applicable
   * @result 0 on success, -1 on failure (i.e. no valid filename found)
   */
  int CommandSave( vector<char*> &result );

  /**
   * If this brain is to receive dynamic commands, the port based
   * communication needs to be setup before the doThink loop gets underway.
   * This function will setup that communication for either a specify host/port
   * or use a server's auto port capabilities if Port is -1.
   */
  FILE *setupPort();

};

#endif

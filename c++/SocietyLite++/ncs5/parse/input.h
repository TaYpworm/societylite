/* Defines structures for input of brain input elements.  Part of the parse */
/* module.  See comments at end of file                                     */

#ifndef INPUT_DEF
#define INPUT_DEF 1

/* Definition of STRLEN needed for parse.y and scan.l. */

#ifndef STRLEN
#define STRLEN 128
#endif

#include "../InitStruct.h"

/*
 * This is a temporary structure which holds the linked lists of input brain
 * elements, plus some general information about the input.  It be deleted
 * once the ConvertIn step has been done.
 */
struct TMP_INPUT
{
          ///For parallel
  int Node;
          ///Name of current input file being read from when the INCLUDE statement is used
  char *file;
          ///Line of file currently being read
  int line;
          ///Column reading from - not implemented
  int col;
          ///Number of brain elements read from input
  int nElements;
          ///Number of parse errors encountered
  int nParseErr;
          ///Number of data errors encountered
  int nDataErr;
          ///Number of brain objects defined - must only be one when parsing completes
  int nBrain;
          ///Pointer to brain object
  T_BRAIN *Brain;
          ///Number of Column shell objects
  int nCsh;
          ///Linked list of Column Shell objects
  T_CSHELL *Csh;
          ///Number of Column objects
  int nColumn;
          ///Linked list of Column objects
  T_COLUMN *Column;
          ///Number of Layer Shell objects
  int nLsh;
          ///Linked list of Layer Shell objects
  T_LSHELL *Lsh;
          ///Number of Layer objects
  int nLayer;
          ///Linked list of Layer objects
  T_LAYER *Layer;
          ///Number of Cell objects
  int nCell;
          ///Linked list of Cell objects
  T_CELL *Cell;
          ///Number of Compartment objects
  int nCmp;
          ///Linked list of Compartment objects
  T_CMP *Cmp;
          ///Number of Channel objects
  int nChannel;
          ///Linked list of Channel objects
  T_CHANNEL *Channel;
          ///Number of Synapse objects
  int nSynapse;
          ///Linked list of Synapse objects
  T_SYNAPSE *Synapse;
          ///Number of Synaptic Learning objects
  int nSynLearn;
          ///Linked list of Synaptic Learning objects
  T_SYNLEARN *SynLearn;
          ///Number of Synaptic Facilitation/Depression objects
  int nSynFD;
          ///Linked list of Synaptic Facilitation/Depression objects
  T_SYN_FD *SynFD;
          ///Number of Post-Synaptic Waveform objects
  int nSynPSG;
          ///Linked list of Post-Synaptic Waveform objects
  T_SYNPSG *SynPSG;
          ///Number of Synaptic Data objects (deprecated)
  int nSynData;
          ///Linked list of Synaptic Data objects (deprecated)
  T_SYNDATA *SynData;
          ///Number of Synaptic Augmentation objects
  int nSynAugmentation;
          ///Linked List of Synaptic Augmentation objects
  T_SYNAUGMENTATION *SynAugmentation;
          ///Number of Spike Shape template objects
  int nSpike;
          ///Linked list of Spike Shape template objects
  T_SPIKE *Spike;
          ///Number of Stimulus objects
  int nStimulus;
          ///Linked list of Stimulus objects
  T_STIMULUS *Stimulus;
          ///Number of Stimulus Injection objects
  int nStInject;
          ///Linked list of Stimulus Injectin objects
  T_STINJECT *StInject;
          ///Number of Report objects
  int nReport;
          ///Linked list of Report objects
  T_REPORT *Report;
};
typedef struct TMP_INPUT TMP_INPUT;

/* function definitions */

T_BRAIN     *makebrain ();
T_CSHELL    *makecsh ();
T_COLUMN    *makecolumn ();
T_LSHELL    *makelsh ();
T_LAYER     *makelayer ();
T_CELL      *makecell ();
T_CMP       *makecmp ();
T_CHANNEL   *makechan (char *);
T_SYNAPSE   *makesynapse ();
T_SYN_FD    *makesyn_fd ();
T_SYNLEARN  *makesyn_learn ();
T_SYNDATA   *makesyn_data ();
T_SYNAUGMENTATION *makesyn_augmentation ();
T_SYNPSG    *makesyn_psg ();
T_SPIKE     *makespike ();
T_STINJECT  *makesti ();
T_STIMULUS  *makestim ();
T_REPORT    *makereport ();

T_CONNECT *makeConnect (T_CONNECT *, int *,
                        char *, char *, char *, char *,
                        char *, char *, char *, char *,
                        char *, double, double);
                        
/**
 * Define a connection with decaying probability.  These connections
 * use the distance between the cells to determine the probability that
 * any two cells will be connected.  As distance increases, the probability
 * of a connection being made decreases.
 *
 * @param Con Pointer to linked list of already created connections where this new one will be appended
 * @param idx Pointer to the count of connections made, to be incremented here
 * @param fromCol Column Name of the source cluster
 * @param fromLay Layer Name of the source cluster
 * @param fromCel Cell Name of the source cluster
 * @param fromCmp Compartment Name used within the source cluster
 * @param toCol Column Name of the destination cluster
 * @param toLay Layer Name of the destination cluster
 * @param toCel Cell Name of the destination cluster
 * @param toCmp Compartment Name used within the destination cluster
 * @param synName Name of Synapse to use when making the connection
 * @param probability Max Probability if cells had no distance (or distance is turned off)
 * @param stepCurrent Distance used in exponential decay of probability: Max_P*exp( distance/stepCurrent )
 * @param speed Speed of transmission along synapse, used to determine number of timesteps before a spike arrives at destination cell
 */
T_CONNECT *makeDecayingConnect (T_CONNECT *Con, int *idx,
                        char *fromCol, char *fromLay, char *fromCel, char *fromCmp,
                        char *toCol, char *toLay, char *toCel, char *toCmp,
                        char *synName, double probability, double stepCurrent, double speed );
                        
/**
 * Define two existing connections as recurrent with one another.  When
 * the Brain is constructed, it will make a certain percentage of connections
 * occur between the same cells.  For example, if cell A-12 connects to cell B-34,
 * then B-34 will also connect to cell A-12 with a specified probability.
 *
 * @param Con Pointer to linked list of recurrent connections where this new one will be appended
 * @param idx Pointer to the count of recurrent connections defined, to be incremented here
 * @param ACol Column name of one of the cell clusters
 * @param ALay Layer name of one of the cell clusters
 * @param ACel Cell name of one of the cell clusters
 * @param ACmp Compartment name used within one of the cell clusters
 * @param BCol Column name of the other cell cluster
 * @param BLay Layer name of the other cell cluster
 * @param BCel Cell name of the other cell cluster
 * @param BCmp Compartment name used within the other cell cluster
 * @param synName Name of Synapse used by the base connections (must be same on both)
 * @param AtoB Probability that a connection from A to B also connects back from B to A
 * @param BtoA Probability that a connection from B to A also connects back from A to B
 * @return Pointer to the new front of linked list of recurrent connections
 */
T_RECURRENT *makeRecurrentConnect (T_RECURRENT *Con, int *idx,
                        char *ACol, char *ALay, char *ACel, char *ACmp,
                        char *BCol, char *BLay, char *BCel, char *BCmp,
                        char *synName, double AtoB, double BtoA );

T_CMPCONNECT *makeCmpConn (T_CMPCONNECT *, char *, char *, double, double, double);

char *TK2name (int);
#endif

/*-------------------------------------------------------------------------

This structure is used for initial input of brain definitions from the
input file.  As it scans through the input file, the parser creates a
linked list of each type of brain element, allocating a new object for
each new element it finds.  This temporary set of linked lists will then
be converted to arrays, in order to do efficient indexing & lookup.

To avoid conflicts, the element type is the code set in the #define statements
in parse.h (the header file output by yacc/bison).
-------------------------------------------------------------------------*/

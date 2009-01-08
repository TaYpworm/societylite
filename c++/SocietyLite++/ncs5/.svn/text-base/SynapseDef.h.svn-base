#ifndef SYNAPSEDEF_H
#define SYNAPSEDEF_H

/* This holds the data that makes up the definition of a synapse type.  It   */
/* includes much of what was in the old SynapseRSE, SynapsePSG, and          */
/* SynapseLearning objects, as well as some info from the synapse itself.    */
/* Basically, everything that is the same across some type of synapse is     */
/* included (or pointed to from) here, in order to minimize the size of      */
/* individual synapses.                                                      */

#include "InitStruct.h"
#include "defines.h"
#include "parse/arrays.h"
#include <stdio.h>

/**
 * This holds the data that makes up the definition of a synapse type.  It
 * includes much of what was in the old SynapseRSE, SynapsePSG, and
 * SynapseLearning objects, as well as some info from the synapse itself.
 * Basically, everything that is the same across some type of synapse is
 * included (or pointed to from) here, in order to minimize the size of
 * individual synapses.
 */
class SynapseDef
{
 public:
          ///For debugging and Reporting on Synapse properties
  T_SYNAPSE *source;
          /// may be FACIL, DEPR, BOTH, NONE
  int RSEType;
          ///These are pointers to [mean, stdev]
  double *TauDepr;
  double *TauFacil;
  double *SynDelay;
  double *SynReversal;  
          ///Length of IPSG or EPSG waveform template
  int PSGLen;
          ///Pointer to IPSG or EPSG waveform template
  double *PSGTemplate;

#ifdef USE_FLOAT
          ///Maximum Conductance
  float MaxG[2];
#else
  double *MaxG;
#endif

          ///Flag to indicate whether this synapse should receive Positive Hebbian learning
  bool LearnPos;
          ///Number of entries in the positive learning table
  int nPosLearn;
          ///Positive learning table.  Depending on conditions, a particular index is used
  double *PosLearnTable;
          ///Flag to indicate whether this synapse should recieve Negative Hebbian learning
  bool LearnNeg;
          ///Number of entries in the negative learning table
  int nNegLearn;
          ///Negative learning table. Depending on conditions, a particular index is used
  double *NegLearnTable;

  // functions *******************************

  /**
   *  Default Constructor. Used when creating an empty structure for Load op
   */
  SynapseDef();
  /**
   * Primary constructor
   * @param iSyn Intermediate object generated from input containing synapse template
   * @param AR User input data in parsed and structured format
   */
  SynapseDef (T_SYNAPSE *iSyn, ARRAYS *AR);
  /**
   * Destructor, notifies memory tracker that resourced have been freed
   */
  ~SynapseDef ();

  /**
   *  Set the learning mode of the synapse
   *  @param code The type of learning this synapse will now use: 0 = NONE, 1 = +HEBBIAN, 2 = -HEBBIAN, 3 = BOTH
   *  @return 0 on success, -1 on failure
   */
  int SetLearning( int code );

  /**
   * Write SynapseDef to file.  An important note is that the class variable
   * MaxG can be either a float or a double depending on the compiler options
   * turned on at the time of compile.  Therefore, if the version that saves
   * MaxG uses the float format, then the version that loads MaxG must also
   * use the float format.
   * @param out file stream to write data
   * @return number of bytes written
   */
  int Save( FILE *out );
  /**
   * Read SynapseDef from file.  An important note is that the class variable
   * MaxG can be either a float or a double depending on the compiler options
   * turned on at the time of compile.  Therefore, if the version that saves
   * MaxG uses the float format, then the version that loads MaxG must also
   * use the float format.
   * @param in file stream to read data from
   * @param nothing dummy pointer to use - will hopefully be removed later
   */
  void Load( FILE *in, char *nothing );

  /**
   * Determines where this Synapse Definition appears in TheBrain's list of
   * SynapseDefs.  Created to assist in Saving/Loading the Brain state.
   * @return index into TheBrain->SynapseDefs (0..nDefs-1)
   */
  int getID();
};

#endif

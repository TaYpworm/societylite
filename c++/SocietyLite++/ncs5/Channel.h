#ifndef CHANNEL_H
#define CHANNEL_H

#include <stdio.h>
#include <math.h>

#include "InitStruct.h"
#include "Compartment.h"
#include "Random.h"
#include "defines.h"

class Compartment;
class Random;

/**
 * Models the channels that are present in the cell membrane -
 * specifically the Sodium (Na), Calcium (Ca), and Potassium (K) channels.   
 * In Neurobiology, these channels are responsible for the release of        
 * neurotransmitters (Ca) into the synapse, changing the membrane voltage    
 * potential to be closer to an Action Potential (Na and K) or inhibiting an 
 * Action Potential (Cl).  
 *
 * This code can create an object that will model channels of several        
 * different types, depending on the parameters in its input object.  The    
 * type is determined by the Family keyword in the input.  Channels are      
 * owned by a compartment, which may own any number of different types of    
 * channels.                                                                 
 */
class Channel
{
  public:
          ///For debugging & traceback
  T_CHANNEL *source;
          ///Classification code for the kind of channel this object is.
  int family;
          ///Exponent that the activation particle (m) will be raised by during calculations
  double mPower;
          ///Exponent that the inactivation particle (h) will be raised by during calculations
  double hPower;
          ///Reversal Potential of the channel
  double ReversePot;
          ///User defined values that influence activation particle (m) update equations through subtration from the compartment's membrane voltage.
  double eHalfMinM [3]; //alpha and beta values or just M/H
          ///User defined values that influence inactivation particle (h) update equations through subtration from the compartment's membrane voltage..
  double eHalfMinH [3];
          ///Number of values contained in vTauVoltageM (vTauValueM contains 1 additional value)
  int nTauM;
          ///Array of voltages to be searched.  When the correct voltage is found, the corresponding entry in vTauValueM will be used in calculations
  double *vTauVoltageM;
          ///Array of tau values to be used in calculations involving the activation particle (m).
  double *vTauValueM;
          ///Number of values contained in vTauVoltageH (vTauValueH contains 1 additional value)
  int nTauH;
          ///Array of voltages to be searched.  When the correct voltage is found, the corresponding entry in vTauValueH will be used in calculations
  double *vTauVoltageH;
          ///Array of tau values to be used in calculations involving the inactivation particle (h).
  double *vTauValueH;

  bool tauMIsConstant, tauHIsConstant;
  double tauScaleM, tauScaleH;
          ///User defined values used to divide (scale) values during activation particle (m) update equations.
  double *slopeFactorM;
          ///User defined values used to divide (scale) values during inactivation particle (h) update equations.
  double *slopeFactorH;
          ///Channel conductance value
  double unitaryG;
          ///Scaler from 0-1 determines the final influence of this channel on electrical current
  double strength;

// variables that are specific for ahp only
          ///Exponent that calcium concentration will be raised by during calculations
  double Ca_Exp;
          ///Scaler value to multiply the calcium equations
  double Ca_Scale;
          ///User defined value computed in with calcium equations
  double Ca_Half_Min;
          ///Scaler value to multiply the time constant (tau) of calcium
  double Ca_Tau_Scale;

/* These factors can be computed in the channel constructor */

//float pcf1;       /* unitaryG * strength    */
//float sfm0;       /* 1.0 / slopeFactorM [0] */
//float sfm1;       /* 1.0 / slopeFactorM [1] */
//float sfm2;       /* 1.0 / slopeFactorM [2] */
//float sfh0;       /* 1.0 / slopeFactorH [0] */
//float sfh1;       /* 1.0 / slopeFactorH [1] */
//float sfh2;       /* 1.0 / slopeFactorH [2] */

         ///Function pointer to appropriate CalculateCurrent function (set in the constructor)
 double (Channel::*ptrCalcCurrent) (Compartment *);

  //FUNCTIONS

  Channel ();
  Channel (T_CHANNEL *, Random *);   
  ~Channel ();       

  /**
   * Go through vTauVoltageH finding a voltage that is greater than the parameter, then
   * return the corresponding entry in vTauValueH.  If no entry is found, the last value
   * of vTauValuH will be used.
   * @param voltage membrane voltage of this channel's parent compartment
   * @retern the appropriate time constant (tau)
   */
  double GetTauH (double voltage);
  /**
   * Go through vTauVoltageM finding a voltage that is greater than the parameter, then
   * return the corresponding entry in vTauValueM.  If no entry is found, the last value
   * of vTauValueM will be used.
   * @param voltage membrane voltage of this channel's parent compartment
   * @retern the appropriate time constant (tau)
   */
  double GetTauM (double voltage);
  
  /**
   * Interface provided for the parent compartment to call the appropriate
   * current calculation function.
   * added by CWilson July 2000
   */
  double GetCurrent (Compartment *);

  //Functions to calculate current for different channel types

  double CalculateCurrent_Na (Compartment *);
  double CalculateCurrent_Ca (Compartment *);
  double CalculateCurrent_Km (Compartment *);
  double CalculateCurrent_Ka (Compartment *);
  double CalculateCurrent_Kdr (Compartment *);
  double CalculateCurrent_Knicd (Compartment *);
  double CalculateCurrent_Kahp (Compartment *);
  void Print ();
  int GetNumItems ();

  /**
   * Write the channel state to a file.
   * @param out destination file pointer
   * @return the number of bytes written by the function
   */
  int Save( FILE * );
  /**
   * Read the channel state from a file.  This structure has no
   * character string, but "nothing" is included for consistency
   * among load functions.  May be eliminated later.
   * @param in source file pointer
   * @param nothing dummy string for char*(s) to point at
   */
  void Load( FILE *in, char *nothing );
  void ChannelReset( FILE *, char * );
  /**
   * Determine the size of the slopeFactor array (either M or H)
   * @param particle the desired particle, 'm' for activation 'h' for inactivation
   * @return the number of elements contained in that array (from 0-3)
   */
  short int getSlopeFactorSize( char particle );

//private:

  double m_oo, t_m, m;  // activation variables we may want to track
  double h_oo, t_h, h;  // inactivation variables we may want to track
  double funct_m;

          ///channel electrical current
  double I;
};

#endif

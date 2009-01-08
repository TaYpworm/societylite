/* The structures here are a more or less one to one representation of the   */
/* input file.  The parser reads information into them (massaging it         */
/* somewhat).  Once the entire input has been parsed, these structs will be  */
/* used to create the brain objects for the program.  (But that is not a 1-1 */
/* transform: many objects might be created from a single input struct.)     */
/* After this they may be deleted, if not kept around for debugging.         */

#ifndef INITSTRUCT_H
#define INITSTRUCT_H

#include <stdlib.h>
#include <stdio.h>
#include "Locator.h"
#include "parse/list.h"

//include for doc++
//@Include: parse/arrays.h
//@Include: Locator.h
//@Include: parse/list.h

/**
 * Structure to aid in the manipulation of recurrent connections.  When
 * two cell groups have connections made back and forth, a certain percentage
 * of connections are expected to be common.  For example, group A is connected
 * to group B, if cell A1 connects to cell B8, then cell B8 should connect to A1
 * with some probability, and vice-versa.
 */
struct T_RECURRENT
{
          ///General information about this object
  LOCATOR L;
          ///Name of the column of the first cell group
  char *colNameA;
          ///Name of the layer of the first cell group
  char *layNameA;
          ///Name of the cells of the first cell group
  char *cellNameA;
          ///Name of the compartment used by the first cell group
  char *cmpNameA;
          ///Name of the column of the second cell group
  char *colNameB;
          ///Name of the layer of the second cell group
  char *layNameB;
          ///Name of the cells of the second cell group
  char *cellNameB;
          ///Name of the compartment used by the second cell group
  char *cmpNameB;
          ///Name of the synapse that the two connections must both be using to qualify
  char *synName;
          ///Probability that a given connection from group A to B is recurrent
  double PrecurrenceAtoB;
          ///Probability that a given connection from group B to A is recurrent
  double PrecurrenceBtoA;
};
typedef struct T_RECURRENT T_RECURRENT;

/**
 * Information for connections betweens columns, layers, and cells.  The same
 * structure is used to avoid duplication.
 */
struct T_CONNECT
{
          ///General information about this object
  LOCATOR L;

          ///source column name
  char *fromColName;
          ///source layer name
  char *fromLayName;
          ///source cell name
  char *fromCellName;
          ///source compartment name
  char *fromCmpName;
          ///destination column name
  char *toColName;
          ///destination layer name
  char *toLayName;
          ///destination cell name
  char *toCellName;
          ///destination compartment name
  char *toCmpName;
          ///synapse name to use for connection
  char *SynName;

          ///index of source compartment; refers to appropriate T_COLUMN object
  int FromCol;
          ///index of source layer; refers to appropriate T_LAYER object
  int FromLay;
          ///index of source cell; refers to appropriate T_CELL object
  int FromCell;
          ///index of source compartment; refers to appropriate T_COMPARTMENT object
  int FromCmp;
          ///index of destination column; refers to appropriate T_COLUMN object
  int ToCol;
          ///index of destination layer; refers to appropriate T_LAYER object
  int ToLay;
          ///index of destination cell; refers to appropriate T_CELL object
  int ToCell;
          ///index of destination compartment; refers to appropriate T_COMPARTMENT object
  int ToCmp;
          ///index of synapse to use for connection; refers to appropriate T_SYNAPSE object
  int SynType;
          ///speed of connection.  Expected units are millimeters per millisecond (mm/ms)
  double speed;
          ///probability of connection between the source and destination.  Expects values from 0 through 1
  double Prob;
          ///When probability of connection should be modified by distance between the cells, this auxillary value is used in an exponential function to determine the decay of the probability as the distance grows
  double step;
          ///Once all input data is read, a minimum and maximum expected delay can be computed. - eliminate this
  double delay[2];
          ///This connection is responsible for ensuring that recurrent connections are made back to the source cell/cmp
  double recurrentProbability[2];
          ///for recurrent connections: pointer to the other connection, to access information
  struct T_CONNECT *recurrentConnection;
          ///When a recurrent connection is defined, it chooses one T_CONNECT object to use and disables the other
  int disabled;
  
};
typedef struct T_CONNECT T_CONNECT;

/**
 * Structure for connecting compartments within a cell.
 * Does not need values for Column, Layer, Cell like other connects
 */
struct T_CMPCONNECT
{
          ///contains information on object
  LOCATOR L;
          ///string containing compartment name as given in input file
  char *fromCmpName;
          ///string containing compartment name as given in input file
  char *toCmpName;
          ///index of source compartment
  int FromCmp;
          ///index of destination compartment
  int ToCmp;
          ///determines how quickly current propagates between the compartments.
  double Speed;
          ///Conductance value for forward connection. i.e. flow from source to destination
  double G;
          ///Conductance value for reverse connection. i.e. flow from destination back to source
  double retroG;
          ///time of delay in seconds.  Calculated during conversion and filled in.
  double delay;
};
typedef struct T_CMPCONNECT T_CMPCONNECT;

/* These are the actual brain structures.  They begin with a common set of   */
/* header info, which includes a pointer.  The input is first read in as a   */
/* linked list (since the size is not known), then converted to a pointer    */
/* array in AR for ease of access.  The same is done with sub-elements of    */
/* the basic structures (e.g.  connections), where the number isn't known.   */

/**
 * A generic structure
 */
struct T_ANYTHING
{
          ///General information about the structure
  LOCATOR L;
};
typedef struct T_ANYTHING T_ANYTHING;

/**
 * Intermediate Brain Object.
 * After parsing the input, this Brain object will have a list of names for
 * Column, Stimulus injects, and reports.  NCS will look up those names to verify that
 * a corresponding T_COLUMN, T_STINJECT, or T_REPORT exists.  If not, then there is an
 * error; otherwise, the index (ID) of that object is stored in an array within this T_BRAIN
 * object for future reference.
 *
 * As for connections, they are initially stored in a linked list during parsing.  Once the
 * number of connections is known, an array of pointers is created to refer to the elements
 * of the list to allow for random access.
 */
struct T_BRAIN
{
          ///General information about the structure
  LOCATOR L;

          ///string containing a prefix for any files generated by the simulator
  char *job;
          ///distribution algorithm to be used by the simulator for load balancing
  char *distribute;
          ///new variable from ncs5
  int ConnectRpt;
          ///new variable from ncs5
  int SpikeRpt;
          ///The amount of time being simulated specified in seconds
  double Duration;
          ///Frequency Sampling Value.  A second is segmented into FSV timesteps
  int FSV;
          ///Random Seed
  long Seed;
          ///The number of columns declared in the simulation
  int nColumns;
          ///Linked List of the column names lared wihin the Brain block
  LIST *ColumnNames;
          ///array of indexes which refer to the T_COLUMN objects used within the brain
  int *Columns;
          ///The number of stimulus injects declared within the Brain block
  int nStInject;
          ///Linked List of the stimulus inject names &c
  LIST *StInjNames;
          ///array of indexes which refer to the T_STINJECT objects
  int *StInject;
          ///The number of reports declared within the Brain block
  int nReports;
          ///Linked List of report names
  LIST *ReportNames;
          ///array of indexes which refer to the T_REPORT ojects
  int *Reports;
          ///The number of connections declared
  int nConnect;
          ///Linked List of connections created during input
  T_CONNECT *CnList;
          ///array of pointers created for random access
  T_CONNECT **Connect;
          ///filename for saving the Brain state automatically at a specified time
  char *savefile;
          ///Time (in seconds) when the Brain state should be saved automatically
  double savetime;
          ///filename for loading the Brain state at startup
  char *loadfile;
          ///tcp/ip port to use when establishing automatic socket connections
  int HostPort;
          ///tcp/ip port to use for receiving runtime commands
  int Port;
          ///Host location to receive runtime commands from
  char *HostName;
          ///Indicates whether sim should calculate synaptic distances
  //int distanceFlag;
          ///flags
  int flag;
          ///number of recurrent connections
  int nRecurrent;
          ///linked list of recurrent connections created during parsing
  T_RECURRENT *recurrentList;
};
typedef struct T_BRAIN T_BRAIN;

/**
 * Column shell.  This contains additional information for column objects.
 */
struct T_CSHELL
{
          ///General information about this column shell object
  LOCATOR L;
          ///The diameter of the column, measured in micrometers
  double width;
          ///The height of the column, measured in micrometers
  double height;
          ///The x coordinate of the column's center
  double x;
          ///The y coordinate of the column's center
  double y;
};
typedef struct T_CSHELL T_CSHELL;

//typedef struct _Column
/**
 * Container object to organize layers.  After parsing,
 * layers names will be stored in a linked list structure.  These names will be looked up
 * to make sure they exist.  If not, there is an error; otherwise, the index into the layer
 * array will be stored.
 *
 * As for layer connections, they are initially stored in a linked list structure.  Once
 * all connections are known, an array of pointers is made to allow for random access.
 */
struct T_COLUMN
{
          ///general information
  LOCATOR L;
          ///Name of the column shell object to use.  This will give the column dimensions.
  char *shellName;
          ///index refers to the T_CSHELL objects
  int CShell;
          ///Number of layers defined within this column object
  int nLayers;
          ///Linked List of layer names to be included in this column
  LIST *LayerNames;
          ///Array of indexes corresponding to the requested layers
  int *Layers;

          ///number of synapse connections defined between layers in this column
  int nConnect;
          ///Linked List of connections built during parsing
  T_CONNECT *CnList;
          ///Array of connections built after parsing is complete to allow for random access
  T_CONNECT **Connect;
           ///number of recurrent connections
  int nRecurrent;
          ///linked list of recurrent connections created during parsing
  T_RECURRENT *recurrentList;
};
typedef struct T_COLUMN T_COLUMN;

/**
 *  Layer shell object contains spatial information about a layer.
 *  This describes how much area of the column this layer will consume by using percentages.
 *  Because the dimensions of columns can vary, a layer will use percentages rather than
 *  concrete values.
 */
struct T_LSHELL
{
          ///general information
  LOCATOR L;
          ///the bottom of the layer
  double Lower;
          ///the top of the layer
  double Upper;
};
typedef struct T_LSHELL T_LSHELL;

/**
 * layer object contains cells and cell connections.  Cells are read in as a linked list during
 * parsing.  Once all cells are known, The names are looked up.  If a name is not defined, there
 * is an error; otherwise, an array of indexes is made which reference the corresponding cell objects.
 * A separete array containing cell quantities is also created after parsing for faster look up.
 * 
 * As for connections, during parsing, they are placed in a linked list.  After all connections are known,
 * an array of pointers is made to allow for random access.
 */
struct T_LAYER
{
          ///general information
  LOCATOR L;
          ///The name of the layer shell to use
  char *shellName;
          ///Index refers to the appropriate T_LSHELL object
  int LShell;
          ///number of different cell types to place in this layer object
  int nCellTypes;
          ///Linked list of cell names created during parsing
  LIST *CellNames;
          ///Array of indexes corresponding to the requested cell types; built after parsing
  int *CellTypes;
          ///Array of cell quantities; built after parsing
  int *CellQty;
          ///number of connections between cells
  int nConnect;
          ///linked list of connectioins created during parsing
  T_CONNECT *CnList;
          ///Array of pointers to the connections built after parsing to allow for random access
  T_CONNECT **Connect;
          ///number of recurrent connections
  int nRecurrent;
          ///linked list of recurrent connections created during parsing
  T_RECURRENT *recurrentList;
          ///flag to indicate this layer's recurrent connections have been handled (comes into play when a layer is reused in different columns)
  int recurrentHandled;
};
typedef struct T_LAYER T_LAYER;

/**
 *  Intermediate structure for brain cell.
 *  Contains information on compartment instances that make up this cell.
 *  Many fields in this object are filled after the input file has been completely parsed.
 *  Once all the compartments are accounted for, the linked lists are converted to arrays of pointers.
 *  These pointers point to the entries in the linked list to make random access possible whereas using only
 *  the linked list would force traversing the list to move back and forth between items.
 */
struct T_CELL
{
          ///general information
  LOCATOR L;
          ///number of compartments defined in this cell
  int nCmp;
          ///linked list structure holding information on compartments that will make up this cell
  LIST *CmpNames;
          ///array of unique labels assigned to the resulting compartments created from templates
  char **Labels;
          ///array of indexes into the compartment template array
  int *Cmp;
          ///array of x coordinates for each compartment
  double *Xpos;
          ///array of y coordinates for each compartment
  double *Ypos;
          ///array of z coordinates for each compartment
  double *Zpos;       
          ///array of R values - not used but kept to for compatability for now
  double *Rpos;
          ///The number of compartment connections within the cell
  int nConnect;
          ///Linked List of Connection objects
  T_CMPCONNECT *CnList;
          ///Array of Connections.  The array is built after the input file is completely parsed.
  T_CMPCONNECT **Connect;
          ///Each compartment in this cells will have a list of connections it collects info from
  T_CMPCONNECT ***forwardConn;
          ///Each compartment in this cell will have a list of connections for data in reverse
  T_CMPCONNECT ***reverseConn;
          ///array of reverse connection counts for individual compartments
  int *nforward;
          ///array of forward connection counts for individual compartments
  int *nreverse;
};
typedef struct T_CELL T_CELL;

/**
 *  Intermediate compartment object.  There can exist main types of compartments, active (soma) and
 *  non-active (dendrites, axons, spines).  The distinction is that an active compartment has a spikeshape
 *  and threshold that will allow it to fire an action potential upon reaching threshold.  The non-active
 *  compartments merely pass along information.
 */
struct T_CMP
{
          ///general information on this object
  LOCATOR L;
          ///random number generator seed
  long Seed;
          ///Flag to indicate if this compartment is "active" and can fire action potentials
  int Active;
          ///Name of spikeshape to be used when this compartment fires
  char *SpikeName;
          ///Index of appropriate spikeshape in ARRAYS structure
  int Spike;
          ///Number of channels within this compartment
  int nChannels;
          ///Linked list of channel names to be used
  LIST *ChannelNames;
          ///Array of index refer to the appropriate T_CHANNEL objects
  int *Channels;
          ///Spike Half-Width
  double Spike_HW [2];
          ///Time constant used in compartment's membrane voltage calculations
  double Tau_Membrane [2];
          ///Membrane Resistance used in membrane voltage calculations
  double R_Membrane [2];
          ///Active compartments fire action potatials upon reaching the membrane voltage specified in threshold
  double Threshold [2];
          ///Leak value used in calculating the leak of current (I) from the comaprtment
  double Leak_Reversal [2];
          ///When at rest, a compartment's membrane voltage sits at VMREST
  double VMREST [2];
          ///Conductance value used for current (I) leak calculations
  double Leak_G [2];
          ///Initial concentration of Calcium Ions internal to the compartment
  double CaInt [2];
          ///Initial concentration of Calcium Ions external to the compartment
  double CaExt [2];
          ///When a spike occurs, internal calcium concentration increases by this amount
  double CaSpikeInc [2];
          ///Calcium Time Constant, determines how internal calcium persists as time elapses.
  double CaTau [2];             
};
typedef struct T_CMP T_CMP;

/**
 * Intermediate channel structure.
 * Contains fields for all implemented channels.
 * Km
 * Ka
 * Kahp
 * Na
 *
 * Some objects contain a number of slots such as eHalfMinM.  Depending on the channel,
 * some, all, or none of these
 * slots may be filled.  The channel current equation is responsible for accessing the values it needs.
 */
struct T_CHANNEL
{
          ///general information
  LOCATOR L;
          ///seed for random number generator - not used
  long Seed;
          ///The family this channel belongs to: Km, Ka, Kahp, or Na
  int family;
          ///Conductance values. space is given for a mean and a standard deviation
  double unitaryG [2];
          ///The amount of influence by this channel.  Ranges from 0-1, 1 using full channel strength.  Expects mean and std. dev.
  double strength [2];
          ///Initial value assigned to m, the activating particle.  Mean and std. dev.
  double M_Initial [2];
          ///The exponent which the m-particle will be raised.  i.e. m^mPower
  double mPower [2];
          ///Reversal Potential of the Channel.
  double ReversePot [2]; 
          ///Like strength, but expects a lower and upper bound for the values to be uniformly distributed between
  double strength_range [2];
          ///Initial value assigned to h, the inactivating particle.  Mean and std. dev.
  double H_Initial [2];
          ///The exponent which the h-particle will be raised. i.e. h^hPower
  double hPower [2];
          ///Values used when updating channel m-particle. Applied when analyzing the compartment voltage. e.g. V-33.4
  double eHalfMinM [3];
          ///Values used when updating channel h-particle. Applied when analyzing the compartment voltage. e.g. V-33.4
  double eHalfMinH [3];
          ///Number of values stored in tau array
  int nTauM;
          ///Number of values stored in TauValM.  nValM = nVoltM + 1
  int nValM;
          ///Number of values stored in TauVoltM  nVoltM = nValM - 1
  int nVoltM;
          ///List of tau values for m; the one used depends on cmp voltage and TauVoltM.  Last Value is extra to return in case no Voltages in TauVoltM suffice.
  double *TauValM;
          ///List of voltages to compare with compartment voltage.  Each entry corresponds with an entry in TauValM.
  double *TauVoltM;
          ///a std. dev. to apply to all numbers in TauValM array.  If this is zero, one copy of TauValM will be shared to save memory.
  double ValM_stdev;
          ///a std. dev. to apply to all numbers in TauVoltM array.  If this is zero, one copy of TauVoltM will be shared to save memory.
  double VoltM_stdev;
          ///Number of values stored in tau array
  int nTauH;
          ///Number of values stored in TauValH. nValH = nVoltH + 1
  int nValH;
          ///Number of values stored in TauVoltH. nVoltH = nValM - 1
  int nVoltH;
          ///List of tau values for h; the one used depends on cmp voltage and TauVoltH.  Last value is extra to return in case no Voltages in TauVoltH suffice.
  double *TauValH;
          ///List of voltages to compare with compartment voltage.  Each entry corresponds with an entry in TauValH.
  double *TauVoltH;
          ///a std. dev. to apply to all numbers in TauValH array.  If this is zero, one copy of TauValH will be shared to save memory.
  double ValH_stdev;
          ///a std. dev. to apply to all numbers in TauVoltH array.  If this is zero, one copy of TauVoltH will be shared to save memory.
  double VoltH_stdev;

  /* new for Fast Sodium Channels - Na */
          ///Scaler to multiply the alpha value of the tau of the m particle.  Uses mean and std. dev.
  double alphaScaleFactorM [2];
          ///Scaler to multiply the beta value of the tau of the m particle.  Uses mean and std. dev.
  double betaScaleFactorM [2];
          ///Scaler to multiply the alpha value of the tau of the h particle.  Uses mean and std. dev.
  double alphaScaleFactorH [2];
          ///Scaler to multiply the beta value of the tau of the h particle.  Uses mean and std. dev.
  double betaScaleFactorH [2];

          ///Scaler to multiply the tau of the m particle.  Uses mean and std. dev.
  double tauScaleM [2];
          ///Scaler to multiply the tau of the h particle.  Uses mean and std. dev.
  double tauScaleH [2];
          ///Three spaces available for values to divide calculations in the m particle. e.g. (V-33)/6
  double slopeM [3];
          ///A standard deviation to apply to all slopeM values.
  double slopeM_stdev;
          ///Three spaces available for values to divide calculations in the h particle. e.g. (V-33)/6
  double slopeH [3];
          ///A std. dev. to apply to all slopeH values
  double slopeH_stdev;
 
  /* ahp channel specific parameters */
          ///Exponent to apply to the calcium calculation.  Uses mean and std. dev. to create variance.
  double CA_EXP [2];
          ///Exponent to apply to the calcium ccalulation.  Uses lower and upper values to create variance.
  double CA_EXP_RANGE[2];
          ///Scaler values to multiply the calcium equations.  Mean and std. dev.
  double CA_SCALE [2];
          ///Two spaces available for values that are used in calculations.  e.g. (V-34)
  double CA_HALF_MIN [2];
          ///Scaler to multiply the tau of the calcium.  Mean and std. dev.
  double CA_TAU_SCALE [2];
};
typedef struct T_CHANNEL T_CHANNEL;

/**
 *  Intermediate Synapse.  When the input file is read in, the types of synapse
 *  sub-objects to use are read in.  During conversion, the index number of the
 *  corresponding object (T_SYNLEARN, T_SYNDATA, T_SYN_FD, T_SYNPSG) will be
 *  saved.
 *  The T_SYNDATA object no longer needs to be included since its data members have
 *  been added to this T_SYNAPSE object directly
 */
struct T_SYNAPSE
{
          ///general information about this object
  LOCATOR L;
          ///Random number generator seed
  long Seed;
          ///Type of Synapse Facil/Depress (SFD) to use in this synapse
  char *SfdName;
          ///Type of Synapse Learning to use in this synapse
  char *LearnName;
          ///Type of Synapse Datea to use in this synapse (no longer required)
  char *DataName;
          ///Type of Synaptic Waveform (PSG) to use in this synapse
  char *PsgName;
          ///Type of Synaptic Augmentation to use in this synapse (optional)
  char *AugmentationName;
          ///index of appropriate T_SYN_FD object (Previously called RSEType)
  int SfdIdx;
          ///index of appropriate T_SYN_LEARN object
  int LearnIdx;
          ///index of appropriate T_SYNDATA object
  int DataIdx;
          ///index of appropriate T_SYNPSG object
  int PsgIdx;
          ///index of appropriate T_SYNAUGMENTATION object
  int AugmentationIdx;

          ///Initial USE value
  double USE [2];
          ///The maximum synaptic conductance, formerly handled by T_SYNDATA
  double MaxG[2];
          ///The time delay for synaptic activity, formerly handled by T_SYNDATA
  double SynDelay [2];
          ///Synaptic Reversal, formerly handled by T_SYNDATA
  double SynRever [2];
          ///Projected time since the last spike
  double InitDeltaT[2];
          ///Initial value of the synapse RSE value
  double InitRSE[2];
};
typedef struct T_SYNAPSE T_SYNAPSE;

/**
 *  Intermediate Synapse Learning object.  For long term learning
 */
struct T_SYNLEARN
{
          ///general information on object
  LOCATOR L;

          ///The type of learning handled by this object - NONE, +HEBBIAN, -HEBBIAN, BOTH
  int Learning;
          ///Random Number generator seed
  long Seed;
          ///The maximum change in USE allowed for negative learning
  double Neg_Heb_Peak_Delta_Use [2];
          ///The maximum change in USE allowed for positive learning
  double Pos_Heb_Peak_Delta_Use [2];
  double Neg_Heb_Peak_Time [2];
  double Pos_Heb_Peak_Time [2];

          ///Window of time for negative Hebbian learning
  double DownWind [2];
          ///Window of time for positive Hebbian learning
  double UpWind [2];

          ///new from ncs5
  int nPosLearn;
          ///new from ncs5
  double *PosLearnTable;

          ///new from ncs5 
  int nNegLearn;
          ///new from ncs5
  double *NegLearnTable;
};
typedef struct T_SYNLEARN T_SYNLEARN;

/**
 *  Intermediate Synapse Facil/Depr object. Previously called T_SYNRSE.
 */
struct T_SYN_FD
{
          ///Genral Information about this object
  LOCATOR L;
          ///Random Number Generator Seed
  long Seed;
          ///Integer code representing the type of FD - NONE, DEPR, FACIL, BOTH
  int SFD;

          ///Time constant for facilitation
  double Facil_Tau [2];
          ///Time constant for depression
  double Depr_Tau [2];
};
typedef struct T_SYN_FD T_SYN_FD;

/**
 *  Post Synaptic Waveform
 */
struct T_SYNPSG
{
          ///general information about this object
  LOCATOR L;
          ///Name of the file containing waveform data
  char *File;
          ///number of values in the waveform
  int nPSG;
          ///array of values that make up the waveform
  double *PSG;
};
typedef struct T_SYNPSG T_SYNPSG; 

/**
 *  Deprecated synapse data object.  This information can now be put directly into a synapse object.
 */
struct T_SYNDATA
{
          ///general information about this object
  LOCATOR L;
          ///random number generator seed
  long Seed;
          ///Maximum Synaptic conductance
  double MaxG[2];
          ///Time delay from sending cell to receiving cell
  double SynDelay [2];
          ///Synaptic Reversal
  double SynRever [2];
};
typedef struct T_SYNDATA T_SYNDATA;

/**
 * Intermediate aynaptic augmentation object.  This allows synapses to have an optional
 * dependance on calcium concentrations to affect facilitation
 */
struct T_SYNAUGMENTATION
{
          ///general information about this object
  LOCATOR L;
          ///Initial Augmentation value
  double Augmentation_init[2];
          ///Initial Calium concentration
  double CA_init[2];
          ///Calcium decay time constant
  double CA_tau[2];
          ///Maximum augmentation
  double MaxSA[2];
          ///Decay property to shorten CA tau when CA concentrations are high
  double CA_decay[2];
          ///Calcium increment per incoming spike
  double CA_increment[2];
          ///alpha value impacts calcium increment - necessary?
  double Alpha[2];
          ///Augmentation decay time constant
  double Augmentation_tau[2];
          ///Time delay before Calcium can affect augmentation (typ .5 sec)
  double SA_delay[2];
};
typedef struct T_SYNAUGMENTATION T_SYNAUGMENTATION;

/**
 * Spike Shape for an active compartment firing upon reaching threshold
 */
struct T_SPIKE
{
          ///general information about this object
  LOCATOR L;
          ///number of voltages specified in input file
  int nVoltages;
          ///Array of voltages that make up the spike shape
  double *Voltages;
};
typedef struct T_SPIKE T_SPIKE;

/**
 *
 */
struct T_STIMULUS
{
  LOCATOR L;

  int MODE;
  int PATTERN;
  int TIMING;
  int SAME_SEED;
  long Seed;
  double AMP_Start;
  double AMP_End;
  int nTimes;
  int nTStart;
  int nTStop;
  double *TStart;
  double *TStop;
  int *Time_Start;
  int *Time_Stop;
  double FREQ_Start;
  double FREQ_End;
  double WidthSec;          // used for PULSE
  double PHASE;             // for sine
  double VERT_TRANS;        // for sine
  char *FileName;           // for FILE_BASED stimulus
  int Port;
  int nFreqs;
  int CellsPerFreq; 
  int FreqCount;
  int TimeIntervals;
  int nSpikeProb;          
  float *SpikeProb;        /* For FILEBASED, the data from the file */
  double Time_Freq_Incr [2];
  double DynRange [2];
  double DataMax;
  double DataMin;
};
typedef struct T_STIMULUS T_STIMULUS;

/**
 *  Intermediate Stimulus Inject object.  Allows a single stimulus to be used
 *  multiple times.  Names of requested objects are stored until conversion
 *  when those names will be looked up and an index number into the ARRAYS
 *  structure will be saved for more convenient usage.
 */
struct T_STINJECT
{
          ///general information about this object
  LOCATOR L;

          ///Name of stimulus this injection will use
  char *StimName;
          ///Name of Column this injection targets
  char *ColName;
          ///Name of Layer this injection targets
  char *LayName;
          ///Name of Cell this injection targets
  char *CellName;
          ///Name of Compartment this injection targets
  char *CmpName;

          ///Index of corresponding T_STIMULUS object
  int STIM_TYPE;
          ///Index of corresponding T_COLUMN object
  int Column;
          ///Index of correspoding T_LAYER object
  int Layer;
          ///Index of corresponding T_CELL object
  int Cell;
          ///Index of corresponding T_CMP object
  int Cmp;
          ///Probability that a particular cell will be included in the stimulus injection
  double Prob;
};
typedef struct T_STINJECT T_STINJECT;

/**
 *  Intermediate Report object.  As the input file is parsed, the name of requested
 *  objects are stroed.  During conversion, those names are looked up and an index
 *  value into the ARRAYS structure is saved for more convenient usage
 */
struct T_REPORT
{
          ///general information about this object
  LOCATOR L;
          ///Random number generator seed
  long Seed;
          ///Name of Column type
  char *ColName;
          ///Name of Layer type
  char *LayName;
          ///Name of Cell type
  char *CellName;
          ///Name of Compartment type
  char *CmpName;
          ///Name of alternate object (may be Channel, Synapse, or Synapse_RSE types)
  char *Name;

          ///Index of corresponding T_COLUMN object
  int Column;
          ///Index of corresponding T_LAYER object
  int Layer;
          ///Index of corresponding T_CELL object
  int Cell;
          ///Index of corresponding T_CMP object
  int Cmp;
          ///Index of corresponding alternate object (T_CHANNEL, T_SYNAPSE, etc. )
  int Thing;
          ///Name of file report data should be written to (or hostname for real-time output)
  char *FileName;
          ///Type of report data to be extracted from source and written
  int ReportOn;
          ///Flag to indicate if data should be written in ASCII (true) or Binary (false) form
  int ASCII;
          ///For real-time reporting, data should be sent to this port on the target host
  int Port;
          ///Probability that a particular cell will be included in the report
  double Prob;
          ///How often a report should be made
  unsigned int Frequency;
          ///The total number of start/stop time pairs
  int nTimes;
          ///The number of report start up times
  int nTStart;
          ///The number of report stop times
  int nTStop;
          ///Array of report start times in seconds
  double *TStart;
          ///Array of report stop times in seconds
  double *TStop;
          ///Array of report start times in timeticks
  int *Time_Start;
          ///Array of report stop times in timeticks
  int *Time_Stop;
          ///Flags indicating which report version to use (only affects binary reports)
  int reportFlag;
};
typedef struct T_REPORT T_REPORT;

#endif 


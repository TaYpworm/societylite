/* Defines a structure that holds pointers to all the arrays created from    */
/* the input (as defined in InitStruct.h, plus various related info.         */

#ifndef ARRRAY_DEF
#define ARRRAY_DEF

#include <stdlib.h>

#include "../InitStruct.h"

/**
 *  Structure to hold pointers to data created from input file.
 *  Data is the result of conversion done by InitStruct.h
 *  This final structure is what will be returned to the NCS main function
 *
 *  When data is ready to be put into this ARRAYS object, it will be in linked list form
 *  During conversion, arrays will be made and pointers to the original object will be 
 *  put into those arrays.  This will allow for the objects to be accessed using
 *  subscript notation
 */
struct ARRAYS
{
          ///Total number of layers in the running brain
  int LayerCount;
          ///The total number of cells that will be created in the running brain
  int CellCount;
          ///The number of cell clusters
  int ClusterCount;
          ///Number of compartments that will be created when the brain is instantiated
  int CmpCount;
          ///Number of channels that will be created when the brain is instantiated
  int ChanCount;

          ///Brain structure. There can be only one Brain object defined in an input file.
  T_BRAIN *Brain;
          ///The number of column shells.
  int nCsh;
          ///Array of pointers to Column Shell objects
  T_CSHELL **Csh;
          ///The number of columns.
  int nColumns;
          ///Array of pointers to column objects
  T_COLUMN **Columns;
          ///The number of layer shells.
  int nLsh;
          ///Array of pointers to Layer Shell objects
  T_LSHELL **Lsh;
          ///The number of layers.
  int nLayers;
          ///Array of pointers to Layer objects
  T_LAYER **Layers;
          ///The number of Cells
  int nCells;
          ///Array of pointers to Cell objects
  T_CELL **Cells;
          ///Number of compartments
  int nCmp;
          ///Array of pointers to Compartment objects
  T_CMP **Cmp;
          ///Number of channels
  int nChannel;
          ///Array of pointers to Channel objects
  T_CHANNEL **Channel;
          ///Number of Synapses
  int nSynapse;
          ///Array of pointers to synapse objects
  T_SYNAPSE **Synapse;
          ///Number of Synaptic learning objects
  int nSynLearn;
          ///Array of pointers to synaptic learning objects
  T_SYNLEARN **SynLearn;
          ///Number of synaptic facil/depr objects
  int nSynFD;            /* Was RSE */
          ///Array of pointers to synaptic facil/depr objects
  T_SYN_FD **SynFD;
          ///Number of synaptic waveforms
  int nSynPSG; 
          ///Array of pointers to synaptic waveform objects
  T_SYNPSG **SynPSG;
          ///number of synapse data objects
  int nSynData;
          ///Array of pointers to synaptic data objects
  T_SYNDATA **SynData;
          ///number of synapse augmentation objects
  int nSynAugmentation;
          ///Array of pointers to synaptic augmentation objects
  T_SYNAUGMENTATION **SynAugmentation;
          ///number of spike shapes
  int nSpike;
          ///Array of pointers to spike shape objects
  T_SPIKE **Spike;
          ///Number of stimulus
  int nStimulus;
          ///Array of pointers to stimulus objects
  T_STIMULUS **Stimulus;
          ///number of stimulus injects
  int nStInject;
          ///Array of pointers to stim inject objects
  T_STINJECT **StInject;
          ///number of reports
  int nReports;
          ///Array of pointers to report objects
  T_REPORT **Reports;

};
typedef struct ARRAYS ARRAYS;

#endif 

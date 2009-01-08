#ifndef REPORT_H
#define REPORT_H

#include <stdlib.h>
#include <stdio.h>

#include "Brain.h"
#include "Random.h"
#include "Output.h"
#include "defines.h"

using namespace std;

//@Include: Output.h

/**
 * This module handles the gathering of data for a particular report into a
 * buffer.  The buffer is then passed to functions in Output.cpp, which take
 * care of collecting the data from all the nodes, and outputting it to the
 * proper destination.  See explanation at end of .cpp file.
 */
class Report
{
 public:
           ///Index into ARRAYS struct where this report is defined
  int idx;
           ///Linked list pointer to next Report object
  Report *next;
           ///Linked list pointer to previous Report object
  Report *prev;
           ///Pointer to Output Object used to handle the actual writing of data
  Output *Out;
           ///Integer Code indicating what this report reports
  int ReportOn;
           ///A sub-code for Channel Reports
  int family;
           ///True if the report file has been opened
  bool Opened;
           ///True if not start first interval of a report period
  bool Started;
           ///Index of the active Report time interval (i.e. location within TStart/TStop arrays)
  int iTime;
           ///Number of time intervals
  int nTimes;
           ///Array of report start times
  int *TStart;
           ///Array of report stop times
  int *TStop;
           ///A report is made every Freq timesteps
  int Freq;
           ///Helper variable to keep track of how many timesteps are left until next report
  int FreqCounter;
           ///Number of things to report on that are on this node
  int nThings;
           ///Size of things per ID in header
  int ThingSize;
           ///List of pointers to things to report on
  void **ThingList;
           ///buffer to which data is copied for output
  void *RptBuf;
           ///Pointer to appropriate reporting function
  void (Report::*Func) ();
           ///True if the user wants the ability to dynamically add new start/stop times to this report
  bool Dynamic;
           /**
            * For synapse Augmentation reports so that if Augmentation has not been updated
            * This timestep, it will update and produce an accurate look at the growth and  decay of Augmentation
            */
  int currentTime;
           /**
            * Array to store the number of synapses each cell owns.
            * When performing synapse reports, users may wish to average each cell's synapses into
            * one value per cell.  This array will keep track of how many elements from the
            * ThingList need to be summed and averaged per cell.
            */
  int *synapseCounts;
           /**
            * Options activated by the user. A copy of this is stored in the Output object.
            * Current options include: e-notation for ascii reports (1), averaging synapses (2)
            */
  int reportFlag;
           /**
            * The maximum value in a compartment's spikeshape.  For use in
            * Fire Count reports, this is the peak of the spike which
            * indeicates a compartment has fired.
            */
  double spikeMax;

/*--  functions */

  Report (T_REPORT *, char *, Random *);
  ~Report ();                               

           /**
            * This function allocates the buffer where reported data is
            * stored until it is ready to be output into a file.  Special
            * cases are handled here where the number of elements in
            * the buffer are not equal to the number of reported objects.
            * For example, channel reports have more values than the cell
            * count, and fire reports reduce down to one value.
            */
  void AllocateBuffer(); 

  void GetThingsToReport (T_REPORT *, Random *);
  void GetCmpPtrs (T_REPORT *, Random *);
  void GetChanPtrs (T_REPORT *, Random *);
  void GetSynPtrs (T_REPORT *, Random *);

  bool DoReport (int);

  void GetDblPtrVal ();
  void GetFltPtrVal ();
  void GetSynapseAugmentation();
  void GetChanValues ();

           /**
            * Go through list of reporting cells, counting the number that
            * have reached the peak of the spikeshape.  Return only
            * that count.
            */
  void GetFireCount();
  void EmptyFunc ();

  int getFamilyCount();
};

#endif 

#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#ifdef USE_AIO
  #include <aio.h>
  #include <errno.h>
#endif 

#include "Brain.h"
#include "Report.h"
#include "defines.h"
#include "parse/arrays.h"

/**
 * This handles actually writing the report info to output.  Since all of a  
 * cluster is now on a single node, it no longer needs to do collection via  
 * MPI.  It could be replaced with file I/O in Report, but keeping it
 * separate makes it easier to handle e.g.  AIO output.
 *
 * The files are normally written to the user's file space on the head node.
 * However, if the output file has a name beginning with /tmp, it will be
 * written to the node's local /tmp dir.  Note that some mechanism should be
 * added to output node:filename info so they can be located...  JF
 */
class Output
{
 public:
          ///File pointer to send report data to
  FILE *File;
          ///If > 0, send output to designated port, if == -1, contact server to get an auto port
  int Port;
          ///True if report is to be written in ASCII format
  bool ASCII;
          ///True if report is to be written to local node's /tmp dir
  bool IsTmp;
          /**
           * FileName has multiple uses. For file reports, the file name to combine with the
           * job descriptor. Statis Port assignments (Port > 0) use FileName as the server address.
           * Auto Port assignments (Port==-1) use FileName to label the report for proper connections.
           */
  char *FileName;
          ///Number of items in report's data.
  int nThings;
          ///The size of each element in the report data.
  int ThingSize;
          /**
           * Determine which version of report to write. 
           * Binary reports will have additional header information output if
           * versionNum > 1.  This is too maintain compatibility with older utilities
           * that use the original version of binary reports.
           * ASCII reports will use the falg to determine if output should use e notation
           */
  int reportFlag;
          /**
           * For some reports (i.e. Channel reports) multiple values are written to the
           * buffer and must be all output.  By default, this should be  1.
           */
  int nItems;

#ifdef USE_AIO
  struct aiocb aio;
//bool PendingAIO;      /* true if an AOI operation is in progress */
#endif 

  Output (T_REPORT *iRep, int NTHINGS, int THINGSIZE, char *ASCIIhdr);
  ~Output ();

  void OpenFile (char *);
  void CloseFile ();

  void StartOutput (int, void *);
  void FinishOutput (void *);

  void setItems( int nitems );

  /**
   * If the Port is set to -1, then NCS should try to contact the server program
   * which will handle the connection process for NCS.  The server will be given a report
   * name that is constructed from the Brain's type, the Job name, and the report's type
   * 
   */
  FILE* getAutoPort();

};

#endif 

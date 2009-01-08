//Jim King
//reportinfo.h
//Information about a binary NCS report

#ifndef SERVER_REPORT_INFO
#define SERVER_REPORT_INFO

#include <stdio.h>
#include <stdlib.h>

/**
 * When the server is providing binary reports taken from
 * the file system, the information gained must be stored
 * so that it is accessible later.
 */
class ReportInfo
{
  private:
      //pool of already allocated objects
    //static list <ReportInfo*> reportPool;
      //Id pool
    static int repCount;
    static int maxRep;
      //id
    int repID;

      //keep track of files open
    static int filesOpen;


      ///flag to indicate this report is available
    bool available;
      ///report file name
    char *fileName;
      ///NCS brain type
    char brainType[127];
      ///NCS report type
    char reportType[32];
      ///Report On
    char reportOn[32];
      ///Target Column
    char column[32];
      ///Target Layer
    char layer[32];
      ///Target Cell
    char cell[32];
      ///Target Cmp - contains both cmp template and label names
    char cmp[32];
      ///Number of elements
    int nElements;
      ///Size of the elements
    int sizeElements;
      ///File pointer to file for reading
    FILE *fin;
      ///First byte past the binary header
    int startingByte;
      ///Buffer to contain the last line read from the file
    char *lastLine;
      ///Character pointers to quickly access the desriptors
    char *descriptor[6];
      ///Periodically check to see how many timesteps are available
    int maxAvailable;
      ///Current timestep I am at (i.e. about to ead if readLine were called)
    int currentTimeStep;

  public:
    /**
     * Default Constructor. Opens specified file and calls
     * readHeader function to determine validity of file.
     * If file is valid, perform initializations.
     */
    ReportInfo( char *file );

    /**
     * destructor
     */
    ~ReportInfo();

    static void peek();

    /**
     * Attempt to read the file as if it was an NCS binary file.
     * If it is binary, collect its data and remember the first
     * byte where actual data begins.
     * @return 0 on success, -1 on failure (not NCS binary)
     */
    int readHeader();

    /**
     * Indicates that the file is an NCS binary Report
     * @return 1 if file is an NCS binary report, 0 otherwise
     */
    int detectNCSbinary();

    /**
     * Read the next line of text from the file. This assumes
     * that there is data to read.
     */
    int readLine();

    /**
     * Move file pointer to the beginning of the specified timestep.
     * @param timestep Timestep to advance (rewind) to.
     */
    int moveToTimestep( int timestep );

    /**
     * Send select portions of the header to a client application.
     * Data includes: Report Name, Col, Lay, Cell, Cmp, Report On
     * @param fout File stream (over a socket) to the client
     */
    int sendHeader( FILE *fout );

    /**
     * Given criteria - is this report to be included
     * This should use regular expressions as well.  Is there already
     * A regular expression library for c++?
     * @param criteria Fixed length string (192 bytes) segmented into 6 sections
     * @return 1 if it meets the criteria otherwise 0.
     */
    int checkCriteria( char *criteria );

    /**
     * Similar to checkCriteria function, except that any individual
     * field can be of variable length.  To separte - tokenize on the
     * space (' '), tab ('\t'), and null ('\0');
     */
    int variedLengthCheckCriteria( char *criteria );

    /**
     * Return nunmber of elements this report handles.
     */
    int getCount();

    /**
     * Write the currently held line to the specified file stream.
     * @param fout Destination data stream (file or socket based)
     */
    int sendLine( FILE *fout );

    /**
     * Determine if this report info object is alphabetically
     * less than another report info object. This will check
     * all six descriptors (Report Name=0, Report On=1, Column=2, Layer=3,
     * Cell=4, Cmp=5), but the order may vary as the user desires. The order
     * the desriptors are used in the comparison depends on the int array
     * sent. The integers dictate which corresponding descriptor to select
     * first, second, etc. Ex. 3, 5, 4, 2, 1, 0 is sent. The primary sort
     * criteria is based on Layer, the second criteria is cmp, then cell, ...
     * @param RHS The other report info object we are comparing with
     * @param order Integer array dictating the priority of each descriptor in the sorting
     */
    int lessThan( const ReportInfo *RHS, const int order[] ) const;

    /**
     * Determine if this report info object is alphabetically
     * greater than another report info object. This will check
     * all six descriptors (Report Name=0, Report On=1, Column=2, Layer=3,
     * Cell=4, Cmp=5), but the order may vary as the user desires. The order
     * the desriptors are used in the comparison depends on the int array
     * sent. The integers dictate which corresponding descriptor to select
     * first, second, etc. Ex. 3, 5, 4, 2, 1, 0 is sent. The primary sort
     * criteria is based on Layer, the second criteria is cmp, then cell, ...
     * @param RHS The other report info object we are comparing with
     * @param order Integer array dictating the priority of each descriptor in the sorting
     */
    int greaterThan( const ReportInfo *RHS, const int order[] ) const;

    /**
     * See how many time steps exist in total by seeking to the end of the file
     * and calulating how many full data lines exist from the end of the header
     * to the end of the file.  If this is not a whole number (due to a partial
     * line at the end), then truncate the value.
     * 
     */
    int checkAvailable();

    /**
     * Check to make sure that another full line of data is ready for reading.
     * Should I have a function the adds onto checkAvailable? what would be the
     * advatage? faster?
     */

    /**
     *
     */
    int openFile();

    /**
     *
     */
    int closeFile();

    /**
     */
    //ReportInfo *operator new();


};

#endif


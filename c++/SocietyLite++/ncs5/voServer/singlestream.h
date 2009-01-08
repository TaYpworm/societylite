//singlestream.h

//use only one socket to obtain multiple reports (or send multiple stimulus)
//I guess this introduces the question, will I send commands using the same
//socket that I receive reports on?
//I suppose I can, since I can receive the command to forward x reports
//to the client. If those time steps aren't all available, I could push it
//onto a waiting list, then see if the client has more stuff for me to do
//i.e. stimulus to give to NCS

#include <dirent.h>

#include "reportinfo.h"
#include <vector>
#include "serverconsts.h"
#include "storage.h"

using namespace std;

/**
 * Connection method that requires a client to make only one socket connection to the server.
 * This version is useful for when there would be over 1024 open sockets using autoport.
 * The client connects and passes a name with each action (read/write).  For example,
 * a client connects and wants to read data named 'reward', it passes 'read' and 'reward' to
 * the server and the server transmits the data.  The same client then wants to write data
 * named 'motor-response', it passes 'write' and 'motor-response' and the server will
 * accept the data the client then transmits.  Each client has its own instance of
 * SingleStream, but they share the same data space through a static vector of objects.
 */
class SingleStream
{
  private:
    //this is the current path/job that this client wants 
    //to deal with
    char path[1024];
    char job[1024];
    
        ///Shared vector so that all connections can read/write from any stimulus
    static vector<Storage *> writtenData;

        ///Active sort order for report properties
    int sortOrder[6];

    //If this object is in the middle of sending data
    int sending;
    int recall[2];

    int verboseFlag;

    //comm streams
    int sd;
    FILE *fin;
        ///Pointer to the applications pool of socket descriptors
    static fd_set *allset;
        ///Pointer to the applications largest socket value
    static int *maxDescriptor;
        ///Pointer to applications list of NCS executables
    static vector<char*> *appList;
        ///Pointer to script list for NCS support
    static vector<char*> *scriptList;

    //
    vector <ReportInfo*> validReports;
        ///When sending needs to be paused, this will remember
    vector<ReportInfo*> subset;

    vector<vector <char *> >userPatterns;
        ///index into userPatterns for the currently active pattern
    int activePattern;

  public:
    /**
     * Configure this class to use a descriptor set created by
     * the application.  This class can then administer its own
     * additions and removals.
     * @pool The collection of open sockets so that sockets may be added/deleted
     * @descriptors The highest value socket opened.
     */
    static int init( fd_set *pool, int* descriptors, vector<char*> *appList, vector<char*> *scriptList=NULL );

    SingleStream();

    /**
     * Create a single stream from a given socket.
     */
    SingleStream( int socket, int verbose=0 );

    //should the patterns be stored? or just the subset of reports that
    //was built from the pattern(s)
    // if we save pattern(s), we can see if new reports can be added
      //then the question becomes, when do we update?
    // if we save subset(s), we don't have to constantly rebuild
    //I also need to wonder what happens if two reports meet the
    //same parameters, but report on different times.  How is the one
    //report to know that it does not have valid times? They each think
    //they start at zero.  THerefore, it becomes more likely that some
    //way of remembering the time is needed.  How is this best implemented?
    //Include info in the header? Prepend each timestep?  If I include a 
    //header, I can calculate offsets beforehand, but that seems to use time.
    //If I prepend a timestep to each line, then I have to read that number.
    //this seems like a poor soultion.
    //I will add to NCS version2 report that includes time start/end pairs
    int getPatterns();

    /**
     * There is activity on this client's socket descriptor (sd).  Try
     * to read a command or determine if the client has disconnected.
     */
    int process();

    /**
     *
     */
    int setPath();

    /**
     * The user wants to add another set of patterns.  After recieving
     * the pattern(s), send to the user a reference number so that
     * it can be accessed easily
     */
    int setPattern();

    /**
     * User selects the pattern to use by sending one integer.  This
     * function is probably not necessary, since that integer can be sent 
     * within the other commands
     */
    int selectPattern();

    /**
     * Equality operator so that stl objects can function better
     */
    int operator==( const SingleStream &RHS ) const;

    /**
     * Remove this object from master list of desriptors and close
     * the socket.
     */
    int clear();

    /**
     * Send the data - if I cannot finish writing (waiting on data)
     * I should stop and return later.  Therefore, I need to add
     * a way to remember where I left off. (a flag?)
     */
    int transmit( vector<ReportInfo *> &subset, int reportTimes[] );

    /**
     * Rearrange reports according to the user's demand.  This way,
     * lengthy headers with names are not needed.
     */
    int sortReports( vector<ReportInfo*> &subset );

    /**
     * Using the path and job member variables, find valid NCS
     * binary reports and store them in the primary vector.
     */
    int gatherReports();
 
    /**
     * The user selects a pattern and any NCS binary reports matching
     * the pattern(s) are packaged and sent.
     */
    int getData();

    /**
     * A client wants to launch NCS remotely.  Various options are
     * set in the additional parameters sent with the launchNCS command:
     * Determine the type of NCS run (direct or queue based), ethernet or 
     * myrinet, number of nodes, input and output files. 
     */
    int launchNCS();

    /**
     * Used to receive the contents of an input file or node machine list
     * from the client launching a remote NCS simulation.
     * @param filename Target file to store contents read from client socket
     * @return success or failure
     */
    int receiveFile( char *fileName );

    /**
     * Directly call mpirun to launch NCS without using the PBS queue.
     * This allows the user to specifically choose machine nodes to
     * execute NCS.
     * @param userFlags Flags set by the user
     * @param fileNames Files for input and output
     * @return success or failure
     */
    int directLaunch( int *userFlags, char fileNames[][MaxLength] );

    /**
     * Create a script file to give to PBS (portable batch system)
     * with the parameters governing the simulation.  The queuing
     * software will select lightly loaded nodes to run the
     * simulation)
     * @param userFlags Flags set by the user
     * @param fileNames Files for input and output
     * @return success or failure
     */
    int generateScript( int *userFlags, char fileNames[][MaxLength] );

    /**
     * This object is waiting for more data so that it can
     * send it.
     */
    int isSending();

    /**
     * Look at subset of reports and determine minimum number of timesteps
     * available.
     */
    int getTimeCount();

    /**
     * User will specify which fields have higher priority in sorting reports.
     * The server will read six bytes off the stream.  The bytes will
     * be 0-5 in some arrangement.  The numbers correspond to a report field.
     * Report Name = 0, Report On = 1, Column Name = 2, Layer Name = 3, 
     * Cell Name = 4, Compartment Name = 5.  The numbers will be arranged
     * based on which fields to sort on first, second, third, etc.  For example,
     * 2 5 0 3 1 4 means that Columns have priority on sorting, followed by
     * compartment, then report name, layer name, report on, and finally cell
     * name.
     * @return 1 on success, less than 0 on failure
     */
    int setSorting();

    /**
     * Look at valid NCS binary reports and select a subset of them
     * based on pattern matching.
     */
    int buildSubset( int patternID );

    /**
     * Determine how many reports exist in a subset if a given pattern
     * ID is used.  The pattern ID is read off the stream.
     */
    int getReportCount();

    /**
     */
    int emptyReports();

    /**
     * invoke a script -> how many arguments to expect?
     */
    int invoke();

    /**
     * create a directory for a run
     */
    int makeDirectory();

    /**
     * list directory contents for navigation
     */
    int listFiles();

    /**
     * Invoke a server approved script.  This script must appear in the
     * file server.scripts, parameters for this script will be read during
     * the function, and passed to the script.
     */
    int invokeScript();
    
    /**
     * Accept data from a source for an NCS stimulus
     */
    int writeData();
    
    /**
     * Retrieve data for an NCS stimulus to use
     */
    int readData();
    
    /**
     * Retrieve the key for a Storage object, or create one if it doesn't exist.
     * The key will be the index of the object into the Storage vector
     */
    int getKey();

    /**
     * When the client wants to read or write data, this function will
     * get the necessary information to access the data storage area,
     * find or create an entry, and perform the read/write operation
     */
    int manageData();
};

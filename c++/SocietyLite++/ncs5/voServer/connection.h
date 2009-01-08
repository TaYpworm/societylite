//Jim King
//connection.h

#include <cstdlib>
#include <cstring>
#include <list>
#include <queue>
#include <stack>
#include <sys/select.h>

using namespace std;

/**
 * Allows a writer or Reader to specify individual parameters such as verbose, format
 */
class Client
{
  public:
        ///The socket desriptor this client read from/writes to
    int socket;
        ///Not used - thinking whether individual clients should have verbose on/off
    int verbose;
        ///Client data sent/received in ascii(0);  binary(1); or constant binary (2)
    int format;
        ///Binary Reader clients need to be informed about nElements and elementSize
    int needsInfo;

    /**
     * Overloaded equality operator allows the use of list's remove
     * member function.  Compares all three variables of the Client
     * class, so all must match to be equal.
     */
    int operator==( const Client &RHS ) const
    {
      if( verbose == RHS.verbose &&
          socket == RHS.socket &&
          format == RHS.format )
        return 1;
      return 0;
    }

    /**
     * Sets all fields to -1. This is useful for when a client needs to
     * be removed from an STL list
     */
    void clear()
    {
      socket = verbose = format = needsInfo = -1;
    }
};

/**
 * A single class to handle both reading and writing on a port forwarder
 * for the ncs program. This should not require port numbers, but instead
 * only depend of socket descriptors
 */
class Connection
{
  private:
    /**
     * whenever a connection has a reader connected, and its dataQueue has
     * data available, this shared variable will be turned on.
     */
    static int readersWaiting;
        ///The name descriptor used to find this connection for adding new readers/writers
    char *name;
        ///All clients who have connected that will read as data is queued up
    list <Client> readerDescriptors;
        ///This is a list, but only one writer can be handled.
    list <Client> writerDescriptors;
        ///Pointer to the server's descriptor set, so that bits can be turned on/off
    fd_set *allset;
        ///Pointer to the server's descriptor max, so that it can be set higher
    int *maxDescriptor;
        ///Number of elements that Constant binary readers/writers will receive
    int nElements;
        ///Size of each element that Constant binary reader/writer will receive
    int sizeElements;
        /**
         * Maintain a pool of allocated memory. If a new message arrives with
         * different length, just realloc to larger sizes (when necessary), but
         * don't worry about shrinking lengths or finding more appropriately sized
         * chuncks
         */
    static stack<char *> memoryPool;
    static stack<int> memorySizes;

        ///store data for readers
    queue <char *>asciiData;
    queue <int>asciiSize;
    queue <char *>binaryData;
    queue <int> binarySize; 
        ///The amount of memory allocated may be more than needed since we try to reuse older blocks
    queue <int> asciiMemory;
    queue <int> binaryMemory;

        ///Buffer to store data for future parsing
    char *pendingData;
        ///The current amount of data stored for future parsing
    int pendingSize;
        ///The maximum amount of data that can be stored for future parsing
    int pendingLimit;

       ///0 = ascii, 1 = binary - not used
       //int inputFormat, outputFormat;

    int verbose;

  public:
    Connection();

    ~Connection();

    /**
     * Assign pointer to the file descriptor set.  This will allow new clients
     * to be included in the "select" system call.
     * @param newAllset Memory location of the file descriptor set.
     */
    void setAllset( fd_set *newAllset );

    /**
     * Assign pointer to the maximum value of allocated file desriptors.  When
     * the "select" call is made, it will only check file descriptors up to a
     * certain number.  If a client connects, and is given a higher number than the
     * current maximum, then that maximium must be updated so that any new clients are included
     * in the "select" system call.
     * @param newMaxDesriptor Memory address where the current maximum file descriptor is stored.
     */
    void setMaxDescriptor( int *newMaxDescriptor );

    /**
     * Change the format used to output data to readers
     */
    int setOutputFormat( int newFormat );

    /**
     * Change the format used receive input data from writer(s?)
     */
    int setInputFormat( int format );

    /**
     * Changes name to new Name while handling memory allocation.  When a client is searching
     * for the correct Connection object to connect to, it uses the name to identify the 
     * Connection.  NCS creates a name by combining the Brain type, the Brain job, and the
     * report/stimulus filename into one word (with no separator).  The only exception is
     * the Brain command connection which only uses the Brain type and Brain job to
     * construct a name.
     * @param newName A character string to be duplicated.
     */
    void setName( char *newName );

    /**
     * Determine if a given name matches the name on this connection
     * @param compareName The name to compare with
     * @return 1 on match, 0 on no match, -1 on error
     */
    int checkName( char *compareName );

    /**
     * Insert a client that will be sent data as it is received.
     * @param sd The socket Descriptor of the new client
     * @param format 0 for ascii (default), 1 for binary, 2 for constant binary
     * @param verbose 0 for no output (default), 1 for output
     */
    int addReader( int sd, int format=0, int verbose=0 );

    /**
     * Insert a client that will put data onto the data queue(s).  Note - only the
     * first writer will be allowed to contribute data.  Any other writers will be
     * added to the list, but will not be read from until the first writer disconnects
     * and the next writer advances to the front of the list.
     * @param sd The socket Descriptor of the new client
     * @param format 0 for ascii (default), 1 for binary, 2 for constant binary
     * @param verbose 0 for no output (default), 1 for output
     */
    int addWriter( int sd, int format=0, int verbose=0 );

    /**
     * Visit each reader and send it the next item from the appropriate data queue.  That
     * is, readers requesting ascii data will receive data in ascii format, binary readers
     * will receive the data in binary format.
     * @param active The set of file descriptor that are active. If a reader has activated
     *  the file decriptor set, it indicates that the reader has disconnected and needs to
     *  be removed.
     */
    void processReaders( fd_set &active );

    /**
     * Visit the first writer and receive data or remove it from the list of writers if it
     * has disconnected.
     * @param active The set of file descriptor that are active.
     */
    void processWriters( fd_set &active );

    /**
     * Separate data read from the writer into packets.  Data received from the writer
     * should be in the form: number separator data.  data is the actual data that should
     * be added to the queue, number is the length of just the data, and separator is a white
     * space character including space (' '), newline ('\n'), tab ('\t'), or null ('\0').
     * Example: 5 0.346
     */
    int parse();

    /**
     * After data has been collected from the writer, it is added to a block of pending
     * data waiting to be parsed.
     * @param buffer The data read from the writer
     * @param nread The number of bytes read from the writer
     */
    int append( char *buffer, int nread );
    
    /**
     * After a block of actual data has been separated from the block of pending data, it
     * is added to the data queues in preparation for when readers request it.
     * @param buffer Pointer to parsed data
     * @param count Number of bytes in data
     * @param memorySize The amount of memory allocated for data may be larger than needed
     */
    void queueData( char *buffer, int count, int memorySize );

    /**
     * If the data received is in ascii, it needs to be converted before any binary readers
     * can receive it.
     * @param buffer Pointer to parsed data
     * @param count Number of bytes in data.
     */
    void asciiToBinary( char *buffer, int count );
 
    /**
     * If the data received is in binary, it needs to be converted before any ascii readers
     * can receive it.
     * @param buffer Pointer to parsed data
     * @param count Number of bytes in data
     */
    void binaryToAscii( char *buffer, int count );

    /**
     * Single function for the main server program to call that will go to process
     * the reader and writer clients.
     * @param active Set of active file descriptors to check against the client file descriptors
     */
    void process( fd_set &active );

    /**
     * Determine if any readers or writers are connected.
     * @return 1 if there is at least one reader or writer, otherwise 0.
     */
    int empty();

    /**
     * Diconnects all readers and writers and clears all data queues.
     */
    void reset();

    /**
     * Allows the flag that indicates that a reader in any connection
     * object is waiting (and more data is on the queue for it) to be
     * set.
     * @param flag Value to set the pending reads flag to.
     */
    static void setPendingReads( int flag );

    /**
     * Returns whether readers in any connection object are waiting and 
     * more data is on the queue, then clears the flag.
     * @return 1 if readers waiting, 0 if not
     */
    static int pendingReadsAndClear();

    /**
     * Returns whether readers in any connection object are waiting for
     * more data and that data is on the queue
     * @return 1 if readers are waiting, 0 if not
     */
    static int pendingReads();

    /**
     * Peek at first item without removing it from the queue.
     * @return pointer to first item in the ascii data queue.
     */
    char *peek();

    /**
     * Sets flag so that when activity occurs on this connection, messages will
     * be sent to the stderr stream or those messages will be suppressed.
     * @param flag 0 to deactive the verbose feature, 1 to active it
     */
    void setVerbose( int flag );

    /**
     * Before a constant binary reader can get data, it needs to know how many elements there are,
     * and the size of each element.  When that information is available (i.e. a writer has
     * connected ) it will be sent once.  This function will be called if a reader connects, and 
     * a writer has connected previously with the information, or a writer connects, and a reader
     * is waiting for this information
     * @param sd the socket the reader will use
     * @return 0 on  success, -1 on failure
     */
    int giveReaderHeaderInfo( int sd );

    /**
     * When a constant binary writer connects, it will send the number of elements and the size of
     * those elements.
     * @param sd The socket the writer will use
     * @return 0 on success, -1 on failure
     */
    int getWriterHeaderInfo( int sd );

    /**
     * Get memory from first element of class's memory pool.  Enlarge memory size if necessary.
     * Do not shrink to fit.  If no chunks are available, allocate another.
     * @param requestedSize The amount of memory needed in the returned buffer
     * @param memoryAllocated The amount of actual memory in the returned buffer.  This needs to be
     * stored so that when the memory is returned, the size can be recalled.
     * @return A pointer to the memory buffer.
     */
     static char *acquireMemory( int requestedSize, int &memoryAllocated );

    /**
     * Return memory to the memory pool.
     * @param memory pointer to allocated memory
     * @param emoryAllocated The size of the memory in bytes
     */
     static void releaseMemory( char *memory, int memoryAllocated );
};


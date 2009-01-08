//Jim King
//storage.cpp
//stimulus storage for singlestream class, perhaps can be used by connection class

#include <queue>
#include <vector>

using namespace std;

/**
 * Handle data management for stimulus coming in/out of a singlestream object.
 * 
 */
class Storage
{
    private:
        /**
         * whenever a connection has a reader connected, and its dataQueue has
         * data available, this shared variable will be turned on.
         */
        static int readersWaiting;
            ///The name descriptor used to find this connection for adding new readers/writers
        char *name;
            ///Number of elements that Constant binary readers/writers will receive
        int nElements;
            ///Size of each element that Constant binary reader/writer will receive
        int sizeElements;

           ///store data for readers
        queue <char *>asciiData;
        queue <int>asciiSize;
        queue <char *>binaryData;
        queue <int> binarySize; 

            ///Buffer to store data for future parsing
        char *pendingData;
            ///The current amount of data stored for future parsing
        int pendingSize;
            ///The maximum amount of data that can be stored for future parsing
        int pendingLimit;
            ///Buffer shared by all Storage instances for shifting data
        static char *shiftBuffer;
            ///Amount of memory allocated to the shiftBuffer
        static int shiftSize;

           ///0 = ascii, 1 = binary - not used
           //int inputFormat, outputFormat;

        int verbose;
    
    public:
        /**
         * Constructor - create storage for Stimulus data
         * @param Name The name assigned to this storage unit
         * @param Key The key assigned to this storage unit
         */
        Storage( char *Name, int Key );
        
        /**
         * Destructor - Free memory
         */
        ~Storage();
        
        //Accessor functions
        
        /**
         * Retrieve the key used by this storage unit
         * @return Integer key used to identify this storage unit
         */
        int getKey();
        
        /**
         * After data has been collected from the writer, it is added to a block of pending
         * data waiting to be parsed.
         * @param buffer The data read from the writer
         * @param nread The number of bytes read from the writer
         */
        int append( char *buffer, int nread );

        /**
         * Separate data read from the writer into packets.  Data received from the writer
         * should be in the form: number separator data.  data is the actual data that should
         * be added to the queue, number is the length of just the data, and separator is a white
         * space character including space (' '), newline ('\n'), tab ('\t'), or null ('\0').
         * Example: 5 0.346
         */
        int parse();

};

#ifndef SERVERTYPE
#define SERVERTYPE

/*
 *  servertype.h
 *  Thesis
 *
 *  Created by James King on Thu Mar 04 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <queue>
#include <list>

using namespace std;

/**
 * The original object used to manage data flow into and out of the
 * server program.  This Server object handled one port and clients
 * could connect to it.  Is mostly deprecated with creation of
 * the Connection object.
 */
class Server
{
  public:
    ///File descriptor returned by socket system call
    int socketHandle;
    ///Port on host that clients will connect to
    int port;
    ///When data is exchanged, will it be in ascii/binary format?
    char dataFormat;
    ///data waiting to go out - only used by writers
    queue <char*> dataQueue;
    ///nuber of bytes in data
    queue <int> dataSize;
    ///socket descriptor for clients connected
    list <int> clients;
    ///holder for leftover data
    char* remainingData;
    ///length of remaining data
    int remainingLength;
    ///If this is true, them the server should output activity to stdout
    bool verbose;
    ///For automatic port assignments, use a name to reference the server
    char *name;

    Server();
    ///add some amount of data read off a socket onto the remainingData
    void append( int &length, char* &buffer );
    ///do as best as you can to parse data from remaining data
    int parse();
    ///set whether the server should be verbose or not
    void setVerbose( int mode );
    ///clear queues, pending data, clients
    void reset();

    /**
     * Sets the name of this server to a newName. If this server already had
     * a name, it deallocates that original name, and reallocates memory
     * for the new name.
     * @param newName String to set this server's name to.
     */
    void setName( char *newName );

    /**
     * Check whether a string is the same as this server's name, and return
     * this server's port value if it is; otherwise, return -1.
     * @param targetName String to compare this server's name with
     * @return -1 if not a match, this server's port value if the names match
     */
    int getPort( char *targetName );
    
};

#endif

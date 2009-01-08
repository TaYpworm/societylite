/*
 *  manage_server.h
 *  Thesis
 *
 *  Created by James King on Sun Feb 15 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include "socket_accept.h"
#include "servertype.h"
#include "serverconsts.h"

//@Include: connection.h
//@Include: socket_accept.c

class ReportInfo;

/**
 * Regulate the incoming and outgoing of data communications
 * @param serverSocket Socket created for dynamic commands and connections
 * @param writers Sockets opened statically using command-line configuration file
 * @param readers Sockets opened statically using command-line configuration file
 * @param verboseFlag Indicates if output should be logged to stdout
 */
void manageServer( const int &serverSocket, vector<Server> writers, vector<Server> readers, int verboseFlag );

/**
 * Static writers/readers and Connection objects may have verbose setting changed
 * during exectution.  In the case on Connection objects, this will create a new
 * Connection if no matching name is found.
 * @param fin Socket Reader will determine which server needs to be set and to what mode.
 */
int setVerbose( FILE *fin );

/**
 * Reads local file server.apps to determine which applications this server is allowed
 * to send to the Queue.  Names are stored in global vector appList.
 */
void loadAppList();

/**
 * Reads local file server.scripts to determine which scripts this server is allowed
 * to run. Names are stored in global vector sriptList.
 */
void loadScriptList();

/**
 * Reloads local file server.apps to determine which applications may be launched with
 * the server program and sends a list to the requesting client.  First, the total number
 * of bytes will be sent.  Then, the application names, separated by newlines ('\n').
 * The client can refer to the applications by index number, where the first name is index
 * zero (0), the second is index one (1), etc.
 * @param clifd Socket Descriptor to send data on.
 * @return 0 on success 
 */
int provideAppList( int clifd );

/**
 * Reloads local file server.scripts to determine which scripts may be invoked with
 * the server program and sends a list to the requesting client. First, the total number
 * of bytes will be sent. Then, the script names, separated by newlines ('\n').
 * The client can refer to the scripts by index number, where the first name is index
 * zero (0), the second is index one (1), etc.
 * @param clifd Socket Descriptor to send data on.
 * @return 0 on success.
 */
int provideScriptList( int clifd );

/**
 * Link client up with a Connection object based on a Name, or create a new Connection object
 * if one by that Name does not exist. The name's root is formed from the Brain's Type and Job.
 * If the Connection is for Brain Commands, no more is needed.  For Reports and Stimulus, the
 * filename will also be sent as part of the name.  The client also sends the mode - reader or
 * writer.  Finally, the client sends the format - ascii, binary, or constant binary.  See the
 * Connection object for details on the mode and format.
 * @param clifd Socket Descriptor that the client is using.
 * @param fin Reader for the socket to assist in collection of initialization data
 */
int autoPort( int clifd, FILE *fin );

/**
 * Accepts connections to the Main Server and prepares the client for reading.
 * @param serverSocket Socket Descriptor of the Main Server.
 */
int handleMainServerConnection( int serverSocket );

/**
 * Interpret commands sent to the Main Server.
 * @param fd Socket Descriptor where commands will be read from.
 */
int mainCommands( int fd );

/**
 * For static ports, convert data between ascii and binary if the reader and writer
 * want different formats.
 * @param data The original data as received from the writer client.
 * @param size The number of bytes in data.
 * @param inputFormat The format (ascii or binary) used by the writer client.
 * @param outputFormat The format desired by the reader client.
 * @param result The resulting data after conversion.
 * @param finalSize The number of bytes in result.
 * @return 1 if conversion made, 0 if no conversion needed
 */
int convertData( char* &data, int size, char &inputFormat, char &outputFormat,
                 char* &result, int &finalSize );

/**
 * Before using the system call "select", a File Decriptor set must be built using the
 * active server sockets (i.e. the Static Servers).
 * @param allset The File Descriptor set that will ultimately be used
 * @param writeServers Vector containing the write servers
 * @param readServers Vector containing the read servers
 * @return The maximum socket descriptor in use by any of the servers
 */
int setServerBits( fd_set &allset, vector<Server> writeServers, vector<Server> readServers );

//Signal catchers

/**
 * Catch Interrupt Signal from user (ctrl-c).  Close open sockets.
 */
static void sig_int( int signo );

/**
 * Catch Broken Pipe Signal.  Occurs when data is trying to be sent to a
 * disconnected reader.  Just remove the faulted reader from consideration.
 */
static void sig_pipe( int signo );

/**
 * Catch Child Exit Signal.  May be used in future if better fork/threading
 * is needed for server.
 */
static void sig_chld( int singno );


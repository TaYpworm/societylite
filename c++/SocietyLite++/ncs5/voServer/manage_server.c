/*
 *  manage_server.c
 *  Thesis
 *
 *  Created by James King on Sun Feb 15 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <signal.h>
#include "manage_server.h"
#include "server_listen.h"
#include "connection.h"
#include "reportinfo.h"
#include "singlestream.h"
//#include "QQ.h"

#include <iostream>
#include <vector>
#include <queue>
#include <list>
#include <sys/wait.h>
#include <fstream>

using namespace std;

//integers for profiling
int eventID[10];
int gatherTime;

fd_set *allsetLink;
int *maxLink;

//for named connections
vector<Connection> dynamicConnections;

//for single socket connections
list<SingleStream *> singleConnections;
list<int> additionalIDs;

vector<char*> appList;
vector<char*> scriptList;

#define RESET -3

//these pointers are to be used only when the program is killed and connections need to be closed
vector<Server> *wServe, *rServe;

//If verbose output should be sent to stdout, set this to 1
int globalVerbose = 0;

//prototypes
int handleWriteServer( Server &writer );
int handleWriteClient( Server &writer, fd_set &rset, fd_set &allset );
int handleReadClient( Server &reader, Server &writer, fd_set &rset, fd_set &allset );
void prepProfile();

//-----------------------------------------------------

void loadScriptList()
{
  char *helper;
  char current[MaxLength];
  ifstream fin( "server.scripts" );

  if( !fin )
    return;

  //clear scriptList if it already has stuff
  for( unsigned int i=0; i<scriptList.size(); i++ )
    delete [] scriptList[i];
  scriptList.clear();

  while( fin>>current )
  {
    helper = new char[strlen(current)+1];
    strcpy( helper, current );
    scriptList.push_back( helper );
  }

  fin.close();
}

//----------------------------------------------------

void loadAppList()
{
  char current[256];
  char *helper;
          
  ifstream fin( "server.apps" );

  if( !fin )
    return;

  //clear appList if it already has stuff
  for( unsigned i=0; i<appList.size(); i++ )
    delete [] appList[i];
  appList.clear();

  while( fin>>current )
  {
    helper = new char[256];
    strcpy( helper, current );
    appList.push_back( helper );
  }  

  fin.close();
}

//---------------------------------------------------------

void prepProfile()
{
  char event[7][64] = {"setpath", "getdata", "gettime", "setpattern", "launch",
                    "reportcount", "mkdir" };

  //for( int i=0; i<7; i++ )
    //eventID[i] = QQAddState( event[i] );

  //gatherTime = QQAddState( "gather" );
}

//---------------------------------------------------------

void manageServer( const int &serverSocket, vector<Server> writeServers, vector<Server> readServers, int verboseFlag )
{
    int i, n, maxDescriptor, maxi, clifd, nread;
    const int MAXLINE = 1024;
    int attemptPort, newSocket;
    char buf[MAXLINE];
    uid_t uid;
    fd_set rset, allset;
    struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 10;  //if a timeout is needed for select, wait 10 microseconds
    int useTimeout = 0;
    unsigned int index;
    //int QQload;

    //does the user want verbose output?
    globalVerbose = verboseFlag;

    ///global pointers to descriptor set info
    allsetLink = &allset;
    maxLink = &maxDescriptor;

    //QQBaseTime(0);

    //create new event to monitor load functinos
    //QQload = QQAddState( "load" );

    //QQStateOn (QQload);  //Begin monitoring

    loadAppList();
    loadScriptList();

    //QQStateOff (QQload); //End monitoring
    
    //reset all file descriptors
    FD_ZERO( &allset );   

    //prep the SingleStream class for when it is time to add connections
    SingleStream::init( &allset, &maxDescriptor, &appList, &scriptList );
 
    //add the server's socket descriptor to the list
    if( serverSocket > -1 )
    {
        FD_SET( serverSocket, &allset );
        maxDescriptor = serverSocket;
    }

    //add all the file descriptor and determine the hightest value among them
    maxDescriptor = setServerBits( allset, writeServers, readServers );
    if( maxDescriptor < serverSocket ) //not likely
      maxDescriptor = serverSocket;
        
    //setup global pointers so that a kill signal can close open ports/sockets
    wServe = &writeServers;
    rServe = &readServers;

    //prepare to catch a broken pipe signal
    if( signal( SIGPIPE, sig_pipe ) == SIG_ERR )
      cerr<<"Unable to setup signal handler\n";
    if( signal( SIGINT, sig_int ) == SIG_ERR )
      cerr<<"Unable to setup signal handler\n";
    if( signal( SIGCHLD, sig_chld ) == SIG_ERR )
      cerr<<"Unable to setup child handler\n";

    prepProfile();
            
    while (1)
    {
      rset = allset;  //get ready to check sockets

      //check if any of my servers have connections

      //give select a timeout if there is data available for readers to access;
      //otherwise, use NULL for infinite timeout
      if( useTimeout || Connection::pendingReadsAndClear()  )
      {
        if(( n=select(maxDescriptor + 1, &rset, NULL, NULL, &tv)) < 0 )
          fprintf( stderr, "select error with timeout\n" );
      }
      else if(( n=select(maxDescriptor + 1, &rset, NULL, NULL, NULL)) < 0 )
      {
        //don't need to worry about interrupts; report other errors
        if( !(errno & EINTR) )
          perror( "select error" );
        continue;
      }

      //reset useTimeout
      useTimeout = 0;

      //check for connections to the primary server
        if( serverSocket > -1 && FD_ISSET( serverSocket, &rset))
        {
            clifd = handleMainServerConnection( serverSocket );
            if( clifd == RESET )
            {
                cerr<<"resetting\n";
                //clear all server queues, remaining data, and close all clients
                for( int j=0; j<writeServers.size(); j++ )
                    writeServers[j].reset();
                for( int j=0; j<readServers.size(); j++ )
                    readServers[j].reset();
                for( unsigned int i=0; i<dynamicConnections.size(); i++ )
                  dynamicConnections[i].reset();
                
                //need to reset bits in allset
                FD_ZERO( &allset );   
 
                //add the server's socket descriptor to the list
                FD_SET( serverSocket, &allset );
                
                //add all the file descriptor and
                // determine the hightest value among them
                setServerBits( allset, writeServers, readServers );
            }
            else
            {
                if( clifd > 0 ) //add to my list
                {
                  if( globalVerbose )
                    cout<<"Adding clifd: "<<clifd<<endl;
                  FD_SET( clifd, &allset );
                  if( clifd > maxDescriptor )
                    maxDescriptor = clifd;
                  additionalIDs.push_back( clifd );
                }

                continue;
            }
        }

    int code = 0;
    for( list<int>::iterator it=additionalIDs.begin(); it!=additionalIDs.end(); it++ )
    {
//cerr<<"checking existing connections for primary commands\n";
      code = mainCommands(*it);
      if( code < 0 )            //client has disconnect
      {
        FD_CLR( *it, &allset );
        close( *it );
        *it = -1;
      }
    }
    additionalIDs.remove(-1);

    //any dynamic Connections I need to worry about?
    for( index = 0; index<dynamicConnections.size(); index++ )
      dynamicConnections[index].process( rset );

    //any single Stream connections need processing?
    //cerr<<"checking single streams\n";
    for( list<SingleStream*>::iterator it=singleConnections.begin();
         it != singleConnections.end(); it++ )
    {
      if( (*it)->process() < 0 )
      {
        (*it)->clear();
        delete *it;
        *it = NULL;
      }
      else if( (*it)->isSending() )
      {
        useTimeout = 1;
      }
    }
    singleConnections.remove( NULL );

    //any one connected to the config.txt servers?
    //first the writer - I should check to make sure that only one writer is ever connected
    for( int index=0; index<writeServers.size(); index++ )
    {
      if( FD_ISSET( writeServers[index].socketHandle, &rset) ) //someone new has connected
      {
        if( (clifd = handleWriteServer( writeServers[index] ) ) > 0 )
        {
          //cerr<<"adding new write client\n";
          //add new fd to allset
          FD_SET( clifd, &allset );
          if( clifd > maxDescriptor )
            maxDescriptor = clifd;
        }
      }
    }

    //now for the readers - any number can connect to these guys
    for( int index=0; index<readServers.size(); index++ )
    {
      if( FD_ISSET( readServers[index].socketHandle, &rset) ) //someone new has connected
      {
        //cerr<<"new read server connection\n";
        //get new file descriptor, append to this server's client list
        if( (clifd=acceptConnection( readServers[index].socketHandle, &uid)) < 0 )
          fprintf( stderr, "serv_accept error: %d\n", clifd );

        readServers[index].clients.push_back( clifd );

        //add new fd to allset
        FD_SET( clifd, &allset );
        if( clifd > maxDescriptor )
          maxDescriptor = clifd;
      }
    }

    //cerr<<"checking writer clients\n";
    //check clients
    for( int index=0; index<writeServers.size(); index++ )
      handleWriteClient( writeServers[index], rset, allset );
    
    //the reader clients; if there if new data for them; send it and dequeue that data
    //make sure there is at least one client connected before dequeing data
    for( int index=0; index<readServers.size(); index++ )
    {
        handleReadClient( readServers[index], writeServers[index], rset, allset );
        if( !writeServers[index].dataQueue.empty() )
            useTimeout = 1;
    }

  }//end infinite while

}//end func

/**
 * For handling connections to the command port
 */
int handleMainServerConnection( int serverSocket )
{
  char buffer[ MaxLength ];
  int n, clifd;
  uid_t uid;
  int errorCode;

  //check for request to be connected to a named port
  //accept new client request
  if( (clifd=acceptConnection( serverSocket, &uid)) < 0 )
  {
    fprintf( stderr, "serv_accept error: %d\n", clifd );
    return -1;
  }

  return mainCommands( clifd );
}

//-----------------------------------------------------------

int mainCommands( int clifd )
{
  char buffer[MaxLength];
  int errorCode;
  SingleStream *addStream;

  //need a file pointer to use fgets
  FILE *fin = fdopen( clifd, "r+" );
  setbuf( fin, NULL );                //so that the stream does not read in too much

  //read until a newline - this keeps newline
  if( !fgets( buffer, MaxLength, fin ))
    return -1; //no characters read, socket should be closed

  if( globalVerbose )
    cout<<"Command: "<<buffer;

  //remove the newline
  if( strlen( buffer ) > 0 )
    buffer[ strlen( buffer ) - 1 ] = 0;

  //reset server?
  if( strcasecmp( buffer, "reset" ) == 0 )
  {
    close( clifd );
    return RESET;
  }
  else if( strncmp( buffer, "verbose", 7 ) == 0 )  //turn on/off verbose for the socket
  {
    setVerbose( fin );
  }
  else if( strncmp( buffer, "applist", 7 ) == 0 )
  {
    return provideAppList( clifd );
  }
  else if( strncmp( buffer, "scriptlist", 10 ) == 0 )
  {
    return provideScriptList( clifd );
  }
  else if( strncmp( buffer, "request", 7 ) == 0 )  // use Connection object
  {
    return autoPort( clifd, fin );
  }

  //if any of the following commands are called, then a single stream
  //connection needs to be created and used for the durration of that
  //client's lifetime

  else if( strncmp( buffer, "launch", 6 ) == 0 )
  {
    addStream = new SingleStream( clifd, globalVerbose );
    singleConnections.push_back( addStream );
    addStream->launchNCS();
    return 0;
  }
  else if( strncmp( buffer, "collect", 7 ) == 0 ) //collect file names
  {
    return 0;
  }
  else if( strncmp( buffer, "setpath", 7 ) == 0 ) //set directory and job
  {
    addStream = new SingleStream( clifd, globalVerbose );
    singleConnections.push_back( addStream );
    addStream->setPath();
    return 0;
  }
  else if( strncmp( buffer, "getdata", 7 ) == 0 )
  {
    addStream = new SingleStream( clifd, globalVerbose );
    singleConnections.push_back( addStream );
    addStream->getData();
    return 0;
  }
  else if( strncmp( buffer, "getavailabletime", 16 ) == 0 )
  {
    return 0;
  }
  else if( strncmp( buffer, "mkdir", 5 ) == 0 )
  {
    addStream = new SingleStream( clifd, globalVerbose );
    singleConnections.push_back( addStream );
    addStream->makeDirectory();
    return 0;
  }
  else if( strncmp( buffer, "exec", 4 ) == 0 )
  {
    addStream = new SingleStream( clifd, globalVerbose );
    singleConnections.push_back( addStream );
    addStream->invokeScript();
    return 0;
  }
  else if( strncmp( buffer, "managedata", 10 ) == 0 )
  {
    addStream = new SingleStream( clifd, globalVerbose );
    singleConnections.push_back( addStream );
    addStream->manageData();
    return 0;
  }
  else if( strncmp( buffer, "setpattern", 10 ) == 0 )
  {
    return 0;
  }

  else  //invalid command
  {
    //aquirePort();
    //cerr<<"output profile file\n";
    //char file[] = "QQoutput.txt";
    //QQOut( file, 0, 1 );
    return 0;
  }

  return clifd;
}

//------------------------------------------------------------------------

int autoPort( int clifd, FILE *fin )
{
  char buffer[MaxLength];
  int mode = 0, format = 0, verbose = 0;
  vector<char *> results;

  verbose = globalVerbose; //match the main server (but can be overridden)

  //mode r/w and verbose (optional)
  if( !fgets( buffer, MaxLength, fin ) ) //read until '\n' - what if MaxLength was not enogh?
  {
    //error
    return -1;
  }

  if( globalVerbose )
    cout<<"received mode: "<<buffer;

  //parse buffer (in case verbose is there)
  char *helper = strtok( buffer, " \n\t" );
  while( helper )
  {
    results.push_back( helper );
    helper = strtok( NULL, " \n\t" );
  }

  if( buffer[0] == 'r' || buffer[0] == 'R' ) mode = 0;
  else if( buffer[0] == 'w' || buffer[0] == 'W' ) mode = 1;
  else return -1;

  if( results.size() > 1 ) //may include "verbose"
    if( results[1][0] == 'v' )
      verbose = 1;

  //data format
  if( !fgets( buffer, MaxLength, fin ) ) //read until '\n' - what if MaxLength was not enogh?
  {
    //error
    return -1;
  }
  results.clear();

  if( globalVerbose )
    cout<<"received format: "<<buffer;

  if( buffer[0] == 'a' || buffer[0] == 'A' ) format = 0;
  else if( buffer[0] == 'b' || buffer[0] == 'B' ) format = 1;
  else if( buffer[0] == 'c' || buffer[0] == 'C' ) format = 2;
  else return -1;

  //name
  if( !fgets( buffer, MaxLength, fin ) )
  {
    //error
    return -1;
  }

  if( globalVerbose )
    cout<<"received name: "<<buffer;
  
  //remove any newlines
  for( int i=strlen( buffer )-1; buffer[i] == '\n'; i-- )
    buffer[i] = 0;

  //is there a connection already with this name?
  int found = -1;
  for( unsigned int index=0; index<dynamicConnections.size() && found<0; index++ )
  {
    if( dynamicConnections[index].checkName( buffer ) ) //match
      found = index;
  }

  if( found<0 ) //must create new connection
  {
    Connection latest;
    dynamicConnections.push_back( latest );
    found = dynamicConnections.size()-1;
    dynamicConnections[found].setName( buffer );
    dynamicConnections[found].setAllset( allsetLink );
    dynamicConnections[found].setMaxDescriptor( maxLink );
  }
  if( verbose )
    dynamicConnections[found].setVerbose( verbose );

  if( mode == 0 ) //add reader
    dynamicConnections[found].addReader( clifd, format );
  else            //addWriter
    dynamicConnections[found].addWriter( clifd, format );

  if( globalVerbose )
    cout<<endl;

  return 0;
}

//---------------------------------------------------------------------------

int provideScriptList( int clifd )
{
    int data = 0;
    char *dataBuffer;

    //reload server.scripts file incase it has changed
    loadScriptList();

    //count how many bytes are going to be sent
    for( unsigned int i=0; i<scriptList.size(); i++ )
      data += strlen( scriptList[i] ) + 1;

    //send data length and the data (using '\n' as separators)
    write( clifd, (char *) &data, 4 );

    if( data > 0 )
    {
      dataBuffer = new char[data+1];
      dataBuffer[0] = dataBuffer[data] = 0;
      for( unsigned int i=0; i<scriptList.size(); i++ )
      {
        strcat( dataBuffer, scriptList[i] );
        strcat( dataBuffer, "\n" );
      }
      write( clifd, dataBuffer, data );

      delete [] dataBuffer;
    }

  return 0;
}

//---------------------------------------------------------------------------

int provideAppList( int clifd )
{
    int data = 0;
    char *dataBuffer;

    //reload server.apps file incase it has changed
    loadAppList();

    //count how many bytes are going to be sent
    for( unsigned int i=0; i<appList.size(); i++ )
      data += strlen( appList[i] ) + 1;

    //send data length and the data (using '\n' as separators)
    //data = htonl( data );
    write( clifd, (char *) &data, 4 );

    if( data > 0 )
    {
      dataBuffer = new char[data+1];
      dataBuffer[0] = dataBuffer[data] = 0;
      for( unsigned int i=0; i<appList.size(); i++ )
      {
        strcat( dataBuffer, appList[i] );
        strcat( dataBuffer, "\n" );
      }
      write( clifd, dataBuffer, data );

      delete [] dataBuffer;
    }

  return 0;
}

//----------------------------------------------------------------------------

int setVerbose( FILE *fin )
{
    char buffer[ MaxLength ];

    //are there anymore arguments? - expect an address (port # or connection name) and a mode
    fgets( buffer, MaxLength, fin );

    vector <char*> results;
    char *helper = strtok( buffer, " \t\n" );
    while (helper != NULL )
    {
      results.push_back( helper );
      helper = strtok( NULL, " \n" );
    }

    //if no third arg is given (ex. "verbose 10100" ), just toggle
    if( results.size() > 0 )
    {
      int id = atoi( results[0] );
        //if( id > 0 ) //must be a port
        //else try using as a name
      int mode = 2;

      //determine whether to turn verbose on/off/toggle
      if( results.size() > 1 )
      {
        if( strcasecmp( results[1], "on" ) == 0 )
          mode = 1;
        else if( strcasecmp( results[1], "off" ) == 0 )
          mode = 0;
        //else //if( strcasecmp( results[1], "toggle" ) == 0 )
          //mode = 2;
      }

      //cerr<<"target is: "<<results[0]<<endl<<"mode is "<<mode<<endl;

      //now find the server with id matching
      if( id > 0 )
      {
        for( int i=0; i<rServe->size(); i++ )
          if( (*rServe)[i].port == id )
            (*rServe)[i].setVerbose( mode );
        for( int i=0; i<wServe->size(); i++ )
          if( (*wServe)[i].port == id )
            (*wServe)[i].setVerbose( mode );
      }
      else if( id == 0 ) //try finding connection with name
      {
        int found = 0;
        for( unsigned int i=0; i<dynamicConnections.size(); i++ )
          if( (found = dynamicConnections[i].checkName( results[0] )) )
            dynamicConnections[i].setVerbose( mode );

        if( !found ) //create new connection - without readers/writers
        {
//cerr<<"creating new server\n";
          Connection latest;
          dynamicConnections.push_back( latest );
          found = dynamicConnections.size()-1;
          dynamicConnections[found].setName( results[0] );
          dynamicConnections[found].setAllset( allsetLink );
          dynamicConnections[found].setMaxDescriptor( maxLink );
          dynamicConnections[found].setVerbose( mode );
        }
      }

    }
    else
      cerr<<"usage: verbose <port> [on|off|toggle]\n";

    return 0;
}

//------------------------------------------------------------------------------

int convertData( char* &data, int size, char &inputFormat, char &outputFormat, char * &result, int &finalSize )
{
	vector<float> helper;
	char *target = NULL;
	char *ptr = data;
	int fflag, latest;
    
	//ascii -> binary
	if( (inputFormat == 'a' || inputFormat == 'A') && (outputFormat == 'b' || outputFormat == 'B' ))
	{
		//use atof to grap numbers off the string and put them onto a new string
		//go through string, finding numerical separators, and use atof to make them floats
		helper.push_back( atof( ptr ) ); //first number - what if string did not start with number?
		int fflag = 1; //we are starting in a number already - assumption
		int latest = 0;
		for( int index=0; index<size; index++ )
		{
			//look for 0-9 or a '.'
			if( !fflag && (( ptr[index] >= '0' && ptr[index] <= '9' ) || ptr[index] == '.' ))
			{
				//start a new number
				fflag = 1;
				target = &ptr[index];
				helper.push_back( atof( target ) );
			}
			else if ( fflag && !(( ptr[index] >= '0' && ptr[index] <= '9' ) || ptr[index] == '.' ))
			{
				//end current number
				fflag = 0;
			}
		}
        
        //write binary data to stream
		//copy floats into a simple string
		finalSize = sizeof(float) * helper.size();
		result = new char[finalSize];
		memcpy( result, &helper[0], finalSize );
		return 1;
		//rc = fwrite( &(helper[0]), helper.size(), sizeof( float ), dest );
	}
	//binary->ascii
	else if( (inputFormat =='b' || inputFormat == 'B') && (outputFormat == 'a' || outputFormat == 'A' ) )
	{
		//assume data is in float format
		//size should be in multiple of 4 = sizeof(float)
		int floatCount = size/sizeof(float);
		helper.resize( floatCount );
		for( int i=0; i<floatCount; i++ )
		{
			//sprintf( 
		}
		return 1;
	}
	else
	{
		finalSize = 0;
		result = 0;
		return 0;
	}
}

//------------------------------------------------------------

int setServerBits( fd_set &allset, vector<Server> writeServers, vector<Server> readServers )
{
	int maxDescriptor = 0;
	
	for( int index=0; index<writeServers.size(); index++ )
	{
		FD_SET( writeServers[index].socketHandle, &allset );
		if( maxDescriptor < writeServers[index].socketHandle )
			maxDescriptor = writeServers[index].socketHandle;
	}
	for( int index=0; index<readServers.size(); index++ )
	{
		FD_SET( readServers[index].socketHandle, &allset );
		if( maxDescriptor < readServers[index].socketHandle )
			maxDescriptor = readServers[index].socketHandle;
	}
	
	return maxDescriptor;
}

//-----------------------------------------------------------------------

int handleWriteServer( Server &writer )
{
	int clifd;
	uid_t uid;

	//get new file descriptor, append to this server's client list
	if( (clifd=acceptConnection( writer.socketHandle, &uid)) < 0 )
		fprintf( stderr, "serv_accept error: %d\n", clifd );
	if( writer.clients.empty() )
		writer.clients.push_back( clifd );
	else
	{
		cerr<<"Error: multiple writers attempting to connect\n";
		cerr<<"Ignoring new connection\n";
		close( clifd );
		return -1;
	}

	return clifd;
}

//--------------------------------------------------------------------

int handleWriteClient( Server &writer, fd_set &rset, fd_set &allset )
{
	int clifd;
	const int MAXLINE = 1024;
	int nread = 0;
	int count, total;
	char buf[MAXLINE];
	
	for( list<int>::iterator clientIterator = writer.clients.begin(); clientIterator != writer.clients.end(); clientIterator++ )
	{
		if( FD_ISSET( *clientIterator, &rset) ) //activity on this client - read data
		{
			clifd = *clientIterator;

			//read data off port, as much as I can, but not more than MAXLINE
			if(( nread = read( clifd, buf, MAXLINE )) < 0 )
				fprintf( stderr, "read error on fd %d\n", clifd );
			else if( nread == 0 )
			{
				//cerr<<"Client connection closed - disconnecting\n";
				FD_CLR( clifd, &allset );
				close(clifd);
				*clientIterator = -1;
			}
			else
			//if( nread > 0 )//else  //read data into buffer, store on server
			{
				//give newly acquired data to the server
				char *blah = buf;
				writer.append( nread, blah );

				//the server will break up data using the formula
				//nbytes[ws]123...nbytes[ws]123...
				//putting the 123... onto the queue
				//until there is an incomplete set
				writer.parse();
				
				//what should happen if parse returns 0 successful parsings?
			}
		} //end if FD_ISSET
	} //end loop
	
	//remove all -1's from list - those are now defunct clients
	writer.clients.remove( -1 );
}

//------------------------------------------------------------------

int handleReadClient( Server &reader, Server &writer, fd_set &rset, fd_set &allset )
{
    int clifd = 0;

    //does the writer for this server have data?
    if( writer.dataQueue.empty() || reader.clients.empty() )
    {
        //The writer has no data for this reader
        //make sure a reader client isn't trying to disconnect
        for( list<int>::iterator clientIterator = reader.clients.begin(); clientIterator != reader.clients.end(); clientIterator++ )
        {
            //cerr<<"analysing reader client\n";
            clifd = *clientIterator;

            //check to be sure clients don't need disconnecting
            if( FD_ISSET( *clientIterator, &rset) ) //activity on this client - needs to be disconnected since readers should never activate the bit
            {
                //cerr<<"closed\n";
                FD_CLR( clifd, &allset );
                close(clifd);

                //-1 indicates that this should be removed when the time is right
                *clientIterator = -1;
            }
        }
    }
    else
    {
        //cerr<<"data is available\n";
        char *dat = writer.dataQueue.front();
        int size = writer.dataSize.front();

        //do I need to convert data?
        char *result;
        int finalSize;
        convertData( dat, size, writer.dataFormat, reader.dataFormat, result, finalSize );

        //what was returned?
        if( finalSize && result )  //data was converted - need to patch pointers
        {
            dat = result;
            size = finalSize;
        }

        int nwritten = 0;

        list<int>::iterator clientIterator;
        for( clientIterator = reader.clients.begin(); clientIterator != reader.clients.end(); clientIterator++ )
        {
            clifd = *clientIterator;
            //cerr<<"clifd: "<<clifd<<" send "<<size<<" bytes"<<endl;
            //I don't think I need to check FD_ISSET, do I?

            //write data to the clients; need to make sure I don't get a broken pipe
            //need to set up a signal catcher
            //I also need to see if I need to convert the data

            if( (nwritten = write( clifd, dat, size )) < 0 )
            {
                //I probably got a broken pipe signal; so unless I caught the signal,
                //I have crashed
                //remove clifd from list
                FD_CLR( clifd, &allset );
                close(clifd);
                //readServers[index].clients.erase( clientIterator );
            }
                    
            //otherwise, write was probably successful
            if( reader.verbose )
            {
                cout<<"[ "<<reader.port<<" ]wrote "<<nwritten<<" bytes\n";
                if( reader.dataFormat == 'A' ||
                    reader.dataFormat == 'a' )
                {
                    cout<<dat<<endl;
                }
            }
        }//end for each server client
                
        //I need to free memory!
        delete [] writer.dataQueue.front();

        writer.dataQueue.pop();
        writer.dataSize.pop();
    } //end else

    //remove disconnected clients
    reader.clients.remove( -1 );
}

//-----------------------------------------------------------------

void sig_pipe( int signo )
{
  //catch sig pipe and return
  return;
}

//------------------------------------------------------------------

void sig_chld( int signo )
{
  //accept child's return code
  if( globalVerbose )
      cerr<<"done executing sim\n";
  wait(0);
  return;
}

//------------------------------------------------------------------

void sig_int( int signo )
{
    //user is quitting, close all open ports/sockets/etc.
    cout<<"closing all sockets\n";

    //reset all connections
    
    //disconnect all clients - writers and readers
    for( int j=0; j<wServe->size(); j++ )
        for( list<int>::iterator i=(*wServe)[j].clients.begin(); i != (*wServe)[j].clients.end(); i++ )
            close( *i );
    for( int j=0; j<wServe->size(); j++ )
        for( list<int>::iterator i=(*rServe)[j].clients.begin(); i != (*rServe)[j].clients.end(); i++ )
            close( *i );
    
    //close all server ports
    for( int j=0; j<wServe->size(); j++ )
        close( (*wServe)[j].socketHandle );
    for( int j=0; j<wServe->size(); j++ )
        close( (*rServe)[j].socketHandle );
    
    //exit
    exit(0);
}


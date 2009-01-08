//Jim King
//connection.cpp

#include "connection.h"

#include <iostream>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>

using namespace std;

// Init static variables

int Connection::readersWaiting = 0;
stack<char*> Connection::memoryPool;
stack<int> Connection::memorySizes;

// Member functions

//---------------------------------------------------------------

Connection::Connection()
{
  //socket = -1;
  name = NULL;
  //inputFormat = outputFormat = 0;
  pendingData = NULL;
  pendingSize = pendingLimit = 0;
  allset = NULL;

  nElements = sizeElements = -1;
}

//---------------------------------------------------------------

Connection::~Connection()
{
  if( pendingLimit )
    delete [] pendingData;
}

//---------------------------------------------------------------

void Connection::setAllset( fd_set *newAllset )
{
  allset = newAllset;
}

//---------------------------------------------------------------

void Connection::setMaxDescriptor( int *newMaxDescriptor )
{
  maxDescriptor = newMaxDescriptor;
}

//---------------------------------------------------------------

int Connection::setInputFormat( int newFormat )
{
/*
  if( newFormat == 0 || newFormat == 1 || newFormat == 2 )
  {
    inputFormat = newFormat;
    return inputFormat;
  }
  else
*/
    return -1;
}

//---------------------------------------------------------------

int Connection::setOutputFormat( int newFormat )
{
/*
  if( newFormat == 0 || newFormat == 1 || newFormat == 2 )
  {
    outputFormat = newFormat;
    return outputFormat;
  }
  else
*/
    return -1;
}

//---------------------------------------------------------------

int Connection::checkName( char *compareName )
{
  if( !name || !compareName ) //check for NULLs
    return -1;

  if( strcmp( name, compareName ) == 0 )
    return 1;
  else 
    return 0;
}

//---------------------------------------------------------------

void Connection::setName( char *newName )
{
  if( !newName || strlen( newName ) < 1 )
    return;

  if( name ) //already allocated, need to deallocate
  {
    delete [] name;
    name = NULL;
  }

  name = new char[ strlen( newName )+1 ];
  strcpy( name, newName );
}

//----------------------------------------------------------------

int Connection::giveReaderHeaderInfo( int sd )
{
  int data[2], nwritten;

  //do I need to calculate nelements?
  if( nElements == -1 && !binaryData.empty() )
  {
    sizeElements = 4;
    nElements = binarySize.front();
    nElements /= 4;
  }

  //convert to net standard
  data[0] = htonl( nElements );
  data[1] = htonl( sizeElements );

  if( (nwritten = write( sd, (char*) data, sizeof( int ) * 2 ) ) < 0 )
  {
    if( verbose ) fprintf( stderr, "%s: broken pipe - remove reader\n", name );

    //caught broken pipe signal?
    FD_CLR( sd, allset );
    close( sd );
    return -1;
  }

  return 0;
}

//----------------------------------------------------------------

int Connection::getWriterHeaderInfo( int sd )
{
  int data[2], nread;

  if( (nread = read( sd, (char *) data, sizeof(int) * 2 ) ) < 0 )
  {
    //error
    return -1;
  }
  else if( nread = 0 )
  {
    //disconnect
    FD_CLR( sd, allset );
    close( sd );
    return -1;
  }
  else
  {
    nElements = ntohl( data[0] );
    sizeElements = ntohl( data[1] );
  }

  return 0;
}

//----------------------------------------------------------------

int Connection::addReader( int sd, int format, int c_verbose )
{
  if( verbose || c_verbose ) fprintf( stderr, "%s: Adding new reader\n", name );

  Client current;
  current.socket = sd;
  current.format = format;
  current.verbose = c_verbose;

  FD_SET( sd, allset );
  if( sd > *maxDescriptor )
    *maxDescriptor = sd;

  current.needsInfo = 0;
  if( format == 2 ) //binary readers need a special 2-byte header
  {
    current.needsInfo = 1;
    
    if( nElements == -1 && !binaryData.empty() ) //writer has connected, is not a constant binary, but has data
    {
      //try to calculate const sizes using front data
      sizeElements = 4;
      nElements = binarySize.front();
      nElements /= 4;
      if( (giveReaderHeaderInfo( current.socket )<0) )
        return -1;  //client disconnected, function closed socket for me
      current.needsInfo = 0;
    }
    else if( nElements > 0 ) //writer has connected and already provided this info
    {
      if( (giveReaderHeaderInfo( current.socket )<0) )
        return -1;  //client disconnected, function closed socket for me
      current.needsInfo = 0;  //has info now
    }
  }

  //also, if there is already data available on this queue, turn on flag
  if( !asciiData.empty() )
    readersWaiting = 1;

  readerDescriptors.push_back( current );
}

//----------------------------------------------------------------

int Connection::addWriter( int sd, int format, int c_verbose )
{
  if( verbose ) fprintf( stderr, "%s: Adding new writer\n", name );

  Client current, empty;
  current.socket = sd;
  current.format = format;
  current.verbose = c_verbose;

  FD_SET( sd, allset );
  if( sd > *maxDescriptor )
    *maxDescriptor = sd;

  if( format == 2 ) //constant binary writers will send a special 2 byte header
  {
    if( (getWriterHeaderInfo(sd) < 0) )
      return -1;  //disconnect
    //are any readers waiting for constant binary header info?
    for( list<Client>::iterator client=readerDescriptors.begin(); client != readerDescriptors.end(); client++ )
    {
      if( client->needsInfo )
      {
        if( ( client->needsInfo = giveReaderHeaderInfo(client->socket)) < 0 )
          client->clear();
      }
    }
    //remove any bad readers
    empty.clear();
    readerDescriptors.remove( empty );
  }

  writerDescriptors.push_back( current );
}

//----------------------------------------------------------------

void Connection::process( fd_set &active )
{
  processWriters( active );
  processReaders( active );  //should I do writers before readers?
}

//----------------------------------------------------------------

void Connection::processReaders( fd_set &active )
{
  int dataSent = 0;
  char *data = NULL;
  int size = 0;
  int nwritten;

  if( !allset )  //make sure this pointer has been set
    return;

  //for each reader
  for( list<Client>::iterator client = readerDescriptors.begin(); client != readerDescriptors.end(); client++ )
  {
    //if a reader has activity on it's descriptor, that means it is trying to disconnect
    if( FD_ISSET( client->socket, &active ) )
    {
      if( verbose ) fprintf( stderr, "%s: flagging reader for removal\n", name );

      //remove this client from main set
      FD_CLR( client->socket, allset );
      close( client->socket );               //close socket

      //use -1 to indicate this element needs to be removed
      client->socket = client->verbose = client->format = -1;
    }
    else
    {
      //send it the next bit of data -> I can check either ascii or binary queues since both will be filled
      if( !asciiData.empty() )
      {
        //each reader chooses which format it will receive data in
        //just take it from appropriate queue
        if( client->format == 0 ) //send from ascii list
        {
          data = asciiData.front();
          size = asciiSize.front();
        }
        else if( client->format == 1 || client->format == 2 ) //send from binary - no sizes
        {
          if( client->needsInfo && 
              (client->needsInfo = giveReaderHeaderInfo( client->socket )) < 0 )
          {
            client->clear();
            continue;
          } 
            
          data = (char *) binaryData.front();
          size = binarySize.front();
        }

        if( (nwritten = write( client->socket, data, size ) ) < 0 )
        {
          if( verbose ) fprintf( stderr, "%s: broken pipe - remove reader\n", name );

          //caught broken pipe signal?
          FD_CLR( client->socket, allset );
          close( client->socket );
          client->socket = client->verbose = client->format = -1;
        }
        else
          dataSent = 1;
      }
    }
  }

  //pop off front off data lists if any data was sent
  if( dataSent )
  {
    if( verbose ) fprintf( stderr, "%s: sent data: %s\n", name, asciiData.front() );

    releaseMemory( asciiData.front(), asciiMemory.front() );
      asciiData.front() = NULL;
    releaseMemory( binaryData.front(), binaryMemory.front() );
      binaryData.front() = NULL;
    asciiData.pop();
    binaryData.pop();
    asciiSize.pop();
    binarySize.pop();
    asciiMemory.pop();
    binaryMemory.pop();
  }

  //purge -1's
  Client target;
  target.socket = target.verbose = target.format = -1;
  readerDescriptors.remove( target );

  //if there are still some readers and the dataQueue is not empty, set flag
  if( !readerDescriptors.empty() && !asciiData.empty() )
    readersWaiting = 1;
}

//------------------------------------------------------------------------------

void Connection::processWriters( fd_set &active )
{
  if( !allset )
    return;

  int nread = 0;
  
  //only the first writer is listened to, all others will be ignored
  const int Limit = 1024;
  char buffer[ Limit ] = {};  //clear

  list<Client>::iterator client = writerDescriptors.begin();
  if( client == writerDescriptors.end() )
    return;

  if( !FD_ISSET( client->socket, &active ) )
    return;

  if( (nread = read( client->socket, buffer, Limit ) ) < 0 )
  {
    //error
    if( verbose ) fprintf( stderr, "%s: Error on read\n", name );
  }
  else if( nread == 0 )
  {
    if( verbose ) fprintf( stderr, "%s: remove writer\n", name );

    FD_CLR( client->socket, allset );
    close( client->socket );
    client->socket = client->format = client->verbose = -1;

    //should I ignore any pending data left over from this writer?
    pendingSize = 0;
  }
  else
  {
    //parse data and store it for the readers
    // - this will parse as much as possible, not just once
    append( buffer, nread );
    parse();
  }

  //purge -1's
  //writerDescriptors.remove( -1 );
  Client target;
  target.socket = target.verbose = target.format = -1;
  writerDescriptors.remove( target );

  //did I add data to my queue? Do I have readers?
  if( !asciiData.empty() && !readerDescriptors.empty() )
    readersWaiting = 1;
}

//-----------------------------------------------------------------------------

int Connection::append( char *buffer, int nread )
{
  char *helper;

    //allocate new memory to hold everything if necessary
    if( pendingLimit < nread + pendingSize )
    {
      helper = new char [ pendingSize + nread ];

      //copy old data over - if any
      if( pendingSize )
        memcpy( helper, pendingData, pendingSize );

      if( pendingLimit ) //need to free alocated memory
      {
        delete [] pendingData;
        pendingData = NULL;
      }

      //patch pointers
      pendingData = helper;
      pendingLimit = pendingSize + nread;
    }

    //append new data to end
    memcpy( &pendingData[pendingSize], buffer, nread );

    pendingSize += nread;
}

//-----------------------------------------------------------------------------

int Connection::parse()
{
  int done = 0, successfulParsings = 0;
  int stringPosition, count, found;
  static char *shiftBuffer;
  static int shiftSize;

  if( !pendingSize ) //nothing to parse
    return 0;

  //assume that what I want to see is the only thing on pendingData

  while ( !done )
  {
    found = 0;
    if( pendingSize )
      count = atoi( pendingData ); //assume first chunk of data can form a value

    //where did this number end?
    for( stringPosition = 0; stringPosition < pendingSize && !found; stringPosition++ )
    {   //look for white space: "\n\t\0 "
      if( pendingData[stringPosition] == ' ' || pendingData[stringPosition] == '\n' ||
          pendingData[stringPosition] == '\t' || pendingData[stringPosition] == 0 )
        found = 1;  //found end of number
    }

    if( !found ) //can't parse any more data
      done = 1;
    else
    {
      //make sure there is at least count bytes left on string
      if( pendingSize - stringPosition < count ) //can't parse it
        done = 1;
      else
      {
        int memorySize = 0;
        char *dataChunk = acquireMemory( count+1, memorySize );
        memcpy( dataChunk, &pendingData[stringPosition], count );

        queueData( dataChunk, count, memorySize );

        if( pendingSize-count-stringPosition ) //data remains
        {
          if( shiftSize < pendingSize-count-stringPosition ) //need more shift space
          {
            delete [] shiftBuffer;
            shiftBuffer = NULL;
            shiftSize = pendingSize-count-stringPosition;
            shiftBuffer = new char[shiftSize];
          }

          pendingSize = pendingSize-count-stringPosition;
          memcpy( shiftBuffer, &pendingData[count+stringPosition], pendingSize );
          memcpy( pendingData, shiftBuffer, pendingSize );
        }
        else
        {
          pendingSize = 0;
          done = 1;
        }

        successfulParsings++;

      }
    }
  }
  return successfulParsings;
}

//----------------------------------------------------------------------

//when multiple readers demand multiple formats, this must be changed
void Connection::queueData( char *buffer, int count, int memorySize )
{
  //what format is the writer using?
  if( (writerDescriptors.begin())->format == 0 ) //inputFormat == 0 ) //ascii
  {
    buffer[count] = 0; //null terminate - space was given to fit the null terminator
    asciiData.push( buffer );
    asciiSize.push( count );
    asciiMemory.push( memorySize );
    asciiToBinary( buffer, count );  //should I bother with conversions if it's not needed?
  }
  else //binary and const binary
  {
    //testing
    binaryData.push( buffer );
    binarySize.push( count );
    binaryMemory.push( memorySize );
    binaryToAscii( buffer, count );  //should I bother with conversions if it's not needed?
  }

  if( verbose ) fprintf( stderr, "%s: queued new data\n", name );
}

//------------------------------------------------------------------------

void Connection::asciiToBinary( char *buffer, int count )
{
  vector<float> helper;
  int inFloat;
  int index, vIndex;
  int lastValue;

  //use atof to grab numbers off the string and put them onto a new string
  //go through string, finding numerical separators, and use atof to make them floats

  char valid[] = "+-eE."; //other valid characters besides 0-9

  //go through string until we reach the first valid number character
  lastValue = 0;
  for( index=0; index<count; index++ )
  {
    inFloat = 0;
    if( buffer[index] >= '0' && buffer[index] <= '9')
      inFloat = 1;
    for( vIndex=0; valid[vIndex] && !inFloat; vIndex++ )
    {
      if( !inFloat && (buffer[index] == valid[vIndex]) )
        inFloat = 1;
    }

    if( lastValue != 1 && inFloat == 1 ) //we were not in a float, but now are
    {
      helper.push_back( atof( &buffer[index] ) );
    }

    lastValue = inFloat;
  }

  //copy floats into a simple string and add them to the binary queues
  int finalSize = sizeof(float) * helper.size();
  int memorySize = 0;
  char *result = acquireMemory( finalSize, memorySize );
  memcpy( result, &helper[0], finalSize );

  binaryData.push( result );
  binarySize.push( finalSize );
  binaryMemory.push( memorySize );
}

//--------------------------------------------------------------------------

void Connection::binaryToAscii( char *buffer, int count )
{
  vector<char *> dataSnacks;
  int totalLength = 0;

  //hopefully, 32 is big enough for one number
  char *current;

  //assume data is in float format
  //size should be in multiple of 4 = sizeof(float)
  int floatCount = count/sizeof(float);

  //determine ultimate size of this string
  for( int i=0; i<floatCount; i++ )
  {
    current = new char[32];
    sprintf( current, "%4.4f", ((float*)buffer)[i] );//helper[i] );
    dataSnacks.push_back( current );
    totalLength += strlen( current ) + 1; //need a space between, and a final newline
  }

  int memorySize = 0;
  char *final = acquireMemory( totalLength+1, memorySize );  //one more for a null terminator
  final[0] = final[totalLength] = 0;
  for( unsigned int i=0; i<dataSnacks.size(); i++ )
  {
    strcat( final, dataSnacks[i] );
    strcat( final, " " );
    delete [] dataSnacks[i];
  }
  final[totalLength-1] = '\n';

  asciiData.push( final );
  asciiSize.push( totalLength );
  asciiMemory.push( memorySize );

  dataSnacks.clear();
}

//------------------------------------------------------------------

void Connection::reset()
{
  Client empty;
  empty.socket = empty.verbose = empty.format = -1;

  //remove all writers and readers from their lists,
  //and clear their fd bits
  for( list<Client>::iterator client=writerDescriptors.begin(); client != writerDescriptors.end(); client++ )
  {
    FD_CLR( client->socket, allset );

    //close the socket
    close( client->socket );

    //use -1 to indicate this element needs to be removed
    client->socket = client->verbose = client->format = -1;
  }
  writerDescriptors.remove( empty );
  
  for( list<Client>::iterator client=readerDescriptors.begin(); client != readerDescriptors.end(); client++ )
  {
    FD_CLR( client->socket, allset );

    //close the socket
    close( client->socket );

    //use -1 to indicate this element needs to be removed
    client->socket = client->verbose = client->format = -1;
  }
  readerDescriptors.remove( empty );

  //clear all data queues
  while( !asciiData.empty() )
  {
    //delete [] asciiData.front();
    //delete [] binaryData.front();
    releaseMemory( asciiData.front(), asciiMemory.front() );
    releaseMemory( binaryData.front(), binaryMemory.front() );

    asciiData.pop();
    asciiSize.pop();
    binaryData.pop();
    binarySize.pop();
    asciiMemory.pop();
    binaryMemory.pop();
  }
}

//------------------------------------------------------------------

void Connection::setPendingReads( int flag )
{
  readersWaiting = flag;
}

//------------------------------------------------------------------

//------------------------------------------------------------------

int Connection::pendingReadsAndClear()
{
  int value = readersWaiting;
  readersWaiting = 0;
  return value;
}

int Connection::pendingReads()
{
  return readersWaiting;
}

//------------------------------------------------------------------

int Connection::empty()
{
  return( readerDescriptors.empty() && writerDescriptors.empty() );
}

//------------------------------------------------------------------

void Connection::setVerbose( int flag )
{
  if( flag == 2 )
    verbose = !verbose;
  else
    verbose = flag;
}

//------------------------------------------------------------------

char* Connection::peek()
{
  if( asciiData.empty() )
    return NULL;
  return asciiData.front();
}

//------------------------------------------------------------------

char* Connection::acquireMemory( int requestedSize, int &memoryAllocated )
{
    char *holder = NULL;

    //check if stack is empty
    if ( memoryPool.empty() )
    {
        //just allocate the memory directly
        holder = new char[requestedSize];
        if( !holder )
            cerr<<"Error allocating memory\n";

        memoryAllocated = requestedSize;
    }
    else
    {
        //remove the first element, resize if necesary
        holder = memoryPool.top();
            memoryPool.pop();
        memoryAllocated = memorySizes.top();
            memorySizes.pop();

        if( memoryAllocated < requestedSize )
        {
            delete [] holder;
            holder = new char[requestedSize];
            if( !holder )
                cerr<<"Error reallocating memory\n";

            memoryAllocated = requestedSize;
        }
    }

    return holder;
}

//------------------------------------------------------------------

void Connection::releaseMemory( char *memory, int memoryAllocated )
{
    memoryPool.push( memory );
    memorySizes.push( memoryAllocated );
}


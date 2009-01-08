/*
 *  servertype.cpp
 *  Thesis
 *
 *  Created by James King on Thu Mar 04 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
#include "servertype.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

Server::Server()
{
    name = NULL;
    remainingData = NULL;
    remainingLength = 0;
    verbose = false;
}

//------------------------------------------------------------------------------------------

void Server::append( int &amount, char *&buffer )
{
	char *helper;
	
	if( remainingData )
	{
		//allocate new memory to hold everything
		helper = new char [remainingLength+amount];
		//copy old data over
		memcpy( helper, remainingData, remainingLength );
		
		//patch pointers
		delete [] remainingData;
		remainingData = helper;
		
		//append new data to end
		memcpy( &remainingData[remainingLength], buffer, amount );
		remainingLength += amount;
	}
	else
	{
		//allocate memory
		remainingData = new char[amount];
		//copy over
		memcpy( remainingData, buffer, amount );
		remainingLength = amount;
	}
}

//------------------------------------------------------------------------------------------

int Server::parse()
{
    //look at first value of string - if it is an integer, number and there are that
    //many bytes remaining on the string - extract them as data

    int done = 0;
    int successfulParsings = 0;
    int count;
    int stringPosition;
    int found;
	
    if( !remainingLength )
        return 0;
	
    while ( !done )
    {
        found = 0;
        if( remainingLength )
            count = atoi( remainingData );
        
	//where did this number end?
	for( stringPosition = 0; stringPosition < remainingLength && !found; stringPosition++ )
	{   //look for "\n\t\0 "
		if( remainingData[stringPosition] == ' ' || remainingData[stringPosition] == '\n' ||
			remainingData[stringPosition] == '\t' || remainingData[stringPosition] == 0 )
			found = 1;
	}
		
	if( !found ) //can't parse any more data
		done = 1;
	else
	{
		//make sure there is at least count bytes left on string
		if( remainingLength - stringPosition < count ) //can't parse it
		  done = 1;
		else
		{
			char *dataChunk = new char[count+1];
			memcpy( dataChunk, &remainingData[stringPosition], count );
			dataChunk[count] = 0; //null terminate
			dataQueue.push( dataChunk );
			dataSize.push( count );
				
                	if( verbose ) fprintf( stderr, "[%d] extracted info: %s\n", port, dataQueue.back() );
				
                	if( remainingLength-count-stringPosition )
                	{
                    		dataChunk = new char[remainingLength-count-stringPosition];
                    		memcpy( dataChunk, &remainingData[count+stringPosition], remainingLength-count-stringPosition );
                    		delete [] remainingData;
                    		remainingData = dataChunk;
                    		remainingLength = remainingLength-count-stringPosition;
                	}
                	else
                	{
                    		remainingData = NULL;
                    		remainingLength = 0;
                    		done = 1;
                	}
                
                	successfulParsings++;
				
		}
	}
    }
	
    return successfulParsings;
}

void Server::setVerbose( int mode )
{
    if( mode == 1 )
      verbose = true;
    else if( mode == 0 )
      verbose = false;
    else
      verbose = !verbose;
}

void Server::reset()
{
    //clear queue
    while( !dataQueue.empty() )
    {
        delete [] dataQueue.front();
        dataQueue.pop();
        dataSize.pop();
    }
    
    //close all connected clients
    for( list<int>::iterator it=clients.begin(); it != clients.end(); it++ )
        close( *it );
    clients.clear();
    
    //if there is any pending data, delete it
    if( remainingData )
    {
        delete [] remainingData;
        remainingLength = 0;
    }
    
}

void Server::setName( char *newName )
{
  if( name ) //already allocated?
    delete [] name;
  name = new char[ strlen( newName ) + 1 ];
  strcpy( name, newName );
}

int Server::getPort( char *targetName )
{
  if( !name ) //name is NULL
    return -1;
  else if( strcmp( name, targetName ) == 0 )
    return port;
  else
    return -1;
}


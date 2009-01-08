//Jim King
//storage.cpp
//stimulus storage for singlestream class, perhaps can be used by connection class

#include "storage.h"

//---------------------------------------------------------------------------------
// Static Data Members
//---------------------------------------------------------------------------------

char* Storage::shiftBuffer = NULL;
int Storage::shiftSize = 0;
stack<char*> Storage::memoryPool;
stack<int> Storage::memorySizes;

//---------------------------------------------------------------------------------
// Constructors & Destructors
//---------------------------------------------------------------------------------

Storage::Storage( char *Name, int Key )
{
    //Init most data
    name = null;
    key = Key;
    pendingData = NULL;
    pendingSize = pendingLimit = 0;

    //Yes, This should be remembered
    nElements = sizeElements = -1;

    //Do I require a name?
    if( !Name )
        return;
        
    name = new char[ strlen(Name)+1 ];
    strcpy( name, Name );
}

//---------------------------------------------------------------------------------

Storage::~Storage()
{
    //free memory used
    if( name )
        delete [] name;
    
    if( pendingData )
        delete [] pendingData;
        
    
}

//---------------------------------------------------------------------------------
//Accessor functions
//---------------------------------------------------------------------------------

int Storage::getKey()
{
    return key;
}

//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
//Data Management functions
//---------------------------------------------------------------------------------

int Connection::append( char *buffer, int nread )
{
    char *helper;

    //allocate new memory to hold everything if necessary
    if( pendingLimit < nread + pendingSize )
    {
        helper = new char [ pendingSize + nread ];
        memUsed += pendingSize + nread;

        //copy old data over - if any
        if( pendingSize )
            memcpy( helper, pendingData, pendingSize );

        if( pendingLimit ) //need to free alocated memory
        {
            delete [] pendingData;
            memUsed -= pendingSize;
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
        {   //look for "\n\t\0 "
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
                char *dataChunk = new char[count+1];
                memUsed += count+1;
                memcpy( dataChunk, &pendingData[stringPosition], count );
                //dataChunk[count] = 0; //null terminate - let queueData handle, if necessary

                queueData( dataChunk, count );

                if( pendingSize-count-stringPosition ) //data remains
                {
                    if( shiftSize < pendingSize-count-stringPosition ) //need more shift space
                    {
                        delete [] shiftBuffer;
                        memUsed -= shiftSize;
                        shiftBuffer = NULL;
                        shiftSize = pendingSize-count-stringPosition;
                        shiftBuffer = new char[shiftSize];
                        memUsed += shiftSize;
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

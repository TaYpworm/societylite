#include "CircleList.h"

CircleList::CircleList()
{
  maxSize = 10;
  start = -1;
  clist = (CmpVoltage*) malloc( sizeof(CmpVoltage)*maxSize );
  used = 0;
}

CircleList::CircleList( int size )
{
  if( size < 1 )
    size = 1;
  maxSize = size;
  start = -1;
  //clist = new CmpVoltage[ size ];
  clist = (CmpVoltage*) malloc( sizeof(CmpVoltage)*maxSize );
  used = 0;
}

CircleList::~CircleList()
{
  //delete [] clist;
  free( clist );
}

unsigned int CircleList::capacity()
{
  return maxSize;
}

unsigned int CircleList::size()
{
  return used;
}

void CircleList::push_back( CmpVoltage data )
{
  //this function is called push_back to indicate that data is added
  // to the end of the buffer.  data must be shifted within the vector
  // to maintain proper order; thus, this operation is slower that
  // push_front.  The start will only move if the circular list has become full
  // and the new data will overwrite the start of the list.  start will then
  // move to the next element
}

void CircleList::push_front( CmpVoltage data )
{
  //this function is called push_front to indicate that the start will move
  // to point at the newly added data

  if( start == -1 )  //empty list
  {
    start = 0;                 //point to first element
    clist[start] = data;
    used++;
  }
  else if( used < maxSize )  //space available
  {
    used++;
    start++;
    clist[start] = data;
  }
  else  //no more space, move start then overwrite
  {
    start++;
    while( start >= maxSize )
        start -= maxSize;

    clist[start] = data;
  }
}

CmpVoltage CircleList::at( int offset )
{
  int location = offset + start;
  CmpVoltage dummy;

  if( used == 0 )  //empty list - return
    return dummy;

  //wrap around
  while ( location < 0 )
    location += used;
  while ( location >= used )
    location -= used;

  return clist[location];
}

int CircleList::resize( int maxsize )
{
  //should this clear the list?
  //it will only be resized upon compartment creation, so no data should exist yet.

  if( maxsize == maxSize )
    return maxsize;

  CmpVoltage *replacement = new CmpVoltage[ maxsize ];

  //copy from start and move down the list
  long location = start;
  for( int i=0; i<maxsize && i<maxSize && i<used; i++ )
  {
    replacement[maxsize-i-1] = clist[location];
    location--;
    if( location < 0 )
      location+=used;
  }

  if (used > maxsize )
    used = maxsize;
  maxSize = maxsize;

  delete [] clist;
  clist = replacement;
  replacement = NULL;

  return maxsize;
}

int CircleList::Save( FILE *out )
{
  int sum = 0;

  fwrite( this, sizeof( CircleList ), 1, out );

  //CmpVoltage is a simple struct (one variable)
  fwrite( clist, sizeof( CmpVoltage ), maxSize, out );

  sum += sizeof( CircleList ) + sizeof( CmpVoltage ) * maxSize;

  return sum;
}

void CircleList::Load( FILE *in, char *nothing )
{
  fread( this, sizeof( CircleList ), 1, in );
  clist = NULL;
  clist = (CmpVoltage*) malloc( sizeof(CmpVoltage)*maxSize );

  fread( clist, sizeof( CmpVoltage ), maxSize, in );
}


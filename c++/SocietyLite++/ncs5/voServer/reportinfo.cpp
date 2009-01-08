//Jim King
//reportinfo.cpp
//implement the ReportInfo class

#include "reportinfo.h"

#include <string.h>
#include <iostream>
#include <vector>

#ifdef QQ_ENABLE
  #include "QQ.h"
#endif

#include <errno.h>

using namespace std;

int ReportInfo::filesOpen = 0;
int ReportInfo::repCount = 0;
int ReportInfo::maxRep = 0;

ReportInfo::ReportInfo( char *file )
{
  maxRep++;
  repID = maxRep;
  repCount++;

  fileName = new char[strlen(file)+1];

  startingByte = -1;
  lastLine = NULL;
  fin = NULL;

  //open file for reading
  strcpy( fileName, file );

/*
  fin = fopen( fileName, "r" );
  filesOpen++;

  if( !fin )
  {
    filesOpen--;
    delete [] fileName;
    fileName = NULL;
    return;
  }
*/

  if( readHeader() < 0 )
  {
    if( fin )
    {
      fclose( fin );
      filesOpen--;
    }
    delete [] fileName;
    fileName = NULL; fin = NULL;
    return;
  }

  //allocate space for storing the last line of data
  lastLine = new char[ nElements*sizeElements ];
}

//-------------------------------------------------------

int ReportInfo::openFile()
{
  if( !fin )
  {
    fin = fopen( fileName, "r" );
    filesOpen++;
  }
  if( !fin )
  {
    return -1;
  }
  repCount--;
  return 1;
}

//-------------------------------------------------------

int ReportInfo::closeFile()
{
  if( fin )
  {
    filesOpen--;
    fclose(fin);
  }
  fin = NULL;

  return 1;
}

//-------------------------------------------------------

void ReportInfo::peek()
{
  cout<<"files open: "<<filesOpen<<endl;
}

//-------------------------------------------------------

ReportInfo::~ReportInfo()
{
  if( fileName )
  {
    delete [] fileName;
  }
  if( lastLine )
  {
    delete [] lastLine;
  }
  if( fin )
  {
    fclose( fin );
    filesOpen--;
   }

  fileName = NULL;
  lastLine = NULL;
  fin = NULL;
}

//-------------------------------------------------------

int ReportInfo::sendHeader( FILE *fout )
{
  //send constant length fields

  //Report Name, col, lay, cell, cmp, report on
  fwrite( reportType, 32, 1, fout );
  fwrite( column, 32, 1, fout );
  fwrite( layer, 32, 1, fout );
  fwrite( cell, 32, 1, fout );
  fwrite( cmp, 32, 1, fout );
  fwrite( reportOn, 32, 1, fout );

}

//-------------------------------------------------------

int ReportInfo::sendLine( FILE *fout )
{
  fwrite( lastLine, nElements, sizeElements, fout );
}

//-------------------------------------------------------

int ReportInfo::getCount()
{
  return nElements;
}

//-------------------------------------------------------

int ReportInfo::readHeader()
{
  char buffer[128];
  int *ids;

  if( openFile() < 0 )
  {
    closeFile();
    return -1;
  }

  //read file header
  //magic number (4 chars) and version (4 chars)
  fread( buffer, 8, 1, fin );
  buffer[8] = 0;

  if( strcmp( buffer, "NCS 1.00" ) != 0 )
  {
    closeFile();
    return -1;
  }

  //begin reading the rest of the header
  fread( buffer, 1, 1, fin ); //dummy -> newline

  //nElements ands sizeElements
  fread( &nElements, sizeof(int), 1, fin );
  fread( &sizeElements, sizeof(int), 1, fin );

  //Sim Title, Report Type, Report On, Col, Lay, Cell, Cmp
  fread( brainType, sizeof(char), 127, fin );
  fread( reportType, sizeof(char), 32, fin );
  fread( reportOn, sizeof(char), 32, fin );
  fread( column, sizeof(char), 32, fin );
  fread( layer, sizeof(char), 32, fin );
  fread( cell, sizeof(char), 32, fin );
  fread( cmp, sizeof(char), 32, fin );

  //remove excess whitespace from the names - spaces, newlines
  for( int i=126; i>0 && (brainType[i] == '\n' || brainType[i] == ' '); i-- )
    brainType[i] = 0;
  for( int i=31; i>0 && (reportType[i] == '\n' || reportType[i] == ' '); i-- )
    reportType[i] = 0;
  for( int i=31; i>0 && (reportOn[i] == '\n' || reportOn[i] == ' '); i-- )
    reportOn[i] = 0;
  for( int i=31; i>0 && (column[i] == '\n' || column[i] == ' '); i-- )
    column[i] = 0;
  for( int i=31; i>0 && (layer[i] == '\n' || layer[i] == ' '); i-- )
    layer[i] = 0;
  for( int i=31; i>0 && (cell[i] == '\n' || cell[i] == ' '); i-- )
    cell[i] = 0;
  //cmp is more complicated, but it is not important for now.
  for( int i=31; i>0 && (cmp[i] == '\n' || cmp[i] == ' '); i-- )
    cmp[i] = 0;

  //ThingIDs - can be discarded?
  //NCS does not correctly number the items anymore
  ids = new int[nElements];
  fread( ids, sizeof(int), nElements, fin );

  delete [] ids;

  //remember the location - this is where the header ends.
  startingByte = ftell(fin);

  checkAvailable();

  //assign descriptors pointers to have shortcuts ready
  descriptor[0] = reportType;
  descriptor[1] = reportOn;
  descriptor[2] = column;
  descriptor[3] = layer;
  descriptor[4] = cell;
  descriptor[5] = cmp;

  //closeFile();
  return 1;
}

//-------------------------------------------------------------

int ReportInfo::detectNCSbinary()
{
  if( startingByte > 0 )
    return 1;
  return 0;
}

//-------------------------------------------------------------

int ReportInfo::readLine()
{
  //open file
//jim
  //openFile();

  //check to see that I can read the line?
  //should I keep track of my current timestep?
  if( currentTimeStep >= maxAvailable )
    checkAvailable();

  if( currentTimeStep >= maxAvailable )
    return 0;

  fread( lastLine, sizeElements, nElements, fin );
//cerr<<currentTimeStep<<" aquired: "<<((float*)lastLine)[0]<<endl;

  //close file
//jim
  //closeFile();

  currentTimeStep++;
  return 1;
}

//-------------------------------------------------------------

int ReportInfo::moveToTimestep( int timestep )
{
  //if( OpenFile()

  //check to make sure I can move to the requested time
  if( timestep >= maxAvailable )
    checkAvailable(); //have I added time since last I checked

  if( timestep >= maxAvailable )  //don't move file pointer
    return 0;
 
  int byte = timestep*nElements*sizeElements + startingByte;

  fseek( fin, byte, SEEK_SET );
  currentTimeStep = timestep;

  return 1;
}

//-------------------------------------------------------------

int ReportInfo::checkCriteria( char* criteria )
{
  //how is the criteria given to me?
  //well, I guess I can accept the same fields I sent out
  // in the header
  // Report Name (ANY), Col (ANY), Lay (ANY), Cell (ANY), Cmp (ANY), Report On (ANY )
  //also using fixed length fields

  //assume valid
  if( !( strncmp( criteria, "*", 1 ) == 0 || 
         strncmp( criteria, reportType, 32 ) == 0 ) )
    return 0;
  if( !( strncmp( criteria+32, "*", 1 ) == 0 ||
         strncmp( criteria+32, column, 32 ) == 0 ) )
    return 0;
  if( !( strncmp( criteria+64, "*", 1 ) == 0 ||
         strncmp( criteria+64, layer, 32 ) == 0 ) )
    return 0;
  if( !( strncmp( criteria+96, "*", 1 ) == 0 ||
         strncmp( criteria+96, cell, 32 ) == 0 ) )
    return 0;
  if( !( strncmp( criteria+128, "*", 1 ) == 0 ||
         strncmp( criteria+128, cmp, 32 ) == 0 ) )
    return 0;
  if( !( strncmp( criteria+160, "*", 1 ) == 0 ||
         strncmp( criteria+160, reportOn, 32 ) == 0 ) )
    return 0;

  //if I've reached here, this report must be valid
  return 1;
}

//----------------------------------------------------------

int ReportInfo::variedLengthCheckCriteria( char* criteria )
{
  //tokenize criteria
  char front[1024], *helper;
  vector<char*> fields;

  //if I allow null terminators to separate fields,
  //how does strtok work? it can't
  strcpy( front, criteria );
  helper = strtok( front, " \t\0\n" );
  while( helper != NULL )
  {
    fields.push_back( helper );
    helper = strtok( NULL, " \t\0\n" );
  }

  if( fields.size() < 6 )
    return 0;

  if( !( strncmp( fields[0], "*", 1 ) == 0 ||
         strncmp( fields[0], reportType, 32 ) == 0 ) )
    return 0;
  if( !( strncmp( fields[1], "*", 1 ) == 0 ||
         strncmp( fields[1], reportOn, 32 ) == 0 ) )
    return 0;
  if( !( strncmp( fields[2], "*", 1 ) == 0 ||
         strncmp( fields[2], column, 32 ) == 0 ) )
    return 0;
  if( !( strncmp( fields[3], "*", 1 ) == 0 ||
         strncmp( fields[3], layer, 32 ) == 0 ) )
    return 0;
  if( !( strncmp( fields[4], "*", 1 ) == 0 ||
         strncmp( fields[4], cell, 32 ) == 0 ) )
    return 0;
  if( !( strncmp( fields[5], "*", 1 ) == 0 ||
         strncmp( fields[5], cmp, 32 ) == 0 ) )
    return 0;

  //if I've reached here, this report must be valid
  return 1;

}

//----------------------------------------------------------

int ReportInfo::lessThan( const ReportInfo *RHS, const int order[] ) const
{
  int less=0;

  //start at the primary sort column, and work my way up
  for( int i=0; i<6; i++ )
  {
    //make sure order is a valid integer
    if( order[i] < 0 || order[i] > 5 )
      return 0;

    less = strcasecmp( descriptor[order[i]], RHS->descriptor[order[i]] );

    if( less < 0 )
      return 1;
    else if( less > 0 )
      return 0;
  }

  //must be exact same on all fields
  return 0;
}

//-----------------------------------------------------------

int ReportInfo::greaterThan( const ReportInfo *RHS, const int order[] ) const
{
  int great=0;

  //start at the primary sort column, and work my way up
  for( int i=0; i<6; i++ )
  {
    //make sure order is a valid integer
    if( order[i] < 0 || order[i] > 5 )
      return 0;

    great = strcasecmp( descriptor[order[i]], RHS->descriptor[order[i]] );

    if( great > 0 )
      return 1;
    else if( great < 0 )
      return 0;
  }

  //must be exact same on all fields
  return 0;
}

//-----------------------------------------------------------

int ReportInfo::checkAvailable()
{
  //determine how many total timesteps are avialable to this
  //report by seeking from the end of the header (startingByte) to the end of 
  //the file
  int currentLocation = ftell( fin );

  fseek( fin, 0, SEEK_END );

  int bytes = ftell(fin)-startingByte;

  int numTimeSteps = (int) bytes/(nElements*sizeElements);

  //return to where I was
  fseek( fin, currentLocation, SEEK_SET );

  maxAvailable = numTimeSteps;

  return numTimeSteps; //I should save this somewhere - probably
    //because recalculating it again and again might be expensive
    //it might be easier to total once, then add on as more data is available
    //hovever, that creates more variables to store.
    //actually- I don't think saving this number would save any time
    //when I call fseek, It probably takes just as much time no matter
    //where I start from, and where I move.
}

//-----------------------------------------------------------


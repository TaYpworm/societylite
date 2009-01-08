//singlestream.cpp

#include <iostream>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "singlestream.h"

using namespace std;

fd_set* SingleStream::allset = NULL;
int* SingleStream::maxDescriptor = NULL;
vector<char*>* SingleStream::appList = NULL;
vector<char*>* SingleStream::scriptList = NULL;

char defaultPattern[MaxLength] = "* * * * * *\n";

extern int eventID[7];
extern int gatherTime;

vector<Storage*> SingleStream::writtenData;

//------------------------------------------------------------

int SingleStream::init( fd_set *pool, int *descriptors, vector<char*> *applist, vector<char*> *scriptlist )
{
  allset = pool;
  maxDescriptor = descriptors;
  appList = applist;
  scriptList = scriptlist;
}

//------------------------------------------------------------

SingleStream::SingleStream()
{
  fin = NULL;
  sd = -1;
  strcpy( job, "job" );
  getcwd( path, 1024 );
  sending = 0;
  verboseFlag = 0;

  for( int i=0; i<6; i++ )
    sortOrder[i] = i;
}

//-------------------------------------------------------------

SingleStream::SingleStream( int socket, int verbose )
{
  //open fin, but set it to a NULL buffer
  //because I still need to use sd in the 'select' functin call
  if( allset == NULL || maxDescriptor == NULL )
  {
    sd = -1;
    fin = NULL;
    return;
  }

  FD_SET( socket, allset );
  if( socket > *maxDescriptor )
    *maxDescriptor = socket;

  sd = socket;
  fin = fdopen( socket, "r+" );

  setbuf( fin, NULL );
  sending = 0;
  verboseFlag = verbose;

  for( int i=0; i<6; i++ )
    sortOrder[i] = i;
}

//-------------------------------------------------------------

int SingleStream::manageData()
{
  //grab mode (read/write)
  //grab format (ascii, binary)
  //grab name
  // does name exist?
  //  y-> perform task if possible
  //  n-> create entry and then execute task

  char buffer[MaxLength];
  int mode, format;

  if( verboseFlag )
    cout<<"manage data\n";

  //mode - read or write
  if( !fgets( buffer, MaxLength, fin ) )
    return 0;  //error reading

  if( verboseFlag )
    cout<<"received mode: "<<buffer;

  if( buffer[0] == 'r' || buffer[0] == 'R' ) mode = 0;
  else if( buffer[0] == 'w' || buffer[0] == 'W' ) mode = 1;
  else return -1;

  //format - ascii, binary, constant binary
  if( !fgets( buffer, MaxLength, fin ) )
    return 0;  //error reading

  if( verboseFlag )
    cout<<"received format: "<<buffer<<endl;

  if( buffer[0] == 'a' || buffer[0] == 'A' ) format = 0;
  else if( buffer[0] == 'b' || buffer[0] == 'B' ) format = 1;
  else if( buffer[0] == 'c' || buffer[0] == 'C' ) format = 2;
  else return -1;

  //name
  if( !fgets( buffer, MaxLength, fin ) )
    return 0;

  if( verboseFlag )
    cout<<"received name: "<<buffer;

  //try to find this name in storage vector
  for( int i=0; i<writtenData.size(); i++ )
  {

  }

  //handling writers:
    //read on my own, append( buffer ), parse
  return 1;
}

//-------------------------------------------------------------

/*
int SingleStream::disconnect()
{
  //This client has disconnected, free any resources
  // it controlled and close its socket

  close( sd );
  sd = -1;
  fin = NULL;
}
*/

//-------------------------------------------------------------

int SingleStream::process()
{
  int result = 0;
  char buffer[1024];

  if( sending )
  {
    transmit( subset, recall );
    return 0;
  }

  //check if this object is active
  if( !FD_ISSET( sd, allset ) )
    return 0;

  if( fgets( buffer, 1024, fin ) == NULL )
    return -1;

  if( verboseFlag )
    cout<<"Command: "<<buffer;

  if( strncmp( buffer, "collect", 7 ) == 0 ) //collect file names
  {
    result = listFiles();
  }
  else if( strncmp( buffer, "setpath", 7 ) == 0 ) //set directory and job
  {
    result = setPath();
  }
  else if( strncmp( buffer, "getdata", 7 ) == 0 )
  {
    result = getData();
  }
  else if( strncmp( buffer, "gettimecount", 12 ) == 0 )
  {
    result = getTimeCount();
  }
  else if( strncmp( buffer, "setpattern", 10 ) == 0 )
  {
    setPattern();
  }
  else if( strncmp( buffer, "launch", 6 ) == 0 )
  {
    result = launchNCS();
  }
  else if( strncmp( buffer, "setsort", 7 ) == 0 )
  {
    result = setSorting();
  }
  else if( strncmp( buffer, "getreportcount", 14 ) == 0 )
  {
    result = getReportCount();
  }
  else if( strncmp( buffer, "mkdir", 5 ) == 0 )
  {
    result = makeDirectory();
  }
  else if( strncmp( buffer, "exec", 4 ) == 0 )
  {
    result = invokeScript();
  }

  if( !sending )
    emptyReports();

  return result;
}

//--------------------------------------------------------

int SingleStream::listFiles()
{
  char dirName[MaxLength], jobName[MaxLength];
  DIR *currentDir;
  struct dirent *directoryContents;
  ReportInfo *currentReport;
  int stringLength;

  //make sure valid reports is cleared
  for( int i=0; i<validReports.size(); i++ )
    delete validReports[i];
  validReports.clear();

  //Should I change to the current directory?

  //look for file names with jobName in front
  if( (currentDir = opendir( path )) == NULL )
    return -1;
  while( (directoryContents=readdir( currentDir )) != NULL )
  {
    //send name to client -> should I send . and ..?
    //directoryContents->d_name
    //send strlen first - 0 indicates no more data
    stringLength = strlen( directoryContents->d_name );
    fwrite( &stringLength, sizeof(int), 1, fin );
    fwrite( directoryContents->d_name, stringLength, 1, fin );
  }
  //send zero to indicate completion
  stringLength = 0;
  fwrite( &stringLength, sizeof(int), 1, fin );

  closedir( currentDir );

  return 1;
}

//--------------------------------------------------------

int SingleStream::getReportCount()
{
  //need PatternID
  int patternID;

  //list files
  gatherReports();

  //get patternID
  fread( &patternID, sizeof(int), 1, fin );

  if( verboseFlag )
    cout<<"PatternID: "<<patternID<<endl;

  //use patternID
  if( buildSubset( patternID ) < -1 )
  {
    if( verboseFlag )
      cout<<"Invalid PatternID\n";
    return 1;
  }

  int count = subset.size();
  if( verboseFlag )
    cout<<"Report Count: "<<count<<endl<<endl;

  fwrite( &count, sizeof(int), 1, fin );

  return 1;
}

//--------------------------------------------------------

int SingleStream::buildSubset( int patternID )
{
  int *acceptedReports = NULL;

  subset.clear();

  if( patternID < 0 || patternID >= userPatterns.size() )
    return -1; //is there a way to clear the buffer?

  vector<char *> patterns = userPatterns[patternID];

  if( validReports.size() > 0 )
  {
    acceptedReports = new int[validReports.size()];

    for( int i=0; i<validReports.size(); i++ )
      acceptedReports[i] = 0;
  }

  for( int j=0; j<patterns.size(); j++ )
  {
    for( int i=0; i<validReports.size(); i++ )
    {
      //if a report has matched a previous pattern, I don't need to check it again
      if( !acceptedReports[i] ) //&&
      {
        if( validReports[i]->variedLengthCheckCriteria( patterns[j] ) )
        {
          acceptedReports[i] = 1;
          subset.push_back( validReports[i] );
        }
      }
    }
  }

  if( acceptedReports )
    delete [] acceptedReports;

  return 1;
}

//--------------------------------------------------------

int SingleStream::setSorting()
{
  int result = 1;
  char newOrder[6];

  //read six bytes off stream (don't need to waste space)
  fread( newOrder, 1, 6, fin );

  //confirm that each entry is valid
    //also, allow that client might send binary or ascii 0-5
  for( int i=0; i<6; i++ )
  {
    switch( newOrder[i] )
    {
      case '0': case 0:
        if( verboseFlag )
          cout<<"Sort Priority "<<i<<": Report Name\n";
        newOrder[i] = 0; //for consistency
        break;
      case '1': case 1:
        if( verboseFlag )
          cout<<"Sort Priority "<<i<<": Report On\n";
        newOrder[i] = 1; //for consistency
        break;
      case '2': case 2:
        if( verboseFlag )
          cout<<"Sort Priority "<<i<<": Column Name\n";
        newOrder[i] = 2; //for consistency
        break;
      case '3': case 3:
        if( verboseFlag )
          cout<<"Sort Priority "<<i<<": Layer Name\n";
        newOrder[i] = 3; //for consistency
        break;
      case '4': case 4:
        if( verboseFlag )
          cout<<"Sort Priority "<<i<<": Cell Name\n";
        newOrder[i] = 4; //for consistency
        break;
      case '5': case 5:
        if( verboseFlag )
          cout<<"Sort Priority "<<i<<": Compartment Name\n";
        newOrder[i] = 5; //for consistency
        break;

      default:
        if( verboseFlag )
          cout<<"Bad field ID (must be from 0-5)\n";
        result = 2;
        break;
    }
  }

  //all entries are valid (should I check for duplicates?)
    //possibly
  
  //copy into sortOrder
  for( int i=0; i<6; i++ )
    sortOrder[i] = (int) newOrder[i];

  //send back one byte reply
  char reply = 0;
  if( result > 1 )
    reply = 1;
  
  fwrite( &reply, 1, 1, fin );

  return result;
}

//--------------------------------------------------------

int SingleStream::getTimeCount()
{
  int patternID;
  vector<char*> patterns;
  int minTime=0, compare;
  int *acceptedReports = NULL;

  //which pattern to use
  fread( &patternID, sizeof(int), 1, fin );
    if( verboseFlag )
      cout<<"PatternID: "<<patternID<<endl;

  if( patternID < 0 || patternID >= userPatterns.size() )
    return -1; //is there a way to clear the buffer?

  patterns = userPatterns[patternID];

  gatherReports();

  //use class member subset
  subset.clear();

  if( validReports.size() )
  {
    acceptedReports = new int[validReports.size()];
    for( int i=0; i<validReports.size(); i++ )
      acceptedReports[i] = 0;
  }

  for( int j=0; j<patterns.size(); j++ )
  {
    for( int i=0; i<validReports.size(); i++ )
    {
      //if a report has matched a previous pattern, I don't need to check it again
      if( !acceptedReports[i] && validReports[i]->variedLengthCheckCriteria( patterns[j] ) )
      {
        acceptedReports[i] = 1;
        subset.push_back( validReports[i] );
      }
    }
  }

  if( acceptedReports )
    delete [] acceptedReports;

  if( subset.size() > 0 )
    minTime = subset[0]->checkAvailable();
  for( int i=1; i<subset.size(); i++ )
  {
    compare = subset[i]->checkAvailable();
    if( compare < minTime )
      minTime = compare;
  }
  
  //send minTime
  fwrite( &minTime, sizeof(int), 1, fin );
    if( verboseFlag )
      cout<<"MinimumTime: "<<minTime<<endl<<endl;

  return 0;
}

//--------------------------------------------------------

int SingleStream::getData()
{
  int patternID;
  vector<char*> patterns;
  char *holder;
  int reportTimes[2];
  int *acceptedReports = NULL;

  //which pattern am I using?
  fread( &patternID, 1, sizeof(int), fin );
    if( verboseFlag )
      cout<<"PatternID: "<<patternID<<endl;

  if( patternID < 0 || patternID >= userPatterns.size() )
    return -1; //is there a way to clear the buffer?

  patterns = userPatterns[patternID];

  //what time steps do we need?
  fread( reportTimes, sizeof(int), 2, fin );
    if( verboseFlag )
      cout<<"ReportTimes: "<<reportTimes[0]<<" "<<reportTimes[1]<<endl;

  //for each file in directory, with job name
  // does it match any pattern?
  //if so - include it when reporting
  gatherReports();

  //use class member subset
  subset.clear();

  if( validReports.size() > 1 )
  {
    acceptedReports = new int[validReports.size()];
    for( int i=0; i<validReports.size(); i++ )
      acceptedReports[i] = 0;
  }

  for( int j=0; j<patterns.size(); j++ )
  {
    for( int i=0; i<validReports.size(); i++ )
    {
      //if a report has matched a previous pattern, I don't need to check it again
      if( !acceptedReports[i] && validReports[i]->variedLengthCheckCriteria( patterns[j] ) )
      {
        acceptedReports[i] = 1;
        subset.push_back( validReports[i] );
      }
    }
  }

  if( acceptedReports )
    delete [] acceptedReports;

  //once the subset of reports are selected, need to sort it
  sortReports( subset );

  transmit( subset, reportTimes );

  return 1;
}

//----------------------------------------------------------
/*
int SingleStream::setSorting()
{

}
*/
//----------------------------------------------------------

int SingleStream::gatherReports()
{
  char dirName[MaxLength], jobName[MaxLength];
  DIR *currentDir;
  struct dirent *directoryContents;
  ReportInfo *currentReport;

  //make sure valid reports is cleared
  for( int i=0; i<validReports.size(); i++ )
    delete validReports[i];
  validReports.clear();

  //ReportInfo::peek();

  //Should I change to the current directory?

  //look for file names with jobName in front
  if( (currentDir = opendir( path )) == NULL )
    return -1;
  while( (directoryContents=readdir( currentDir )) != NULL )
  {
    //does the current file have jobName in front
    if( strncmp( directoryContents->d_name, job, strlen(job) ) == 0 )
    {
      currentReport = new ReportInfo( directoryContents->d_name );
      //is the file a valid binary report file?
      //output the names for now, so I fell useful
      if( currentReport->detectNCSbinary() )
      {
        //store this report in a vector
        validReports.push_back( currentReport );
      }
      else
      {
        delete currentReport;
      }
    }
  }

  //ReportInfo::peek();

  closedir( currentDir );

  //do I want to send this list in this function?
  //should I do the communication in a separate function?

  //next I should accept a list (or flags) that tell me which
  //files the user is interested in viewing
}

//--------------------------------------------------------

int SingleStream::sortReports( vector<ReportInfo *> &subset )
{
    //Using the array sortOrder
    //It is easier to list column IDs in the correct order
    //than to keep columns in place with an assigned rank.
    //clarification:
    //  columns are
    //    report name, report on, column, layer, cell, cmp
    //  the sortOrder array has six numbers to represent the order
    //  method 1 - by column ID
    //    2, 1, 3, 5, 4, 0
    //      translates to
    //    column, report on, layer, cmp, cell, report name
    //  method 2 - assign rank
    //    2, 1, 3, 5, 4, 0
    //       translates to
    //    cmp, report on, report name, column, cell, layer
    //  by using method 1, we know the order in one pass
  //how do I do a quicksort again?
  //bubble sort for now
  ReportInfo *temp;
  for( int i=0; i<subset.size(); i++ )
    for( int j=i; j<subset.size(); j++ )
      if( subset[i]->greaterThan( subset[j], sortOrder ) )
      {
        temp = subset[i];
        subset[i] = subset[j];
        subset[j] = temp;
      }

}

//--------------------------------------------------------

int SingleStream::transmit( vector<ReportInfo*> &subset, int reportTimes[] )
{
  int *header;

  if( !sending )
  {
    header = new int[2+subset.size()];
    header[0] = subset.size();
    header[1] = reportTimes[1]-reportTimes[0]+1;
    for( int i=0; i<subset.size(); i++ )
      header[2+i] = subset[i]->getCount();

    //send NxM -> number of reports, number of timesteps
    //send N numbers: number of emements in each report
    fwrite( header, sizeof(int), 2+subset.size(), fin );
      if( verboseFlag )
      {
        cout<<"N: "<<header[0]<<"\nM: "<<header[1]<<"\nValue counts: ";
        for( int i=0; i<subset.size(); i++ )
          cout<<header[2+i]<<" ";
        cout<<endl<<endl;
      }
  }

  sending = 1;

  //send data
    //goto correct timestep
  for( int i=0; i<subset.size(); i++ )
    if( !subset[i]->moveToTimestep( reportTimes[0] ) )
    {
        recall[0] = reportTimes[0];
        recall[1] = reportTimes[1];
        return 0;
    }

  //how will I know if all the data is available?
  //how should I tell the user?
  //when should I tell the user?
  //for example, the user might want as much data
  //as possible. as the sim runs, more accumulates
  //If I already know that the file is complete,
  //I also don't need to keep updating the limit.
  //How do I know if the file is complete? Should there
  //be something in the header that reveals the estimated
  //number of timesteps that will be in the file?
  //That number would be easy to calculate given
  //THe initial values for timestart, timeend, fsv, duration

  for( int j=reportTimes[0]; j<=reportTimes[1]; j++ )
  {
    for( int i=0; i<subset.size(); i++ )
    {
      //confirm that all the reports can write the timestep
      if( subset[i]->readLine() )
        {}//subset[i]->sendLine( fin );
      else
      {
        //if I can't read, I should stop, and pick up where I left off later
        //remember this time
        recall[0] = j;
        recall[1] = reportTimes[1];
        return 0;
      }
    }

    fwrite( &j, sizeof(int), 1, fin );
    for( int i=0; i<subset.size(); i++ )
      subset[i]->sendLine( fin );

    fflush(fin);
  }

  sending = 0;
}

//--------------------------------------------------------

int SingleStream::setPattern()
{
  int numPatterns;
  vector<char*> patterns;
  char *holder;

  //how many patterns am I matching?
  fread( &numPatterns, 1, sizeof(int), fin );
    if( verboseFlag )
      cout<<"Pattern Count: "<<numPatterns<<endl;

  for( int i=0; i<numPatterns; i++ )
  {
    holder = new char[MaxLength];
    fgets( holder, MaxLength, fin );
      if( verboseFlag )
        cout<<"Pattern "<<i<<": "<<holder;
    patterns.push_back( holder );
  }

  //once I have all the patterns, push this vector
  //onto my class vector
  userPatterns.push_back( patterns );

  //return the index to the user
  numPatterns = userPatterns.size()-1;
  fwrite( &numPatterns, sizeof(int), 1, fin );
    if( verboseFlag )
      cout<<"PatternID: "<<numPatterns<<endl<<endl;

  activePattern = numPatterns-1;

  return 1;
}

//--------------------------------------------------------

int SingleStream::selectPattern()
{
  //read off int
  int target;
  fread( &target, sizeof(int), 1, fin );

  if( target >= 0 && target < userPatterns.size() )
    activePattern = target;
}

//--------------------------------------------------------

int SingleStream::setPath()
{
  //read the path and an optional job id
  int result = 0;
  char code;
  char buffer[MaxLength];
  vector<char*> fields;
  char *search, *find;

  fgets( buffer, MaxLength, fin );

  //is there just a path?
  find = buffer;
  search = strtok( find, " \0\t\n" );

  while( search )
  {
    fields.push_back( search );
    search = strtok( NULL, " \0\t\n" );
  }

  //chdir to the path (if it exists)
  if( strlen( fields[0] ) > 0 )
  {
    strcpy( path, fields[0] );
      if( verboseFlag )
        cout<<"SetPath: "<<fields[0]<<endl;

    //should I switch to that directory?
    //if change is successful, return 1 else 0
    result = chdir( fields[0] );
  }

  //set active job to fields[1] (if it exists )
  if( fields.size() > 1 && strlen(fields[1])>0 )
  {
    //remove newline(s)
    for( int i=strlen(fields[1])-1; fields[1][i]=='\n' && i>0; i-- )
      fields[1][i] = 0;

    strcpy( job, fields[1] );

      if( verboseFlag )
        cout<<"SetJob: "<<fields[1]<<endl;
  }

  if( result == 0 ) //chdir was successful
    code = 0;
  else if( result == -1 ) //chdir failed
    code = 1;

  fwrite( &code, 1, 1, fin );
    if( verboseFlag )
      cout<<"Code "<<(code?'1':'0')<<endl<<endl;

  return 1;
}

//-----------------------------------------------------------

int SingleStream::emptyReports()
{
  //clear validReports
  for( int i=0; i<validReports.size(); i++ )
    delete validReports[i];
  validReports.clear();

  //subsets is fine
  subset.clear();
}

//-----------------------------------------------------------

int SingleStream::clear()
{
  //clear validReports
  for( int i=0; i<validReports.size(); i++ )
    delete validReports[i];
  validReports.clear();

  //subsets is fine
  subset.clear();

  //clear patterns
  for( int i=0; i<userPatterns.size(); i++ )
  {
    for( int j=0; j<userPatterns[i].size(); j++ )
      delete [] userPatterns[i][j];
    userPatterns[i].clear();
  }
  userPatterns.clear();

  //this object should remove itself from the descriptor set
  FD_CLR( sd, allset );
  close( sd );
    //if( verboseFlag )
      cout<<"Close socket\n";

  sd = -1;
  fin = NULL;
}

//-----------------------------------------------------------

int SingleStream::operator==( const SingleStream &RHS ) const
{
  //compare sd
  return ( sd == RHS.sd );
}

//-----------------------------------------------------------

int SingleStream::launchNCS()
{
    if( appList == NULL || appList->size() == 0 )
      return 0;

    const int fileSendFlag = 1;
    const int mergeFlag = 1<<1;
    const int queueBypassFlag = 1<<2;
    int result = 0;
    char code = 0;
    char nodeFile[128] = "machines.mpi";

    //Data needed to launch a simulation
    char fileNames[3][MaxLength]; //0 = input, 1 = stdout, 2 = stderr (if needed)
    int getUserFlags[3]; //element 0 - flags, element 1 - nNodes, element 2 - application
    char *command;

    //get flags 
    if( !fread( getUserFlags, sizeof( int ), 3, fin ) ) //either error or eof
        return -1;

    if( getUserFlags[1] <= 0 )  //invalid number
        return -2;
    if( getUserFlags[2] < 0 || getUserFlags[2] >= appList->size() ) //default to zero
        getUserFlags[2] = 0;

    //input file name
    fgets( fileNames[0], MaxLength, fin );
      fileNames[0][strlen( fileNames[0] )-1] = 0;  //remove trailing newline

    if( getUserFlags[0] & fileSendFlag )  //get file contents as well
      receiveFile( fileNames[0] );

    //if bypassing the queue, get node list
    if( getUserFlags[0] & queueBypassFlag )
      receiveFile( nodeFile );

    //output file
    fgets( fileNames[1], MaxLength, fin );
    //remove trailing newline
    for( int i=0; i<MaxLength; i++ )
        if( fileNames[1][i] == '\n' )
            fileNames[1][i] = 0;

    //error file (if needed)
    if( !(getUserFlags[0] & mergeFlag) )
    {
        fgets( fileNames[2], MaxLength, fin );
        //remove trailing newline
        for( int i=0; i<MaxLength; i++ )
            if( fileNames[2][i] == '\n' )
                fileNames[2][i] = 0;
    }

    if( verboseFlag )
    {
      cout<<"Flags: "<<getUserFlags[0]<<"\nNodeCount: "<<getUserFlags[1]
          <<"\nApplication Index: "<<getUserFlags[2]<<" ("
          <<(*appList)[getUserFlags[2]]<<")\n";
      cout<<"Input: "<<fileNames[0]<<"\nOutput: "<<fileNames[1];
      if( !(getUserFlags[0] & mergeFlag) )
        cout<<"Error: "<<fileNames[2];
    }

    if( getUserFlags[0] & queueBypassFlag )
        result = directLaunch( getUserFlags, fileNames );
    else
        result = generateScript( getUserFlags, fileNames );

    //set code based on result - default to zero for now

    //send reply to client
    fwrite( &code, 1, 1, fin );
      if( verboseFlag )
        cout<<"Code "<<(code?'1':'0')<<endl<<endl;;

    return result;
}

//---------------------------------------------------------------

int SingleStream::receiveFile( char *fileName )
{
  char *helper, *baseName = fileName;
  int fileSize = 0;
  FILE *fout;

  //if the name has a fullpath (i.e. directory name) I need
  //to find the final file name
  while( (helper = strstr( baseName, "/" )) )
    baseName = helper+1;

  //on Windows do I need to search for "\"?

  //overwrite the old filename so that it is just the base name
  strcpy( fileName, baseName );

  fout = fopen( fileName, "w" );
  if( !fout )
    return -1;

  //receive the length in bytes
  fread( &fileSize, 1, sizeof( int ), fin );

  helper = new char[fileSize+1];
  fread( helper, fileSize, 1, fin );
  helper[fileSize] = 0;

  fwrite( helper, fileSize, 1, fout );
  fclose( fout );

  delete [] helper;

  return 0;
}

//-----------------------------------------------------------------------------

int SingleStream::directLaunch( int *userFlags, char fileNames[][MaxLength] )
{
  int pid = -1;
  const int myrinetFlag = 1<<3;
  int mpichSelect = 0; //default to ethernet version

  char command[1024];

  if( userFlags[0] & myrinetFlag )
      mpichSelect = 1; //override

  //build command string
  sprintf( command, "%s -np %d -machinefile machines.mpi %s %s -d . > %s 2> %s",
           mpich[mpichSelect], userFlags[1], (*appList)[userFlags[2]], fileNames[0],
           fileNames[1], fileNames[2] );

  if( verboseFlag )
      cerr<<"Executing command: "<<command<<endl;

  //need to fork then call system
  pid = fork();

  if( pid != 0 ) //Parent code
  {
      //server is configured to listen for child interrupts,
      // so it will accepts the child's return code when it
      // completes
  }
  else
  {
      system( command );
      exit( 0 );
  }

  return 1;
}

//-----------------------------------------------------------------------------

int SingleStream::generateScript( int *userFlags, char fileNames[][MaxLength] )
{
  const int fileSendFlag = 1;
  const int mergeFlag = 1<<1;
  const int myrinetFlag = 1<<3;
  int mpichSelect = 0; //default to ethernet

  if ( userFlags[0] & myrinetFlag )
      mpichSelect = 1; //override

  char scriptName[MaxLength];
  char *helper, *baseName;
  char command[MaxLength];

  baseName = fileNames[0];
  while( (helper = strstr( baseName, "/" ) ) )
    baseName = helper+1;

  sprintf( scriptName, "%s.sh", baseName );
  FILE *scriptOut = fopen( scriptName, "w" );
  if( !scriptOut )
    return -1;

  fprintf( scriptOut, "#!/bin/bash\ncd $PBS_O_WORKDIR\n\n" );

  //request nodes
  fprintf( scriptOut, "#PBS -l nodes=%d:cpp=1\n", userFlags[1] );

  //output
  fprintf( scriptOut, "#PBS -o %s\n", fileNames[1] );

  //are outputs to be merged?
  if( userFlags[0] & mergeFlag )
    fprintf( scriptOut, "#PBS -j y\n" );
  else
    fprintf( scriptOut, "#PBS -e %s\n", fileNames[2] );

  //setup parallel environment
  //fprintf( scriptOut, "#$ -pe mpich %d\n", userFlags[1] );

  //final command
  fprintf( scriptOut, "%s -np %d -machinefile $PBS_NODEFILE %s %s -d .\n", mpich[mpichSelect], userFlags[1], (*appList)[userFlags[2]], fileNames[0] );

  fclose( scriptOut );

  //change mode of script
  sprintf( command, "chmod u+x %s", scriptName );
  system( command );

  //submit script
  sprintf( command, "/usr/pbs/bin/qsub %s", scriptName );
  system( command );

  //when can I delete the script?
  return 1;
}

//----------------------------------------------------------

int SingleStream::isSending()
{
  return sending;
}

//----------------------------------------------------------

int SingleStream::invokeScript()
{
  //read off application value
  if( scriptList == NULL || scriptList->size()==0 )
    return 0;

  //I should probably fork off, since this script could take
  //any amount of time to finish.

  //that way, I can still handle requests from other clients

  int pid;
  int scriptNum;
  int size;
  char *arguments=NULL;
  char output[] = "scriptOut";
  char error[] = "scriptErr";
  int total = 0;
  char *final=NULL;
  char byte=0;

  //delete these files if needed
  unlink( output );
  unlink( error );

  fread( &scriptNum, sizeof(int), 1, fin );

  if( verboseFlag )
    cout<<"Received script index "<<scriptNum<<endl;

  //make sure valid, or default to zero
  if( scriptNum < 0 || scriptNum >= scriptList->size() )
  {
    if( verboseFlag )
      cout<<"invalid index, default to zero\n";
    scriptNum = 0;
  }

  if( verboseFlag ) 
    cout<<(*scriptList)[scriptNum]<<endl;

  //arguments
  fread( &size, sizeof(int), 1, fin );

  if( size > 0 )
  {
    arguments = new char[size+1];
    fread( arguments, sizeof( char ), size, fin );
    arguments[size] = 0;

    if( verboseFlag )
      cout<<"Received arguments: "<<arguments<<endl;
  }

  total = strlen((*scriptList)[scriptNum]) + size + strlen(output) +
          strlen(error) + 4;
  final = new char[total];
  final[total-1] = 0;
  sprintf( final, "%s %s > %s 2> %s", (*scriptList)[scriptNum], arguments, output, error );

  if( verboseFlag )
    cout<<"Final script command: "<<final<<endl;

  system( final );

  byte = 0;
  fwrite( &byte, 1, 1, fin );

  delete [] final;
  delete [] arguments;

  //after system finishes, get scriptOut and scriptErr
  FILE *aux;
  aux = fopen( output, "r" );
  fseek( aux, 0, SEEK_END );
  size = ftell( aux );
  fseek( aux, 0, SEEK_SET );

  if( verboseFlag )
    cout<<"Transmit output file: "<<size<<" bytes\n";
  
  char segment[4096]; //anticipate huge sizes - need chunks
  fwrite( &size, sizeof(int), 1, fin );
  int i;
  for( i=0; i+4096<size; i+=4096 )
  {
    fread( segment, 4096, 1, aux );
    fwrite( segment, 4096, 1, fin );
  }
  if ( i<size )
  {
    fread( segment, size-i, 1, aux );
    fwrite( segment, size-i, 1, fin );
  }
  fclose(aux);

  aux = fopen( error, "r" );
  fseek( aux, 0, SEEK_END );
  size = ftell( aux );
  fseek( aux, 0, SEEK_SET );

  if( verboseFlag )
    cout<<"Transmit error file: "<<size<<" bytes\n";

  fwrite( &size, sizeof(int), 1, fin );
  for( i=0; i+4096<size; i+=4096 )
  {
    fread( segment, 4096, 1, aux );
    fwrite( segment, 4096, 1, fin );
  }
  if ( i<size )
  {
    fread( segment, size-i, 1, aux );
    fwrite( segment, size-i, 1, fin );
  }
  fclose(aux);

  if( verboseFlag )
    cout<<endl;

  //the parent will return to serving, the child will finish and exit
  return 1;
}

//----------------------------------------------------------

int SingleStream::makeDirectory()
{
  int done;
  char newDirectory[MaxLength];

  //read new directory name until newline
  fgets( newDirectory, MaxLength, fin );

  done = 0;
  for( int i=strlen(newDirectory)-1; i>=0 && !done; i-- )
  {
    if( newDirectory[i] == '\n' )
      newDirectory[i] = 0;
    else
      done = 1;
  }
  if( verboseFlag )
    cout<<"Creating directory: "<<newDirectory<<endl;

  //try to create directory
  int result = 0;
  result = mkdir( newDirectory, 0744 );

  char code;
  if( result == -1 )
    code = 1;
  else
    code = 0;
  fwrite( &code, 1, 1, fin );

  return 1;
}

//----------------------------------------------------------

int SingleStream::writeData()
{
    //get the key of the Storage object the user is interested in
    
    //retrieve first data item
    
    //send to socket
    
    //remove data
    
    return 0;
}

//----------------------------------------------------------

int SingleStream::readData()
{
    //get the key of the Storage object the user is interested in
    
    //accept data
    
    //append it
    
    //parse it
    
    return 0;
}

//----------------------------------------------------------

int SingleStream::getKey()
{
    //get name of object
    
    //search vector
    
    //return key
    
    return 0;
}

/* This is the top level routine of the actual brain code */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <deque>

#include "Brain.h"
#include "Managers.h"
#include "DoStim.h"
#include "DoReport.h"
#include "Port.h"

#include "debug.h"
#include "memstat.h"

#define MEM_KEY  KEY_BRAIN

#include "QQ.h"

int QQDoStim, QQDoCell, QQDoReport;

int LearnCode( char *str );

using namespace std;

Brain::Brain (char *filename, char *nothing)
{
  Load (filename, nothing);
  TimeStep++;
}

Brain::Brain (T_BRAIN *iBrain)
{
  MEMADDOBJ (MEM_KEY);

  QQDoStim   = QQAddState ("DoStim");
  QQDoCell   = QQAddState ("DoCell");
  QQDoReport = QQAddState ("DoReport");

  flag = iBrain->flag;

  TotalTime = iBrain->Duration;   //total time to run the program - seconds - input from user
        
  ReportTime = 10;

  if (NI->FSV == 0)
    CycleCount = (long) (TotalTime);
  else
    CycleCount = (long) (TotalTime * NI->FSV);       

  SaveFlag = ResetFlag = false;

  nStimulus = 0;
  StimList = NULL;

  nReports = 0;
  RptList = NULL;

    SaveTime = -1.0;
  SaveFile = NULL;
  if( iBrain->savefile != NULL )
  {
    SaveTime = iBrain->savetime;
    if( NI->FSV != 0 ) SaveTime = SaveTime * NI->FSV - 1;

    SaveFile = iBrain->savefile;
  }
  if( iBrain->loadfile != NULL )
  {
    printf( "Restoring basic sturcture definitions\n" );
    LoadArrays( iBrain->loadfile );
  }

  //Port and Hostname should be 0 and NULL in iBrain if not set in input file
  Port = iBrain->Port;
  HostName = iBrain->HostName;
  HostPort = iBrain->HostPort;

  //set up communicator for runtime commands
  MPI_Comm_dup (MPI_COMM_WORLD, &commandComm );
  MPI_Comm_group (MPI_COMM_WORLD, &commandGroup );
  
    pendingCommands = NULL;
  pendingLength = 0;
  
  sd = -1;
  input = NULL;
  
  //Cell coordinates will be allocated and assigned in BuildBrain.cpp if they are needed.
    //When are they needed? -> If synaptic distances will add delay to the synapse
  CellPosition[0] = CellPosition[1] = CellPosition[2] = 0;
}

Brain::~Brain ()
{
  int i;

  MEMFREEOBJ (KEY_BRAIN);

  for (i = 0; i < nCells; i++) 
    delete Cells [i];
  free (Cells);

//for (i = 0; i < nSynapseDef; i++)
//  delete SynapseDefs [i];
  free (SynapseDefs);
}

/*------------------------------------------------------------------------*/
/* This is the main thinking loop of the brain.  At each iteration, it calls */
/* the DoStimuli routine to get any incoming stimulus for the TimeStep,      */
/* loops through all the cells to process the stimulus (including synapse    */
/* activity which is now sent/received in the background.  It then calls     */
/* DoReports to handle any needed reporting, and finally synchronizes as     */
/* necessary with other nodes.                                               */


int Brain::DoThink ()
{
  double tstart, tstop, t0, tprev;
  int j, iRcvChk, nRcvChk;
  char filename [256], nothing [] = "Nothing to see here folks\n";
  bool haveMsg;
  int commFlag = 0;

  printf ("Node %d Is Thinking...\n", NI->Node);

  tstart = tprev = MPI_Wtime ();  

  //set up command port if necessary
  if( NI->Node == 0 && (Port > 0 || Port == -1 ) )
  {
    input = setupPort();
  }

  iRcvChk =   0;
  nRcvChk = 500;
  for (TimeStep = 0; TimeStep < CycleCount; TimeStep++) 
  {     
    if (NI->Node == 0)
    {
      t0 = MPI_Wtime ();
      printf ("T = %4d, dt = %f, et = %f\n", TimeStep, t0 - tprev, t0 - tstart);
      tprev = t0;
    }

    if( Port > 0 || Port == -1 )  //check for commands
    {
      commFlag = CommandCheck();
      if( commFlag == 15 ) //user sent exit command
      {
        //Reports have already been closed during command execution,
        if( NI->Node == 0 )
          printf( "Exiting from DoThink loop\n" );
        break;  //is this inappropriate?  should I save state?
      }
    }

/* Do the stimuli, which will also produce local messages */
    QQStateOn (QQDoStim);
    DoStimuli (TimeStep);
    QQStateOff (QQDoStim);
//  printf ("Brain::DoThink: DoStim done\n");

/* Deliver any pending messages for this timestep */
    MsgBus->DeliverMsgs (TimeStep);
//  printf ("Brain::DoThink: DeliverMsgs done\n");

    QQStateOn (QQDoCell);
    for (j = 0; j < nCells; j++)
    {
//    printf ("Brain::DoThink: DoProcessCell %d\n", j);
      Cells [j]->DoProcessCell (TimeStep);

      if (iRcvChk == nRcvChk)
      {
        haveMsg = true;
        while (haveMsg)
          haveMsg = MsgBus->ReceiveMsgs (TimeStep, false);

        iRcvChk = 0;
      }
      iRcvChk++;
    }

    QQStateOff (QQDoCell);
//  printf ("Brain::DoThink: Cell loop done\n");

/* Make sure all messages for this timestep have been sent, with sync flags */
    MsgBus->FlushMsgs (TimeStep);
//  printf ("Brain::DoThink: Flushed\n");

//  printf ("Brain::DoThink: calling DoReports\n");
    QQStateOn (QQDoReport);
    DoReports (TimeStep);
    QQStateOff (QQDoReport);

    /* After reporting is done, reset the Compartment Stimulus Current tracking
       variable.  This is done now, so that any reports on Stimulus Current
       will have completed. */
    for ( j=0; j<nCells; j++ )
       for( int k=0; k<Cells[j]->nCompartments; k++ )
         Cells[j]->Compartments[k]->PrepareStimulusCurrent();

/* Synchronize here.  Makes sure this node has gotten the messages for the   */
/* next timestep from all of the nodes it receives from.                     */
//  printf ("Brain::DoThink: Calling Sync\n");
    MsgBus->Sync (TimeStep);

// if save or reset command has been given do now at end of timestep
    if (SaveFlag)
    {
      SaveFlag = false;
      Save (filename);
    }
   
    if (ResetFlag)
    {
      Reset (filename, nothing);
      fprintf (stderr, "%d: Successful return.\n", NI->Node);
      ResetFlag = false;
    }

    if( SaveFile && TimeStep == SaveTime )
    {
      Save( SaveFile );
    }

#ifdef PSG_STATS
    PSGMgr->Write ();
#endif  

//#ifdef MSG_STATS
//    MsgMgr->Write ();
//#endif
  } 
  tstop = MPI_Wtime ();
  printf ("Brain::DoThink: actual thinking time = %f sec\n", tstop - tstart);

  fprintf (stderr, "Node %d is done Thinking...\n", NI->Node);
  return (0); 
}

/*------------------------------------------------------------------------*/

FILE *Brain::setupPort()
{
  if( Port > 0 ) //connect to server using HostName and Port
  {
    return OpenPortFile( HostName, Port, "r" );
  }
  else if( Port == -1 )
  {
    //need to communicate with server to setup a named communication
    sd = OpenPort( HostName, HostPort ); //contact main server line
    
    if( sd < 0 ) //failed
      return NULL;

    //mode = read
    //format = ascii
    //name = type+job

    //send keyword
    write( sd, "request\n", 8 );

    char *label = NULL;
    int length = 0;
    length += strlen( "read\n" ) + strlen( "ascii\n" ) + strlen( NI->AR->Brain->L.name ) + strlen( NI->job ) + 1;
    label = new char[ length + 1 ];
    sprintf( label, "read\nascii\n%s%s\n", NI->AR->Brain->L.name, NI->job );
    write( sd, label, length );

    //now the server will automatically setup the rest of the communication,
    //I will just switch from an sd to a FILE *
    return fdopen( sd, "r" );
  }
  else
    return NULL;
}

/*------------------------------------------------------------------------*/
//CommandCheck: see if any commands exist

int Brain::CommandCheck()
{
  int commFlag = 0;
  char commBuffer[4096] = {};  //Will this buffer ever need to be bigger?

  //can I do everything in this function?
  if( NI->Node == 0 )
  {
    //use fgets (instead of fread) since commands are of variable length
    
    //read off comm buffer
    if( !fgets( commBuffer, 4095, input ) )
      fprintf( stderr, "Error on command read\n" );
    else if( strlen( commBuffer ) > 1 ) //don't consider the newline
      commFlag = 1;
    else
      commFlag = 0;
  }

  //receive commFlag - everybody
  MPI_Bcast( &commFlag, 1, MPI_INT, 0, commandComm );

  //will let each node receive the full command, then each node will parse it themselves
  if( commFlag )
  {
    //receive entire command, and parse it
    MPI_Bcast( commBuffer, 4096, MPI_CHAR, 0, commandComm );

    //all should now parse the command
    return CommandParse( commBuffer );
  }

  return 0;
}

/*------------------------------------------------------------------------*/
//Command Parse - separate commands before executing

int Brain::CommandParse( char *commBuffer )
{
  int rc = 0;
  char *lasttoken;

  //can I use a perl-like split?
  vector<char *> result;
  lasttoken = strtok( commBuffer, " \t" );  //pass original string to c string tokenizer
  while( lasttoken != NULL )
  {
    //can't use strtok again to find ';'. Have to do it myself
    for( int j=0; lasttoken[j]; j++ )
    {
      if( lasttoken[j] == ';' ) //end of one command, remove ';'
      {
        lasttoken[j] = '\0';
        if( lasttoken[0] )  //make sure we have something - not just the null terminator
          result.push_back( lasttoken );
        //Execute the command we've found
        rc = CommandExec( result );
        result.clear();
        lasttoken = lasttoken+j+1;
        j = -1;  //when loop iterates, this will become j = 0;
      }
    }
    if( lasttoken[0] )  //word still remains - push it onto vector
      result.push_back( lasttoken );
    lasttoken = strtok( NULL, " \t" );   //use strtok again; picks up where it left off
  }

  rc = CommandExec( result );
  result.clear();
  return rc;
}

/*------------------------------------------------------------------------*/
//CommandExec - parse and execute commands received

int Brain::CommandExec( vector<char *> &result )
{
  int rc = 0;
  
  if( result.size() < 1 ) //there was nothing?
    return -1;

  if( strcasecmp( result[0], "SETHEBBIAN" ) == 0 )
    rc = CommandHebbian( result );
	
  else if( strcasecmp( result[0], "EXIT" ) == 0 )
    rc = CommandExit( result );
	
  else if( strcasecmp( result[0], "APPENDSTIM" ) == 0 )
    rc = CommandStim( result );
    
  else if( strcasecmp( result[0], "SAVE" ) == 0 )
    rc = CommandSave( result );
  
  return rc;
}

/*------------------------------------------------------------------------*/
// functions to perform commands

int Brain::CommandSave( vector<char*> &result )
{
    //format: SAVE [filename]
      //if a filename is passed, save there
      //otherwise, check this->SaveFile for a filename
    
    if( result.size() < 2 ) //only command "SAVE" sent with no filename
    {
        if( SaveFile )
        {
            Save( SaveFile );
            return 0;
        }
        return -1;
    }
    else
        Save( result[1] );
        
    return 0;
}

/*------------------------------------------------------------------------*/

int Brain::CommandStim( vector<char*> &result )
{
  //format: APPENDSTIM [flags] TStart [TStop]
  int foundName = 0, foundInstance = 0, timestepFlag = 0;
  int *temp, newStart, newStop, newTimeCount;
  int loopstart = 1;
  int increment = 3;
  const double autoStopDuration = .025; //25 milliseconds
  Stimulus *stim = NULL;

  //make sure minimum number of parameters are there
  if( result.size() < 4 ) 
    return -1;

  //any flags?
  if( result[1][0] == '-' )
  {
    loopstart = 2;  //the loop will need to start at vector index 2 since commands took up vector index 1
    //look for certain letters in result[1]
    for( int i=1; result[1][i]; i++ )
    {
      switch( result[1][i] )
      {
        case 'a': //auto select TStop as 2.5 milliseconds after the specified TStart
          increment = 2;
          break;
        case 's': //seconds are being sent (default)
          timestepFlag = false;
          break;
        case 't': //timesteps are being sent
          timestepFlag = true;
          break; 
        default:
          break;
      }
    }
  }

  //this method of appending new times is probably very inefficient
  //linear search to fins stimulus name, then another linear search to find stimulus definition
 for( int n=loopstart; n+increment <= result.size(); n+=increment )
 {
  stim = StimList;
  foundName = foundInstance = 0;
  for( int i=0; i<NI->AR->nStimulus && !foundName; i++ )
  {
    //compare names
    if( strcmp( result[n], NI->AR->Stimulus[i]->L.name ) == 0 )
    {
      foundName = 1;
      //find actual stimulus
      while( stim != NULL && !foundInstance )
      {
        if( stim->idx == NI->AR->Stimulus[i]->L.idx )
        {
          foundInstance = 1;
          if( timestepFlag )  //times are sent as timestep
          {
            newStart = atoi( result[n+1] );
            if( increment == 3 )              //time stops are also being sent
              newStop = atoi( result[n+2] );
            else                              //auto stop
              newStop = (int) ( newStart + (double) NI->FSV * autoStopDuration + EPSILON );
          }
          else  //seconds must be converted to timesteps using FSV
          {
            newStart = (int) ( atof( result[n+1] ) * (double) NI->FSV + EPSILON );
            if( increment == 3 )  //three values means name, start, and stop all sent
              newStop = (int) (atof( result[n+2] ) * (double) NI->FSV + EPSILON );
            else                  //two values means only name and start sent
              newStop = (int) ( newStart + (double) NI->FSV * autoStopDuration + EPSILON );
          }

          //I need to increment the time count in the Stimulus object
          //And make sure that all pointers to the start/end time lists get moved
          //properly.  There are two sets of these pointers: one in the Stimulus
          //and another in the T_STIMULUS object created during parsing

          newTimeCount = stim->nTimes+1;
          temp = (int*) calloc (2*newTimeCount, sizeof( int ) );
          memcpy( temp, stim->TStart, sizeof(int) * stim->nTimes );
          memcpy( temp+newTimeCount, stim->TStop, sizeof(int) * stim->nTimes );
          stim->TStart = temp;
          stim->TStop  = temp+newTimeCount;
          stim->TStart[stim->nTimes] = newStart;
          stim->TStop[stim->nTimes]  = newStop;
          stim->nTimes = newTimeCount;
        }
        else
          stim = stim->next;
      }
    }
    
  }
 }
  
  return 0;
}

/*------------------------------------------------------------------------*/

int Brain::CommandReport( vector<char*> &result )
{
  //format: APPENDREPORT [flags] TStart [TStop]
  int foundName = 0, foundInstance = 0, timestepFlag = 0;
  int *temp, newStart, newStop, newTimeCount;
  int loopstart = 1;
  int increment = 3;
  const double autoStopDuration = .025; //25 milliseconds
  Report *report = NULL;

  //make sure minimum number of parameters are there
  if( result.size() < 4 ) 
    return -1;

  //any flags?
  if( result[1][0] == '-' )
  {
    loopstart = 2;  //the loop will need to start at vector index 2 since commands took up vector index 1
    //look for certain letters in result[1]
    for( int i=1; result[1][i]; i++ )
    {
      switch( result[1][i] )
      {
        case 'a': //auto select TStop as 2.5 milliseconds after the specified TStart
          increment = 2;
          break;
        case 's': //seconds are being sent (default)
          timestepFlag = false;
          break;
        case 't': //timesteps are being sent
          timestepFlag = true;
          break; 
        default:
          break;
      }
    }
  }

  //this method of appending new times is probably very inefficient

 for( int n=loopstart; n+increment <= result.size(); n+=increment )
 {
  report = RptList;
  foundName = foundInstance = 0;
  for( int i=0; i<NI->AR->nReports && !foundName; i++ )
  {
    //compare names
    if( strcmp( result[n], NI->AR->Reports[i]->L.name ) == 0 )
    {
      foundName = 1;
      //find actual stimulus
      while( report != NULL && !foundInstance )
      {
        if( report->idx == NI->AR->Reports[i]->L.idx )
        {
          foundInstance = 1;
          if( timestepFlag )  //times are sent as timestep
          {
            newStart = atoi( result[n+1] );
            if( increment == 3 )
              newStop = atoi( result[n+2] );
            else
              newStop = (int) ( newStart + (double) NI->FSV * autoStopDuration + EPSILON );
          }
          else  //seconds must be converted to timesteps using FSV
          {
            newStart = (int) ( atof( result[n+1] ) * (double) NI->FSV + EPSILON );
            if( increment == 3 )  //three values means name, start, and stop all sent
              newStop = (int) (atof( result[n+2] ) * (double) NI->FSV + EPSILON );
            else                  //two values means only name and start sent
              newStop = (int) ( newStart + (double) NI->FSV * autoStopDuration + EPSILON );
          }

          //I need to increment the time count in the Report object
          //And make sure that all pointers to the start/end time lists get moved
          //properly.  There are two sets of these pointers: one in the Report
          //and another in the T_REPORT object created during parsing

          newTimeCount = report->nTimes+1;
          temp = (int*) calloc (2*newTimeCount, sizeof( int ) );
          memcpy( temp, report->TStart, sizeof(int) * report->nTimes );
          memcpy( temp+newTimeCount, report->TStop, sizeof(int) * report->nTimes );
          report->TStart = temp;
          report->TStop  = temp+newTimeCount;
          report->TStart[report->nTimes] = newStart;
          report->TStop[report->nTimes]  = newStop;
          report->nTimes = newTimeCount;
        }
        else
          report = report->next;
      }
    }
    
  }
 }
  
  return 0;
}

/*------------------------------------------------------------------------*/

int Brain::CommandHebbian( vector<char*> &result )
{
    int found = 0;
    //this command needs at least 3 arguments total - command, target, setting
    if( result.size() < 3 )
      return -1;

    //change hebbian setting on synapse x
    //actually, just need to change synapsedef object
    //this means that any synapse that uses that synapsedef will be changed as well
        //christine points out that each synapse will get a unique synapsedef anyways

    //need to extract out name, and learning code
  for( int n=1; n+2<=result.size(); n+=2 )
  {
    found = 0;
    for( int i=0; i<NI->AR->nSynapse && !found; i++ )
    {
      //go through synapsedefs to find the one that has the corect synapse
      //type as its owner
      if( strcmp( SynapseDefs[i]->source->L.name, result[n] ) == 0 ) //found
      {
        found = 1;
        SynapseDefs[i]->SetLearning( LearnCode( result[n+1] ) );
      }
    }
  }

  return 0;
}

/*------------------------------------------------------------------------*/

int Brain::CommandExit( vector<char*> &result )
{
     //the brain does not need to continue running, so cleanup reports (at least - other things need cleaning up?)
    //and make the brain exit the DoThink loop.
    // how?
    //  1: set final timestep to current timestep?
    //  2: use break/continue? -> I'll use this method for now

    //delete all reports in list.  Destructors will make sure report closes correctly.
    Report *rpt = RptList;
    for( int i=0; i<nReports; i++ )
    {
      if( rpt->next )  //another report exist after this one
      {
        rpt = rpt->next;  //move to it
        delete rpt->prev; //delete where we were
        rpt->prev = NULL;
      }
      else             //last report in list
      {
        delete rpt;
        rpt = RptList = NULL;
      }
    }
    nReports = 0;

    return 15;
}

/*------------------------------------------------------------------------*/
//code name to value - this function is duplicated from one used during
//parsing.

int LearnCode( char *str )
{
  int rc;

  rc = 0;
  if (strcasecmp (str, "+HEBBIAN") == 0)
    rc = LEARN_POS;
  else if (strcasecmp (str, "-HEBBIAN") == 0)
    rc = LEARN_NEG;
  else if (strcasecmp (str, "BOTH") == 0)
    rc = LEARN_POS | LEARN_NEG;
  else if (strcasecmp (str, "NONE") != 0)
  {
    rc = INVALID;
  }
  return (rc);
}

/*------------------------------------------------------------------------*/
//State Save: Takes all the brain's current values and writes them to a file
//all right, now I need to make this work in parallel
//all the nodes write to their tmp dirs then send to Node 0
//Node 0 will copy its data to the final file along with all other nodes
//on load, Node 0 will read the final file, sending pieces to the other nodes
//they will write it to tmp, then finally, open it and read it normal
//this sounds slow - oh well

void Brain::Save( char *fname )
{
  FILE *out, *final;
  char filename[128];
  const unsigned int dataBlockSize = 1024;
  char dataBlock[ dataBlockSize ];
  MPI_Status Status;
  int i;
  unsigned int nbytes = 0, offset, amountRead, dataSize;
  int tag = 1;
  vector <Synapse *> collection;
  int *synapseInfo;

  const int iSize = 4;
  int iholder[iSize];
  
  if (NI->job == NULL)
    sprintf (filename, "/tmp/Save%d", NI->Node);
  else
    sprintf( filename, "/tmp/%sSave%d", NI->job, NI->Node);
  out = fopen( filename, "w" );

  if( !out )
  {
    fprintf( stderr, "Error: could not open destination save file %s\n", filename );
    return;
  }
  
  if( NI->Node == 0 ) //open final output
  {
    final = fopen( fname, "w" );
    if( !final )
    {
      fprintf( stderr, "could not open final file %s.\n", fname );
      return;
    }
  }

  //write out general information - # of nodes, ARRAYS struct, cellManager
  //this could be done sooner, rather than waiting until the entire brain is saved
  if( NI->Node == 0 )
  {
    //Write out how many nodes were being used, since load must occur to same number
    fwrite( &NI->nNodes, sizeof( int ), 1, final );//out );

    //write out a fake int saying how big ARRAYS is - will overwrite later
    offset = ftell( final );
    fwrite( &nbytes, sizeof( unsigned int ), 1, final );//out );

    nbytes += saveArrays( NI->AR, final ); //out );

    nbytes += CellMgr->Save( final );

    //go back and overwrite dummy value with real value
    fseek( final, offset, SEEK_SET );
    fwrite( &nbytes, sizeof( unsigned int ), 1, final ); //out );
    
    //return to end of file for continued saving
    fseek( final, 0, SEEK_END );
  }

  //The following objects can differ between nodes
  //reset nbytes (if necessary)
  nbytes = 0;

  //Save individual Node Info
  NI->Save( out );
  
  //saving Brain values
  i=0;
  iholder[i++] = CycleCount;  //does this need to be saved? will it be overwritten?
  iholder[i++] = TimeStep;    //This ~should~ be saved because some objects may look at relative time
  //iholder[i++] = ReportTime;  //Is this used?
  //iholder[i++] = StartTime;   //Is this used?
  iholder[i++] = nCells;
  iholder[i++] = nSynapseDef;
  fwrite( iholder, i, sizeof( int ), out );
    nbytes += sizeof( int ) * i;

  fwrite( &TotalTime, 1, sizeof( double ), out ); //is this needed? will it be overwritten
    nbytes += sizeof( double );

  //synapse defs - these might not differ between nodes, but need brain saved/loaded 1st
  for( i=0; i<nSynapseDef; i++ )
    nbytes += SynapseDefs[i]->Save( out );

  //just save cells for now - testing
  for( i=0; i<nCells; i++ )
    nbytes += Cells[i]->Save( out );

  //need to save connection info separately
  for( int ReceiverNode=0; ReceiverNode<NI->nNodes; ReceiverNode++ )
  {
    //every one needs to gather memory addresses for the target Node (even itself)
    //problem -> should I have the node collect its own collection later?
    for( int i=0; i<nCells; i++ )
      nbytes += Cells[i]->SaveConnections( collection, ReceiverNode );
 
    dataSize = collection.size();

    //write out data size - note: we send an array with this many elements, but get back 3 times as much
    fwrite( &dataSize, sizeof( unsigned int ), 1, out );
      nbytes += sizeof( unsigned int );

    if( NI->Node != ReceiverNode ) //if this node is not the receiver, it needs to send via mpi
    {
      //send data size
      MPI_Send( &dataSize, 1, MPI_INT, ReceiverNode, tag, MPI_COMM_WORLD );

      if( collection.size() > 0 ) //allocate buffer and recv and write
      {
        //send collected addresses
        MPI_Send( &collection[0], dataSize*sizeof( Synapse *), MPI_BYTE, ReceiverNode, tag, MPI_COMM_WORLD );

        synapseInfo = new int [ collection.size() * 3 ];
          //recv info
        MPI_Recv( synapseInfo, dataSize*3, MPI_INT, ReceiverNode, tag, MPI_COMM_WORLD, &Status );
          //write to disk
        fwrite( synapseInfo, sizeof( int ), dataSize*3, out );
          nbytes += sizeof( int ) * 3*dataSize;
          //free memory
        delete [] synapseInfo;
      }

      collection.clear();
    }
    else //Receiver Node - get ready to recv from every other nodes, or just write out your own data
    {
      for( int AskingNode=0; AskingNode<NI->nNodes; AskingNode++ )
      {
        if( AskingNode != ReceiverNode ) //don't use MPI to talk to yourself, jump to else and gather data directly
        {
          //MPI_Recv data requests from the jth AskingNode, gather info into buffer, send back
          //wait! If I over write dataSize, then bad things happen
          MPI_Recv( &dataSize, 1, MPI_INT, AskingNode, tag, MPI_COMM_WORLD, &Status );

          if( dataSize > 0 )
          {
            collection.resize( dataSize ); //will populate with zero addresses, then overwrite

            MPI_Recv( &collection[0], dataSize*sizeof( Synapse *), MPI_BYTE, AskingNode, tag, MPI_COMM_WORLD, &Status );
            
            //go to specified addresses and get data
            synapseInfo = new int [ dataSize*3 ];
            for( unsigned int targetSynapse=0; targetSynapse<dataSize; targetSynapse++ )
            {
              synapseInfo[3*targetSynapse] = collection[targetSynapse]->Cmp->CellID;
              synapseInfo[3*targetSynapse+1] = collection[targetSynapse]->Cmp->CmpID;
              synapseInfo[3*targetSynapse+2] = collection[targetSynapse]->getID();//SynapseID;
            }

            //send it back
            MPI_Send( synapseInfo, dataSize*3, MPI_INT, AskingNode, tag, MPI_COMM_WORLD );

            delete [] synapseInfo;
          }
        }
        else //if ( dataSize > 0 ) //gather data directly, if there are any synapses
        {

//I overwrote my data (probbly), so gather it up again
collection.clear();
    for( int i=0; i<nCells; i++ )
      nbytes += Cells[i]->SaveConnections( collection, ReceiverNode );

    dataSize = collection.size();
          if( dataSize > 0 )
{

          synapseInfo = new int [ dataSize*3 ];
          for( unsigned int targetSynapse=0; targetSynapse<collection.size(); targetSynapse++ )
          {
            synapseInfo[3*targetSynapse]   = collection[targetSynapse]->Cmp->CellID;
            synapseInfo[3*targetSynapse+1] = collection[targetSynapse]->Cmp->CmpID;
            synapseInfo[3*targetSynapse+2] = collection[targetSynapse]->getID();//SynapseID;
          }
          fwrite( synapseInfo, sizeof( int ), 3*collection.size(), out );
            nbytes += sizeof( int ) * 3*collection.size();
          delete [] synapseInfo;
}
        }
        collection.clear();
      }
    }
  }
    
  fclose( out );
  
  //Node 0 will collect data
  //first Node 0 will do itself
  if( NI->Node == 0 )
  {
    out = fopen( filename, "r" );
    
    //compare nbytes with actual size - is there a difference?
    fseek( out, 0, SEEK_END );
    
    //yes - nbytes too large! use ftell for now
    offset = ftell( out );
    fseek( out, 0, SEEK_SET );
    fwrite( &offset, sizeof( unsigned int ), 1, final );
    
    for( amountRead=0; amountRead+dataBlockSize < offset; amountRead+=dataBlockSize )
    {
      fread( dataBlock, 1, dataBlockSize, out );
      fwrite( dataBlock, 1, dataBlockSize, final );
    }
    if( amountRead < offset ) //if anything left, write that bit
    {
      fread( dataBlock, 1, offset-amountRead, out );
      fwrite( dataBlock, 1, offset-amountRead, final );
    }
    
    fclose(out);
  }
  
  //all other nodes send to Node 0
  for( i=1; i<NI->nNodes; i++ )
  {
    if( NI->Node == i )
    {
      out = fopen( filename, "r" );
    
      //compare nbytes with actual size - is there a difference?
      fseek( out, 0, SEEK_END );
      
      //yes - nbytes too large! use ftell for now
      offset = ftell( out );
      fseek( out, 0, SEEK_SET );
      
      MPI_Send( &offset, 1, MPI_INT, 0, tag, MPI_COMM_WORLD );
    
      for( amountRead=0; amountRead+dataBlockSize < offset; amountRead+=dataBlockSize )
      {
        fread( dataBlock, 1, dataBlockSize, out );
        //fwrite( dataBlock, 1, dataBlockSize, final );
        MPI_Send( dataBlock, dataBlockSize, MPI_BYTE, 0, tag, MPI_COMM_WORLD );
      }
      if( amountRead < offset ) //if anything left, write that bit
      {
        fread( dataBlock, 1, offset-amountRead, out );
        //fwrite( dataBlock, 1, offset-amountRead, final );
        MPI_Send( dataBlock, offset-amountRead, MPI_BYTE, 0, tag, MPI_COMM_WORLD );
      }

      fclose(out);
    }
    else if( NI->Node == 0 ) //receive data
    {
      MPI_Recv( &offset, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &Status );
      fwrite( &offset, sizeof( unsigned int), 1, final );
      
      for( amountRead=0; amountRead+dataBlockSize < offset; amountRead+=dataBlockSize )
      {
        //fread( dataBlock, 1, dataBlockSize, out );
        MPI_Recv( dataBlock, dataBlockSize, MPI_BYTE, i, tag, MPI_COMM_WORLD, &Status );
        fwrite( dataBlock, 1, dataBlockSize, final );
      }
      if( amountRead < offset ) //if anything left, write that bit
      {
        MPI_Recv( dataBlock, offset-amountRead, MPI_BYTE, i, tag, MPI_COMM_WORLD, &Status );
        //fread( dataBlock, 1, offset-amountRead, out );
        fwrite( dataBlock, 1, offset-amountRead, final );
      }
    }
  }

  if( NI->Node == 0 )
    fclose( final );
}

/*------------------------------------------------------------------------*/
// Function To load Brain to a previous state from a file
// assume same sized memory used, but not necessarily same addresses

// Currently both reset and load have each node read the file directly from the
// user's current working directory.  This could probably be replaced with a 
// method that doesn't require the nodes to have access to that file.

void Brain::Reset( char *fname, char *nothing )
{
}

/*------------------------------------------------------------------------*/

void Brain::LoadArrays( char *filename )
{
  FILE *in, *out;
  char fname[1024];
  unsigned int nbytes = 0;
  unsigned int offset, amountRead;
  const unsigned int dataBlockSize = 1024;
  char dataBlock[ dataBlockSize ];
  int nNodesSaved;

  sprintf( fname, "/tmp/%sSave0", NI->job );
  //fprintf( stderr, "Node %d: filename - %s\n", NI->Node, fname );
  out = fopen( fname, "w" );
  if( !out )
  {
    fprintf( stderr, "Node %d: Error: could not open temporary file for loading initial data\n", NI->Node );
    return;
  }
  
  if( NI->Node == 0 )
  {
    in = fopen( filename, "r" );
    if( !in )
	{
	  fprintf( stderr, "Error: could not open save file: %s\n", filename );
	  return;
	}

    fread( &nNodesSaved, sizeof(int), 1, in );
    if( nNodesSaved != NI->nNodes )
    {
      fprintf( stderr, "Error: Node count mismatch. %d Nodes saved, %d Nodes currently\n", nNodesSaved, NI->nNodes );
      return;
    }

    fread( &offset, sizeof( unsigned int ), 1, in );
  }
  
  //broadcast offset
  MPI_Bcast( &offset, 1, MPI_INT, 0, MPI_COMM_WORLD );
  
  for( amountRead = 0; amountRead+dataBlockSize<offset; amountRead+=dataBlockSize )
  {
    //read data in. broadcast to all nodes
    if( NI->Node == 0 )
      fread( dataBlock, 1, dataBlockSize, in );
    MPI_Bcast( dataBlock, dataBlockSize, MPI_BYTE, 0, MPI_COMM_WORLD );
    fwrite( dataBlock, 1, dataBlockSize, out );
  }
  if( amountRead < offset ) //deal with last remaining
  {
    if( NI->Node == 0 )
      fread( dataBlock, 1, offset-amountRead, in );
    MPI_Bcast( dataBlock, offset-amountRead, MPI_BYTE, 0, MPI_COMM_WORLD );
    fwrite( dataBlock, 1, dataBlockSize, out );
  }

  fclose( out );
  if( NI->Node == 0 )
    fclose( in );
  in = fopen( fname, "r" );
  
  //load arrays struct
  nbytes += loadArrays( NI->AR, in );

  //load cell manager - but has it been allocated?
  CellMgr = new CellManager();
  CellMgr->Load(in);

  fclose( in );
}

/*------------------------------------------------------------------------*/
// Function to load a Brain from a file into a new brain

void Brain::Load( char *filename, char *nothing )
{
  //char nothing[] = "These are not the droids you seek\n";
  FILE *in, *tempOut;
  char fname[1024];
  unsigned int i, nbytes = 0;
  unsigned int offset, amountRead;
  const unsigned int dataBlockSize = 1024;
  char dataBlock[ dataBlockSize ];
  int tag = 1;
  MPI_Status Status;

  int nNodesSaved;

  if( NI->Node == 0 )
    printf( "Loading Brain state\n" );

  const int iSize = 4;
  int iholder[iSize];

  sprintf( fname, "/tmp/%sSave0", NI->job );
  tempOut = fopen( fname, "w" );

  if( !tempOut )
  {
	fprintf( stderr, "Error: could not open temporary file for loading primary data\n" );
	return;
  }

  if( NI->Node == 0 )
  {
    in = fopen( filename, "r" );
    if( !in )
	{
	  fprintf( stderr, "Error: could not open save file %s\n", filename );
	  return;
	}
  }
  
  if( NI->Node == 0 )
  {
    //Make sure the same number of nodes saved is how many are being used now.
    fread( &nNodesSaved, sizeof(int), 1, in );

    if( nNodesSaved != NI->nNodes )
    {
      fprintf( stderr, "Error: node count mismatch. %d Nodes saved, %d Nodes currently\n", nNodesSaved, NI->nNodes );
      return;
    }

    //need to skip pass ARRAYS struct since it has already been loaded
    fread( &offset, sizeof( int ), 1, in );
    fseek( in, offset, SEEK_CUR );
  }
  
  //Node 0 needs to read in data - send to appropriate node, that node will write to temporary
  //and when all is done, will read from its temporary
  if( NI->Node == 0 )
  {
    printf( "Distributing save file contents\n" );
    fread( &offset, sizeof( unsigned int ), 1, in );
  
    //first node 0 - I could just skip forward, handle all other nodes, then skip back
    nbytes = ftell( in );
    
    fseek( in, offset, SEEK_CUR );
    for( i=1; i<NI->nNodes; i++ )
    {
      fread( &offset, sizeof( unsigned int ), 1, in );
      MPI_Send( &offset, 1, MPI_INT, i, tag, MPI_COMM_WORLD );
      
      for( amountRead=0; amountRead+dataBlockSize<offset; amountRead+=dataBlockSize )
      {
        fread( dataBlock, 1, dataBlockSize, in );
        MPI_Send( dataBlock, dataBlockSize, MPI_BYTE, i, tag, MPI_COMM_WORLD );
      }
      if( amountRead < offset )
      {
        fread( dataBlock, 1, offset-amountRead, in );
        MPI_Send( dataBlock, offset-amountRead, MPI_BYTE, i, tag, MPI_COMM_WORLD );
      }
    }
    
    //when node 0 is done, rewind to nbytes
    fseek( in, nbytes, SEEK_SET );
  }
  else //get ready to recv data
  {
    //tempOut should already be opened - as data comes in, write it out
    MPI_Recv( &offset, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &Status );
    
    for( amountRead=0; amountRead+dataBlockSize<offset; amountRead+=dataBlockSize )
    {
      MPI_Recv( dataBlock, dataBlockSize, MPI_BYTE, 0, tag, MPI_COMM_WORLD, &Status );
      fwrite( dataBlock, 1, dataBlockSize, tempOut );
    }
    if( amountRead < offset ) //still some data left
    {
      MPI_Recv( dataBlock, offset-amountRead, MPI_BYTE, 0, tag, MPI_COMM_WORLD, &Status );
      fwrite( dataBlock, 1, offset-amountRead, tempOut );
    }
    
    fclose( tempOut );
    in = fopen( fname, "r" );
  }
  
  //load Brain values using temporary files written to /tmp

  //individual Node Info
  printf( "Restore Previous Node Info\n" );
  NI->Load( in );
  
  i=0;
  fread( iholder, iSize, sizeof( int ), in );
  //CycleCount = iholder[i++];  //Do not overwrite CycleCount
  //TimeStep = iholder[i++];    //This will be reset when we get to the DoThink loop
  i++; i++;
  nCells = iholder[i++];
  nSynapseDef = iholder[i++];

  //fread( &TotalTime, 1, sizeof( double ), in ); //do not overwrite this value
  double falseTime;
  fread( &falseTime, 1, sizeof( double ), in ); //this will be destroyed when function ends

  printf( "Restore Synapse Definition\n" );
  SynapseDefs = (SynapseDef **) calloc ( sizeof(SynapseDef *), nSynapseDef );
  for( i=0; i<nSynapseDef; i++ )
  {
    SynapseDefs[i] = new SynapseDef;
    SynapseDefs[i]->Load( in, nothing );
  }

  //make sure TheBrain points to this - it already should
  TheBrain = this;

  //Load Cells
  printf( "Restore Cells\n" );
  Cells = (Cell **) calloc ( sizeof(Cell*), nCells );
  for( i=0; i<nCells; i++ )
  {
    Cells[i] = new Cell;
    Cells[i]->Load( in, nothing );
  }
  
  //connection information - for each node
  //for each node
    //read in # of connections to that node
    //if # > 0 read in data
    //if node is not this node - use mpi to get memory addresses
    //if it is this node - read in data directly
  unsigned int dataSize;
  int *synapseInfo;
  int cellIndex, cmpIndex, synIndex;
  //vector<Synapse *> collection;
  //queue<Synapse *> collection;
  deque<Synapse *> collection;
  //int *synapseInfo;
  Synapse **transferData;

  printf( "Restore synaptic connections\n" );
  for( int targetNode=0; targetNode<NI->nNodes; targetNode++ )
  {
    if( NI->Node == targetNode ) //This Node will send memory addresses
    {
      for( int askingNode=0; askingNode<NI->nNodes; askingNode++ )
      {
        if( askingNode == targetNode ) //don't ask myself with MPI - just do it directly
        {
          fread( &dataSize, sizeof( unsigned int ), 1, in );

          if( dataSize > 0 )
          {
            synapseInfo = new int[dataSize*3];
            fread( synapseInfo, sizeof( int ), dataSize*3, in );

            for( int targetSynapse=0; targetSynapse<dataSize; targetSynapse++ )
            {
              cellIndex = synapseInfo[targetSynapse*3];
              cmpIndex = synapseInfo[targetSynapse*3+1];
              synIndex = synapseInfo[targetSynapse*3+2];
              collection.push_back( Cells[cellIndex]->Compartments[cmpIndex]->SynapseList[synIndex] );
            }

            //have connection info in my vector - go through cells to disperse info
            for( int targetCell=0; targetCell<nCells; targetCell++ )
              Cells[targetCell]->LoadConnections( collection, targetNode );

            //make sure the queue has emptied
            if( !collection.empty() )
			  fprintf( stderr, "Error: not all connection data loaded was used by cells\n" );

            delete [] synapseInfo;
            synapseInfo = NULL;
          }
        }
        else //it will be answering a request
        {
          MPI_Recv( &dataSize, 1, MPI_INT, askingNode, tag, MPI_COMM_WORLD, &Status );

          if( dataSize > 0 )
          {
            synapseInfo = new int[ 3*dataSize ];

            MPI_Recv( synapseInfo, dataSize*3, MPI_INT, askingNode, tag, MPI_COMM_WORLD, &Status );

            //gather into collection
            transferData = new Synapse*[dataSize];
            for( int targetSynapse=0; targetSynapse<dataSize; targetSynapse++ )
            {
              cellIndex = synapseInfo[targetSynapse*3];
              cmpIndex = synapseInfo[targetSynapse*3+1];
              synIndex = synapseInfo[targetSynapse*3+2];

              transferData[targetSynapse] = Cells[cellIndex]->Compartments[cmpIndex]->SynapseList[synIndex];
            }
            MPI_Send( transferData, dataSize*sizeof(Synapse*), MPI_BYTE, askingNode, tag, MPI_COMM_WORLD );

            delete [] synapseInfo;
            delete [] transferData;

            synapseInfo = NULL; transferData = NULL;
          }
        }
      }
    }
    else //send a request for data
    {
      fread( &dataSize, sizeof(int), 1, in );

      MPI_Send( &dataSize, 1, MPI_INT, targetNode, tag, MPI_COMM_WORLD );

      if( dataSize > 0 )
      {
        synapseInfo = new int[ dataSize*3 ];
        fread( synapseInfo, sizeof( int ), dataSize*3, in );

        MPI_Send( synapseInfo, dataSize*3, MPI_INT, targetNode, tag, MPI_COMM_WORLD );

        //target Node will fulfill the request
        transferData = new Synapse*[dataSize];

        //MPI_Recv( &collection, dataSize*sizeof(Synapse*), MPI_BYTE, targetNode, tag, MPI_COMM_WORLD, &Status );
        MPI_Recv( transferData, dataSize*sizeof(Synapse*), MPI_BYTE, targetNode, tag, MPI_COMM_WORLD, &Status );

        //move data from transfer array to a STL queue
        for( int i=0; i<dataSize; i++ )
          collection.push_back( transferData[i] );

        //have connection info in my vector - go through cells to disperse info
        for( int targetCell=0; targetCell<nCells; targetCell++ )
          Cells[targetCell]->LoadConnections( collection, targetNode );

        //make sure the queue has emptied
        if( !collection.empty() )
		  fprintf( stderr, "Node %d Error: not all connection data was used from %d\n", NI->Node, targetNode );

        collection.clear();
        delete [] synapseInfo;
        delete [] transferData;

        synapseInfo = NULL; transferData = NULL;
      }
    }
  }

  if( NI->Node == 0 ) printf( "Done Loading\n" );
  
  fclose(in);
}


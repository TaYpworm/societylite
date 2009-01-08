/* This handles actually writing the report info to output.  Since all of a  */
/* cluster is now on a single node, it no longer needs to do collection via  */
/* MPI.  It could be replaced with file I/O in Report, but keeping it        */
/* separate makes it easier to handle e.g.  AIO output.                      */

/* The files are normally written to the user's file space on the head node. */
/* However, if the output file has a name beginning with /tmp, it will be    */
/* written to the node's local /tmp dir.  Note that some mechanism should be */
/* added to output node:filename info so they can be located...  JF          */

#include <string.h>
#include <unistd.h>

#include "Managers.h"
#include "Output.h"
#include "Port.h"
#include "debug.h"
#include "memstat.h"

#define MEM_KEY  KEY_OUTPUT

//#include "QQ.h"
//extern int QQStartOutput, QQFinishOutput;

Output::Output (T_REPORT *iRep, int NTHINGS, int THINGSIZE, char *ASCIIhdr)
{
  int id, fnlen;
  char *s;

  MEMADDOBJ (KEY_OUTPUT);

  FileName = iRep->FileName;
  Port     = iRep->Port;
  ASCII    = iRep->ASCII;
  File     = NULL;
  reportFlag  = iRep->reportFlag;

  nThings   = NTHINGS;
  ThingSize = THINGSIZE;
  nItems = 1;

/* Check whether this report is to be written to local node's /tmp dir */

  if (strncmp (iRep->FileName, "/tmp", 4) == 0)
  {
    IsTmp    = true;
    fnlen    = strlen (iRep->FileName) + 1;
    FileName = (char *) malloc (fnlen * sizeof (char));
    strcpy (FileName, iRep->FileName);
  }
  else
  {
    IsTmp    = false;
    fnlen    = strlen (NI->cwd) + strlen (NI->job) + strlen (iRep->FileName) + 3;
    FileName = (char *) malloc (fnlen * sizeof (char));
  
    strcpy (FileName, NI->cwd);          /* Create name for report, and open it */
    strcat (FileName, "/");
    if (NI->job != NULL)
    {
      strcat (FileName, NI->job);
      //strcat (FileName, ".");
      strcat (FileName, "-");  //Matlab sometimes requires a non-period
    }
    strcat (FileName, iRep->FileName);
  }

/* If the file already exists, delete it.  This allows the OpenFile routine  */
/* to always open it in append mode.                                         */

  remove (FileName);
  
/* Write report header info */

  if (!ASCII && !(Port>0 || Port == -1) )  //write header if this is not ascii
  {
    OpenFile ("w");

    if( reportFlag > 1 )  //binary header should output time start/end
                       //compartment info should be just the label
      s = "NCS 2.00\n";
    else
      s = "NCS 1.00\n";
    fwrite (s,          1,            strlen (s), File);
    fwrite (&nThings,   sizeof (int), 1, File);
    fwrite (&ThingSize, sizeof (int), 1, File);
    fwrite (ASCIIhdr,   1,            strlen (ASCIIhdr), File);

    //newer version binary files need to write out time start/end
    if( reportFlag > 1 )
    {
      //what if this is a dynamic report?
      //They are usually for streaming output, instead of file based.
      //So it might not matter
    }

    for (id = 0; id < nThings; id++)      /* Fake no longer used CellIDs */
      fwrite (&id, sizeof (int), 1, File);

#ifdef USE_AIO
    aio.aio_sigevent.sigev_notify = SIGEV_NONE;
    aio.aio_nbytes  = nThings * ThingSize;
    aio.aio_reqprio = 0;
#endif 
    CloseFile ();
  }
}

/*------------------------------------------------------------------------*/

Output::~Output ()
{
  MEMFREEOBJ (KEY_OUTPUT);
}

/*---------------------------------------------------------------------------*/
/* Open the report file when the report is at its first timestep             */

void Output::OpenFile (char *mode)
{
  char *baseName;

  baseName = NULL;
  if (Port > 0)
  {
    baseName = strstr (FileName, NI->job);     /* Check if 'job' added to FileName */
    if (baseName == NULL)
      File = OpenPortFile (FileName, Port, "w");
    else
    {
      baseName += strlen (NI->job) + 1;        /* If found, skip past it */
      File = OpenPortFile (baseName, Port, "w");
    }
  }
  else if ( Port == -1 )
  {
    //what is in FileName?
    File = getAutoPort();
  }
  else
  {
    File = fopen (FileName, mode);
  }

  if (File == NULL)
  {
    printf ("Output::OpenFile: error opening file '%s'\n", FileName);
    fprintf (stderr, "Output::OpenFile: error opening file '%s'\n", FileName);
    exit (-6);
  }

#ifdef USE_AIO
  if (!ASCII)
  {
    aio.aio_fildes = fileno (File);
    aio.aio_offset = ftell (File);
  }
#endif 
}

/*---------------------------------------------------------------------------*/
//Get a port value from the server application

FILE* Output::getAutoPort()
{
  char format[128];
  int binaryHeaderData[2];

  //contact the server - where is server info strored? In the NodeInfo objects
  if( !TheBrain->HostName ) //make sure a server name exist
    return NULL;

  int sd = OpenPort( TheBrain->HostName, TheBrain->HostPort );
  char *label = NULL;
  int length;

  if( sd <= 0 ) //error
    return NULL;
  
  //send command keyword - request
  write( sd, "request\n", 8 );

  //build name from Brain->L.name, Brain->job, Report->filename
  //make sure report filename doesn't have lead characters (job or path)
  char *baseName = NULL;
  if( (baseName = strstr( FileName, NI->job )) )
  {
    //have advanced passed path, advance past job and a separator (. or -)
    baseName += strlen( NI->job ) + 1;
  }
  else
    baseName = FileName;

  //will this report output binary or ascii?
  if( ASCII )
    strcpy( format, "ascii" );
  else
    strcpy( format, "constantbinary" );

  length = strlen( NI->AR->Brain->L.name ) + strlen( NI->job ) + strlen( baseName ) + 1;
  length += strlen( "write" )+1;
  length += strlen( format )+1;

  label = new char[ length+1 ];
  sprintf( label, "write\n%s\n%s%s%s\n", format, NI->AR->Brain->L.name, NI->job, baseName );
 
  //send name
  write( sd, label, length );

  if( !ASCII ) //send two byte header
  {
    binaryHeaderData[0] = htonl( nThings );
    binaryHeaderData[1] = htonl( ThingSize );
    write( sd, (char*) binaryHeaderData, sizeof( int )*2 );
  }

  if( label ) delete [] label;

  return fdopen( sd, "w" );
}

/*---------------------------------------------------------------------------*/
/* Close the report file                                                     */

void Output::CloseFile ()
{
#ifdef USE_AIO
  if (!ASCII && Port == 0 )
  {
    aio_fsync (O_DSYNC, &aio);
    aio.aio_fildes = -1;
  }
#endif 
  fclose (File);
  File = NULL;
}

/*---------------------------------------------------------------------------*/
/* This starts the process of output.  For stdio, just does an fwrite or     */
/* fprintf of data.  If AIO, etc, should start the output and return.        */

void Output::StartOutput (int TimeStep, void *data)
{
  float *fptr;
  int i;
  static int allocatedLength = 1024; //memory allocated for data to be put into
  int dataLength = 0; //amount of memory used
  const int tolerance = 64; //minimum number of characters that should be available at end of string
  static char *dataString = NULL; //for ascii data over a port
  char *dataHelper; //in case more memory is needed

//QQStateOn (QQStartOutput);
  if (ASCII)
  {
    if( Port > 0 || Port == -1 )
    {
      if( !dataString )
        dataString = new char[ allocatedLength ];
      dataLength = sprintf( dataString, "%d ", TimeStep );
      
      fptr = (float *) data;
      for( i=0; i<nThings*nItems; i++ )
      {
          if( dataLength + tolerance > allocatedLength )  //check if we need more memory
          {
            allocatedLength *= 2;
            dataHelper = new char[ allocatedLength ];
            strcpy( dataHelper, dataString );
            delete [] dataString;
            dataString = dataHelper;
          }

          dataLength += sprintf( &dataString[dataLength], "%.4f ", *fptr );
          fptr++;
      }
      
      //send data - do I want to send a new line? yes, until otherwise noted
      fprintf( File, "%d ", dataLength+1 );
      fprintf( File, "%s\n", dataString );
      fflush( File );

      //only allocated dataString once, but when to delete? When the Report object is destroyed?
    }
    else
    {
      if( reportFlag & HIDE_STEP )
      {
      }
      else
      {
        fprintf (File, "%d  ", TimeStep);
      }
      fptr = (float *) data;         /* For now, assume all data is float */

      for (i = 0; i < nThings*nItems; i++)
      {
        if( reportFlag & E_NOTATION )  //use e notation
          fprintf( File, "%.4e ", *fptr );
	else if( reportFlag & INTEGER_CAST)
	  fprintf( File, "%d ", (int)*fptr );	
        else 
          fprintf (File, "%.4f ", *fptr);
        fptr++;
      }
      fprintf (File, "\n");
    }
  }
  else
  {

    if( Port > 0 || Port == -1 )
    {
      //write out the size of data in ascii for the server program
        //this method could probabl be improved on the server's end
        //so that this information doesn't need to be sent over and over
      fprintf( File, "%d ", nThings * ThingSize );
      fwrite (data, ThingSize, nThings*nItems, File);
      fflush( File );
    }
    else
    {
#ifdef USE_AIO
      aio.aio_buf = data;
      aio_write (&aio);
#else
    fwrite (data, ThingSize, nThings*nItems, File);
#endif 
    }
  }
//QQStateOff (QQStartOutput);
}

/*---------------------------------------------------------------------------*/
/* Called to complete output.  For stdio funcs, a nop                        */

void Output::FinishOutput (void *Buffer)
{
#ifdef USE_AIO
  struct aiocb *list [1];

//QQStateOn (QQFinishOutput);
  list [0] = &aio;
  aio_suspend (list, 1, NULL);
  aio.aio_offset += aio.aio_nbytes;
//QQStateOn (QQFinishOutput);
#endif 

//printf ("Output::CompleteOutput:\n");
}

void Output::setItems( int nitems )
{
  if( nitems > 0 )
    nItems = nitems;
}


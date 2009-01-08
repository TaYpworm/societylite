#ifndef FILESTACK
#define FILESTACK

#include "file.h"

/**
 * Structure to keep multiple input files organized.
 * Every filename to be included is put onto the stack to wait for
 * the current file to finish.  A file can then be removed then parsed.
 * This will go on until the stack is empty.
 */
struct FileStack
{
  /**
   * pointer to array of INFILE objects
   */
  INFILE *vector;

  /**
   * the maximum size of the vector.
   */
  int vectorSize;

  /**
   * the index of the last item pushed onto the vector. -1 if the vector is empty.
   */
  int currentElement;

  /**
   * list of filenames to keep track of what files have already been read in.
   * when pushing on an element, this list will be consulted as to whether that file has already
   * been included
   */
   char **fileList;

  /**
   * the maximum size of the fileList
   */
  int listSize;

  /**
   * the index of the last file name pushed onto the fileList. -1 if the vector is empty.
   */
  int lastFile;
};

typedef struct FileStack FileStack;

/**
 * A FileStack structure must first be initialized before it can be used.
 * This allocates the vector and sets size to 10, current to -1;
 *
 * @param fs address of a FileStack object to be initialized
 */
void initFileStack( FileStack *fs )
{
  fs->vectorSize = 10;
  fs->listSize = 10;
  fs->currentElement = -1;
  fs->lastFile = -1;
  fs->fileList = (char **) calloc ( fs->listSize, sizeof( char * ) );
  fs->vector = (INFILE *) malloc ( sizeof( INFILE ) * fs->vectorSize );

  if( !fs->vector )
    fprintf( stderr, "Error: could not allocate FileStack vector\n" );
  if( !fs->fileList )
    fprintf( stderr, "Error: could not allocate FileStack fileList\n" );
}

/**
 * A FileStack structure must be destroyed when it's no longer needed.
 * This deallocates the vector.
 *
 * @param fs address of a FileStack object to be destroyed
 */
void destroyFileStack( FileStack *fs )
{
  int i;
  free( fs->vector );

  //fprintf( stderr, "free memory %d of %d\n", fs->lastFile, fs->listSize );
  for( i=0; i<=fs->lastFile; i++ )
    free( fs->fileList[i] );
  free( fs->fileList );

  fs->listSize = 0;
  fs->vectorSize = 0;
  fs->currentElement = -1;
  fs->lastFile = -1;
}

/**
 * Push a new element onto the FileStack.
 * If more memory is needed, it is allocated.
 * Check to make sure this file has not been seen before
 *
 * @param fs address of a FileStack object to be pushed onto
 * @param in address of a INFILE object to be inserted
 */
void pushFileStack( FileStack *fs, INFILE *in )
{
  int i;
  //check to make sure that the file is different

  for( i=0; i<=fs->lastFile; i++ )
  {
    if( strcmp( fs->fileList[i], in->file ) == 0 )
    {
      fprintf( stderr, "Warning: Include file %s already included, ignoring additional instance.\n", in->file );
      return;
    }
  }

  fs->currentElement++;

  //if there is not enough space allocate more first
  if(!( fs->currentElement < fs->vectorSize ))
  {
    fs->vectorSize = 2*fs->vectorSize;
    fs->vector = (INFILE*) realloc( fs->vector, sizeof( INFILE ) * fs->vectorSize );

    if( !fs->vector )
      fprintf( stderr, "Error: unable to reallocate FileStack vector\n" );
  }

  //copy over INFILE information
  fs->vector[fs->currentElement].from = in->from;
  fs->vector[fs->currentElement].line = in->line;
  fs->vector[fs->currentElement].col = in->col;
  fs->vector[fs->currentElement].file = strdup( in->file );
  fs->vector[fs->currentElement].parentFile = strdup( in->parentFile );

  //add the file name to the seen list
  if( !(fs->lastFile < fs->listSize ) )
  {
    fs->listSize *= 2;
    fs->fileList = realloc( fs->fileList, sizeof( char * ) * fs->listSize );

    if( !fs->fileList )
      fprintf( stderr, "Error: unable to reallocate FileStack fileList\n" );
  }

  fs->lastFile++;
  fs->fileList[fs->lastFile] = (char *) malloc ( sizeof( char) * ( 1+strlen( in->file) ) );
  strcpy( fs->fileList[i], in->file );
}

/**
 * Remove the last element of the FileStack and return it.
 *
 * @param fs address of a FileStack object to be pushed onto
 * @param in address of an INFILE object to put the information 
 */
INFILE* popFileStack( FileStack *fs, INFILE *in )
{
  //if stack is empty, return nothing
  if( fs->currentElement < 0 )
    return NULL;

  //do I need to allocate space for the copy? Will this situation even come up?
  if( in == NULL )
    in = (INFILE *) malloc (sizeof( INFILE ) );

  //copy over INFILE information
  in->from = fs->vector[fs->currentElement].from;
  in->line = fs->vector[fs->currentElement].line;
  in->col = fs->vector[fs->currentElement].col;
  in->file = strdup( fs->vector[fs->currentElement].file );
  in->parentFile = strdup( fs->vector[fs->currentElement].parentFile );

  //decrement
  fs->currentElement--;

  //free memory?
  //free( fs->vector[fs->currentElement].file );
  //free( fs->vector[fs->currentElement].parentFile );

  //return the pointer
  return in;
}

#endif

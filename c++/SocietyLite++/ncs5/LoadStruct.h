/*--------------------------------------------------------------------------
//  This file provides functions that will read in the INPUT structure
//  to a file for future use
//
//  This will allow a loaded brain to use the same names to access the 
//  structures such as columns, layers, cells, etc.
--------------------------------------------------------------------------*/


#include <stdio.h>
#include "InitStruct.h"
#include "parse/arrays.h"  //also contains InitStruct.h, but that's okay

/**@name LoadStruct
 * Functions for loading parsed data.  When the brain state is saved,
 * the basic information stored in the ARRAYS struct is saved first. This information
 * must then be resored first into a new ARRAYS object.  When these functions finish, 
 * the actual simulation objects will be able to retrieve data from the save file 
 * with their individual load functions
 */
//@{

/**
 * Write a dynamically allocated string to a file. Reads in
 * a 4 byte integer (the string length) followed by the characters
 * comprising the string.
 * @param string character string to be read
 * @param in FILE pointer to destination file
 * @return total number of bytes read.
 */
int loadString( char* &string, FILE *in );

/**
 * Alternate function for reading strings.  The same file format is used
 * (int containing number of characters, then the characters), but the
 * actual pointer is not sent.  Rather, the allocated string is returned
 * so that it can be assigned to a pointer instead.
 * @param in FILE pointer to file
 * @return the allocated string.
 */
char *loadString( FILE *in );


/**
 * Read in general information about an object. 
 * @param L Locator object to be read
 * @param in FILE pointer to destination file
 * @return total number of bytes read.
 */
int loadLocator( LOCATOR* &L, FILE *in );

/**
 * Read in linked list to file.  Since each list node uses
 * a Locator object, it calls loadLocator for each node as well
 * as saving additional information.
 * @param count number of list items to read in
 * @param List start of linked list
 * @param in FILE pointer to destination file
 * @return total number of bytes read.
 */
int loadList( int count, LIST* &List, FILE *in );

/**
 * Read in a connection (col->col, lay->lay, or cell->cell). Writes
 * in the names of the objects as well as the indices of where those
 * objects are.
 * @param iConnect Connection object being read.
 * @param in FILE pointer to destination file
 * @return total number of bytes read
 */
int loadConnect( T_CONNECT* &iConnect, FILE *in );

/**
 * Read in connection information for cmp->cmp. Writes in the
 * names of the compartments.
 * @param iConnect Connection object.
 * @param in FILE pointer to destination file
 * @return total number of bytes read
 */
int loadCmpConnect( T_CMPCONNECT* &iConnect, FILE *in );

/**
 * Writes in info contained in the brain object. Does not
 * load info that should be provided by when a new run is started.
 * i.e. Reports, Stimulus, real-time communication
 * @param iBrain Brain object to be read
 * @param in FILE pointer to destination file
 * @return total number of bytes read
 */
int loadTBrain( T_BRAIN* &iBrain, FILE *in );

/**
 * Read in column shell information.
 * @param iShell Column Shell object to be read
 * @param in FILE pointer to destination file
 * @return total number of bytes read
 */
int loadTCShell( T_CSHELL* &iShell, FILE *in );

/**
 * Read in column info to a file.
 * @param iColumn Column object to be read
 * @param in FILE pointer to destination file
 * @return total number of bytes read
 */
int loadTColumn( T_COLUMN* &iColumn, FILE *in );

/**
 * Read in a layer shell object.
 * @param iShell Layer Shell object to be read
 * @param in FILE pointer to destination file
 * @return total number of bytes read
 */
int loadTLShell( T_LSHELL* &iShell, FILE *in );

/**
 * Read in layer object.
 * @param iLayer Layer object to be read
 * @param in FILE pointer to destination file
 * @return total number of bytes read
 */
int loadTLayer( T_LAYER* &iLayer, FILE *in );

/**
 * Read in Cell object
 * @param iCell Cell object to be read
 * @param in FILE pointer to destination file
 * @return total number of bytes read
 */
int loadTCell( T_CELL* &iCell, FILE *in );

/**
 * load compartment object. Writes in many sets of mean/std. dev.
 * @param iCmp Compartment object to be read
 * @param in FILE pointer to destination file
 * @return total number of bytes read
 */
int loadTCmp( T_CMP* &iCmp, FILE *in );

/**
 * Read in a channel object to a file. Due to the volume of 
 * data in this object, I am trying to use memcpy to move
 * double information to a single array before writing.  If this
 * works, good; otherwise, I waill have to move data over using
 * more statements. I don't want to just call fwrite on the object
 * because if it changes in a later version, then old load files would
 * be unreadable. By writing in data as primitives, we can keep old
 * load functions that can still be used on old load files, while
 * writing new load functions to work with new load files.
 * @param in FILE pointer to destination file
 * @return total number of bytes read
 */
int loadTChannel( T_CHANNEL* &iChannel, FILE *in );

/**
 * @param in FILE pointer to destination file
 * @return total number of bytes read
 */
int loadTSynapse( T_SYNAPSE* &iSynapse, FILE *in );
int loadTSynLearn( T_SYNLEARN* &iLearn, FILE *in );
int loadTSynFD( T_SYN_FD* &iFD, FILE *in );

/**
 * @param iPsg pointer (passed by reference) where the object should be restored to.
 * @param 
 */
int loadTSynPSG( T_SYNPSG* &iPsg, FILE *in );
int loadTSynData( T_SYNDATA* &iData, FILE *in );
int loadTSpike( T_SPIKE* &iSpike, FILE *in );

//not going to load stimulus, stim_inject, or reports unless need to

/**
 * Function to load all the input structures gathered during parsing.
 * @param AR pointer to the arrays structure
 * @param in destination file
 * @return number of bytes read.
 */
int loadArrays( ARRAYS* &AR, FILE *in );

//@}

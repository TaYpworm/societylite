/*--------------------------------------------------------------------------
//  This file provides functions that will write out the INPUT structure
//  to a file for future use
//
//  This will allow a loaded brain to use the same names to access the 
//  structures such as columns, layers, cells, etc.
--------------------------------------------------------------------------*/


#include <stdio.h>
#include "InitStruct.h"
#include "parse/arrays.h"  //also contains InitStruct.h, but that's okay

/**@name SaveStruct
 * Functions for saving parsed data.  When the brain state is saved,
 * the basic information stored in the ARRAYS struct is saved first using the
 * following functions.  When these functions finish, then the actual instantiated
 * objects will be able to output to the save file with their individual save functions
 */
//@{

/**
 * Write a dynamically allocated string to a file. Writes out
 * a 4 byte integer (the string length) followed by the characters
 * comprising the string.
 * @param string character string to be written
 * @param out FILE pointer to destination file
 * @return total number of bytes written.
 */
int saveString( char *string, FILE *out );

/**
 * Write out general information about an object.
 * @param L Locator object to be written
 * @param out FILE pointer to destination file
 * @return total number of bytes written.
 */
int saveLocator( LOCATOR *L, FILE *out );

/**
 * Write out linked list to file.  Since each list node uses
 * a Locator object, it calls saveLocator for each node as well
 * as saving additional information.
 * @param List start of linked list
 * @param out FILE pointer to destination file
 * @return total number of bytes written.
 */
int saveList( LIST *List, FILE *out );

/**
 * Write out a connection (col->col, lay->lay, or cell->cell). Writes
 * out the names of the objects as well as the indices of where those
 * objects are.
 * @param iConnect Connection object being written.
 * @param out FILE pointer to destination file
 * @return total number of bytes written
 */
int saveConnect( T_CONNECT *iConnect, FILE *out );

/**
 * Write out connection information for cmp->cmp. Writes out the
 * names of the compartments.
 * @param iConnect Connection object.
 * @param out FILE pointer to destination file
 * @return total number of bytes written
 */
int saveCmpConnect( T_CMPCONNECT *iConnect, FILE *out );

/**
 * Writes out info contained in the brain object. Does not
 * save info that should be provided by when a new run is started.
 * i.e. Reports, Stimulus, real-time communication
 * @param iBrain Brain object to be written
 * @param out FILE pointer to destination file
 * @return total number of bytes written
 */
int saveTBrain( T_BRAIN *iBrain, FILE *out );

/**
 * Write out column shell information.
 * @param iShell Column Shell object to be written
 * @param out FILE pointer to destination file
 * @return total number of bytes written
 */
int saveTCShell( T_CSHELL *iShell, FILE *out );

/**
 * Write out column info to a file.
 * @param iColumn Column object to be written
 * @param out FILE pointer to destination file
 * @return total number of bytes written
 */
int saveTColumn( T_COLUMN *iColumn, FILE *out );

/**
 * Write out a layer shell object.
 * @param iShell Layer Shell object to be written
 * @param out FILE pointer to destination file
 * @return total number of bytes written
 */
int saveTLShell( T_LSHELL *iShell, FILE *out );

/**
 * Write out layer object.
 * @param iLayer Layer object to be written
 * @param out FILE pointer to destination file
 * @return total number of bytes written
 */
int saveTLayer( T_LAYER *iLayer, FILE *out );

/**
 * Write out Cell object
 * @param iCell Cell object to be written
 * @param out FILE pointer to destination file
 * @return total number of bytes written
 */
int saveTCell( T_CELL *iCell, FILE *out );

/**
 * Save compartment object. Writes out many sets of mean/std. dev.
 * @param iCmp Compartment object to be written
 * @param out FILE pointer to destination file
 * @return total number of bytes written
 */
int saveTCmp( T_CMP *iCmp, FILE *out );

/**
 * Write out a cahnnel object to a file. Due to the volume of 
 * data in this object, I am trying to use memcpy to move
 * double information to a single array before writing.  If this
 * works, good; otherwise, I waill have to move data over using
 * more statements. I don't want to just call fwrite on the object
 * because if it changes in a later version, then old save files would
 * be unreadable. By writing out data as primitives, we can keep old
 * load functions that can still be used on old save files, while
 * writing new load functions to work with new save files.
 * @param out FILE pointer to destination file
 * @return total number of bytes written
 */
int saveTChannel( T_CHANNEL *iChannel, FILE *out );

/**
 * @param out FILE pointer to destination file
 * @return total number of bytes written
 */
int saveTSynapse( T_SYNAPSE *iSynapse, FILE *out );
int saveTSynLearn( T_SYNLEARN *iLearn, FILE *out );
int saveTSynFD( T_SYN_FD *iFD, FILE *out );

/**
 * Write out PSG data to a file. I am currently writing out the entire
 * contents of the PSG array that were read in from a file. This way,
 * the saved brain does not need to hope that the original file that
 * the data came from still exists in the same location.
 * @param iPsg pointer to object to be written
 * @param out FILE pointer to destination file
 * @param total number of bytes written
 */
int saveTSynPSG( T_SYNPSG *iPsg, FILE *out );
int saveTSynData( T_SYNDATA *iData, FILE *out );
int saveTSpike( T_SPIKE *iSpike, FILE *out );

//not going to save stimulus, stim_inject, or reports unless need to

/**
 * Function to save all the input structures gathered during parsing.
 * @param AR pointer to the arrays structure
 * @param out destination file
 * @return number of bytes written.
 */
int saveArrays( ARRAYS *AR, FILE *out );

//@}

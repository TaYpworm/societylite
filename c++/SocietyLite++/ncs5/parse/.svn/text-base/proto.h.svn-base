/* Just prototypes for most of the parsing functions */

#ifndef PROTO_DEF
#define PROTO_DEF

#include <stdio.h>
#include "../InitStruct.h"
#include "input.h"
#include "tree.h"
#include "arrays.h"

void MakeTIN (int);

LIST *AddName (LIST *, char *, int);
LIST *AddCmp  (LIST *, char *, char *, double, double, double);

ARRAYS *ConvertIn (TREENODE *);

void ConvertCsh      (TREENODE *, ARRAYS *);
void ConvertCol      (TREENODE *, ARRAYS *);
void ConvertLsh      (TREENODE *, ARRAYS *);
void ConvertLay      (TREENODE *, ARRAYS *);
void ConvertCell     (TREENODE *, ARRAYS *);
void ConvertCmp      (TREENODE *, ARRAYS *);
void ConvertChan     (TREENODE *, ARRAYS *);
void ConvertSyn      (TREENODE *, ARRAYS *);
void ConvertSynFD    (TREENODE *, ARRAYS *);
void ConvertSynLearn (TREENODE *, ARRAYS *);
void ConvertSynData  (TREENODE *, ARRAYS *);
void ConvertSynPSG   (TREENODE *, ARRAYS *);
void ConvertSynAugmentation (TREENODE *SymTree, ARRAYS *AR);
void ConvertSpike    (TREENODE *, ARRAYS *);
void ConvertStim     (TREENODE *, ARRAYS *);
void ConvertSti      (TREENODE *, ARRAYS *);
void ConvertRpt      (TREENODE *, ARRAYS *);

int *ConvertList (LIST *, int, TREENODE *, char *);
void ConvertCellList (LIST *, int, TREENODE *, int *, int *);
void ConvertConnect (ARRAYS *, TREENODE *);

/**
 * For each recurrent connection made, make sure 
 * a regular connection exists for both directions (group A->B and
 * group B->A).  Once verified, combine the regular connections into
 * one T_CONNECT structure and disable the second T_CONNECT.  This
 * will allow the connection to be handled normally, but then extra steps
 * taken during Connector.cpp to create the second set of connections
 * simultaneosly.
 *
 * @param ARRAYS data structures parsed from the input file(s)
 */
void ConvertRecurrent (ARRAYS *AR, TREENODE *SymTree );

/**
 * Helper fuction for converting recurrent connections: used to find
 * connections that match the ones listed in the recurrent object.
 * Compares the column, layer, cell, compartment, and synapse indexes
 * to determine if they match or not.
 *
 * @param iCon The connections object to compare with
 * @param aIndex The indexes of the first cell group
 * @param bIndex The indexes of the second group
 * @param synapseIndex The index of the synapse
 * @return 1 if matched, 0 if not
 */
int matchCheck( T_CONNECT *iCon, int aIndex[4], int bIndex[4], int synapseIndex );

/**
 * Helper fuction for converting recurrent connections: Finds the proper
 * indexes of the column, layer, cell, and compartments in the ARRAYS
 * data structure.  Depending on the depth, the column and layer data
 * may not be applicable, so -1 is stored in their place.  Connections
 * between columns have depth 4, connections between layers have depth 3,
 * between cells depth 2.
 *
 * @param aIndex Array to store the first column, layer, cell, and compartment 
 * @param bIndex Array to store the second column, layer, cell, and compartment
 * @param synapseIndex Address of integer where the synapse can be stored
 * @param con The recurrent connection object
 * @param SymTree Symbol Tree/Table for searching for the structures
 * @param depth  The depth of the connection in the brain.
 */
void gatherRecurrentInfo( int aIndex[4], int bIndex[4], int *synapseIndex, T_RECURRENT *con, TREENODE *SymTree, int depth );

/**
 * Helper fuction for converting recurrent connections: If an error is detected
 * while converting a recurrent connection, this will output information
 * to the user to help indicate what problem occured.
 *
 * @param con The recurrent connection object
 * @param conAtoB One of the original connections objects or NULL if not found
 * @param conBtoA The other original connection object or NULL if not found
 */
void outputRecurrenceError( T_RECURRENT *con, T_CONNECT *conAtoB, T_CONNECT *conBtoA );

/**
 * Helper fuction for converting recurrent connections: This coordinates
 * the information between the two original connection structures so that
 * they have the recurrent information, and disables one of the connections
 * (If it is not a self-recurrence) so that the two connections form
 * simultaneously in Connector.cpp
 *
 * @param con The recurrent connection object
 * @param conAtoB One of the original connections objects
 * @param conBtoA The other original connection object
 */
void setupRecurrence( T_RECURRENT *con, T_CONNECT *conAtoB, T_CONNECT *conBtoA );


int getCmpIdx (char *, char *, TREENODE *, LOCATOR *);
int getName (LOCATOR *, char *, TREENODE *, char *);

void CellCount (ARRAYS *);

void dump (FILE *, ARRAYS *);
void PrintBrain (FILE *, ARRAYS *);
void PrintConnect (FILE *, ARRAYS *);

FILE *exist (char *);

#endif 

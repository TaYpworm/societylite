/* */
/* Defines structures for input of brain input elements.  Part of the parse */
/* module.  See comments at end of file                                     */

#ifndef ELEMENT_DEF
#define ELEMENT_DEF 1

/* Definition of STRLEN needed for parse.y and scan.l. */

#ifndef STRLEN
#define STRLEN 128
#endif

#include "list.h"

/*----------------------------------------------------------------------------
  This structure holds the data needed to specify the connections between brain 
  elements.  Lists of them will be included in individual element structures.

  There will be connections specified at brain, column, layer, and cell levels.
  Brain, column, and layer can use the same structure, with the higher-level
  names being implicit in the column & layer ones.  This makes later programming
  a good bit simpler, vs having different structures for each.

  The cell level is different: it specifies conductance instead of connection
  probability, and does not need a synapse.
------------------------------------------------------------------------------*/

typedef struct Connect_
{
  struct Connect_ *next;     /* Pointer to next in list */
  int line;                  /* line where defined, for error reporting */
  int index;                 /* The number of the element, starting from 0 */

  char *fromCol;
  char *fromLayer;
  char *fromCell;
  char *fromCmp;
  char *toCol;
  char *toLayer;
  char *toCell;
  char *toCmp;
  char *SynType;
  int speed;
  double prob;
} CONNECT;


typedef struct CmpConnect_
{
  struct CmpConnect_ *next;     /* Pointer to next in list */
  int line;                     /* line where defined, for error reporting */
  int index;                    /* The number of the element, starting from 0 */

  char *fromCmp;
  char *toCmp;
  int speed;
  double G;                   /* = Conductance */
} CMPCONNECT;


typedef struct CellCmp_       /* Structure holds definition of compartment within cell */
{
  int line;                   
  int index;              

  char *Name;        /* CmpName; */
  char *Label;       /* Name for compartment, used in connects */
  double x;
  double y;
} CELLCMP;


/*----------------------------------------------------------------------------
  These structures define the actual brain structure elements
------------------------------------------------------------------------------*/

/* This is the generic element, which contains all the elements which
   every element must share.  */

typedef struct Anyelement
{
  int kind;                  /* The type of element, from parse.h */
  int line;                  /* The input file line where the element is defined */
  int index;                 /* The number of the element, starting from 0 */
  struct Anyelement *next;   /* pointer to next element in list */
  char *name;                /* The name of the element, from TYPE statement */
} ANYELEMENT;

/* Following are structures for each type of  element */

typedef struct Brain
{
  int kind;               
  int line;               
  int index;             
  struct Brain *next;    
  char *name;            

  char *job;
  char *distribute;
  double duration;
  double fsv;
  int interactive;
  int ignore_empty;
  int seed;
  char *save_syn;
  char *ReloadSynFile;

  NAMELIST *st_inject;
  NAMELIST *column_type;
  NAMELIST *report;
  int nconnect;
  CONNECT *connect;
} BRAIN;


typedef struct Colshell
{
  int kind;               
  int line;               
  int index;             
  struct Colshell *next; 
  char *name;            

  double width;
  double height;
  double x;
  double y;
} COLSHELL;


typedef struct _Column
{
  int kind;               
  int line;               
  int index;             
  struct _Column *next;
  char *name;            

  char *shell;
  NAMELIST *layertype;
  int nconnect;
  CONNECT *connect;
} COLUMN;


typedef struct Layshell
{
  int kind;               
  int line;               
  int index;             
  struct Layshell *next; 
  char *name;            

  double upper;
  double lower;
} LAYSHELL;


typedef struct Layer
{
  int kind;               
  int line;               
  int index;             
  struct Layer *next;    
  char *name;            

  NAMELIST *LShells;
  CELLLIST *cell_type;
  int nconnect;
  CONNECT *connect;
} LAYER;


typedef struct Cell
{
  int kind;               
  int line;               
  int index;             
  struct Cell *next;     
  char *name;            

  int nCellCmp;              /* Number of compartments in cell */
/*TREENODE *CellCmp;  */
  char *CellCmp;             /* Cast to TREENODE * */
  int nconnect;
  CMPCONNECT *connect;
} CELL;


typedef struct Compart
{
  int kind;               
  int line;               
  int index;             
  struct Compart *next;             
  char *name;            

  int seed;
  char *Spike;
  double spike_halfwd [2];    /* These vars have two values.  The second may */
  double tau_membrane [2];    /* be a standard deviation */
  double r_membrane [2];
  double threshold [2];
  double leak_reversal [2];
  double leak_conduct [2];
  double vmrest [2];
  double ca_internal [2];
  double ca_external [2];
  double ca_spike_inc [2];
  double ca_tau [2];
  NAMELIST *channels;
} COMPART;


typedef struct Channel
{
  int kind;               
  int line;               
  int index;             
  struct Channel *next;  
  char *name;            

  int family;            /* Family values #defined above */
  int seed;
  double m_power;   
  double unitary_G;
  double strength;  
  double m_initial [2];
  double reversal [2];
} CHANNEL;


typedef struct ChanKm
{
  int kind;               
  int line;               
  int index;             
  struct Channel *next;  
  char *name;            

  int family;            
  int seed;
  int m_power;                /* double m_power; */
  double unitary_G;
  double strength;  
  double m_initial [2];
  double reversal [2];

  double ehalf;
  double tau_sf;
  double slopem [3];
} CHAN_Km;

typedef struct ChanKahp
{
  int kind;               
  int line;               
  int index;             
  struct Channel *next;  
  char *name;            

  int family;            
  int seed;
  double m_power;   
  double unitary_G;
  double strength;  
  double m_initial [2];
  double reversal [2];

  double ca_sf;
  double ca_ef;
  double ca_half_min;  
  double ca_tau_sf;
} CHAN_Kahp;

typedef struct ChanKa
{
  int kind;               
  int line;               
  int index;             
  struct Channel *next;  
  char *name;            

  int family;            
  int seed;
  double m_power;   
  double unitary_G;
  double strength;  
  double m_initial [2];
  double reversal [2];

  double h_initial [2];
  double h_power;
  double ehalf_m;
  double ehalf_h;
  double slope_m;
  double slope_h;

  int nValH;
  double *TauValH;
  int nVoltH;
  int *TauVoltH;

  int nValM;
  double *TauValM;
  int nVoltM;
  int *TauVoltM;
} CHAN_Ka;


typedef struct Synapse
{
  int kind;               
  int line;               
  int index;             
  struct Synapse *next;  
  char *name;            

  int seed;
  char *syn_fd; 
  char *psg;
  char *learn;
  char *data;
  double absolute_use [2];
} SYNAPSE;


typedef struct Syn_fd
{
  int kind;               
  int line;               
  int index;             
  struct Syn_fd *next;   
  char *name;           

  int seed;
  int SFD;
  double facil_tau [2];
  double depr_tau [2];
} SYN_FD;


typedef struct Syn_psg
{
  int kind;               
  int line;               
  int index;             
  struct Syn_psg *next;
  char *name;           

  char *file;
} SYN_PSG;


typedef struct Syn_learn
{
  int kind;               
  int line;               
  int index;              
  struct Syn_learn *next; 
  char *name;             

  int seed;
  int learning;
  double neg_heb_window [2];
  double pos_heb_window [2];
  double pos_heb_peak [2];
  double neg_heb_peak [2];
  double neg_heb_time [2];
  double pos_heb_time [2];
} SYN_LEARN;


typedef struct Syn_data
{
  int kind;               
  int line;               
  int index;              
  struct Syn_data *next;  
  char *name;             

  int seed;
  double max_conduct;
  double delay [2];
  double reversal [2];
} SYN_DATA;


typedef struct Spikeshape
{
  int kind;               
  int line;               
  int index;              
  struct Spikeshape *next;
  char *name;             

  int nval;
  double *values;
} SPIKESHAPE;


typedef struct Stimulus
{
  int kind;               
  int line;               
  int index;              
  struct Stimulus *next;  
  char *name;             

  char *file;
  int port;
  int mode;
  int pattern;
  int timing;
  int seed;
  int sameseed;
  int freq_rows;
  int cells_per_freq;
  double freq [2];
  double dyn_range [2];
  double amp_start;
  double amp_end;
  double width;
  double freq_start;
  double freq_end;
  int ntstart;
  double *tstart;
  int ntstop;
  double *tstop;
} STIMULUS;


typedef struct St_inject
{
  int kind;               
  int line;               
  int index;              
  struct St_inject *next; 
  char *name;             

  char *stim_type;      
  char *column;
  char *layer;
  char *cell;
  char *compart;
  double probability;
} ST_INJECT;


typedef struct Report
{
  int kind;               
  int line;               
  int index;              
  struct Report *next;    
  char *name;             

  char *column;
  char *layer;
  char *cell;
  char *cmp;
  char *synapse;
  char *file;
  int  ReportOn;
  int  ASCII;
  int  port;
  double prob;
  double freq;
  int ntstart;
  double *tstart;
  int ntstop;
  double *tstop;
} REPORT;


/* This structure is the root of the linked lists of input brain elements. */
/* It also holds some general information about the whole input            */

typedef struct
{
  BRAIN      *brain;
  COLSHELL   *csh;
  COLUMN     *column;
  LAYSHELL   *lsh;
  LAYER      *layer;
  COMPART    *cmp;
  CELL       *cell;
  CHANNEL    *channel;
  SYNAPSE    *synapse;
  SYN_FD     *syn_fd;
  SYN_LEARN  *syn_learn;
  SYN_DATA   *syn_data;
  SYN_PSG    *syn_psg;
  SPIKESHAPE *spike;
  STIMULUS   *stimulus;
  ST_INJECT  *st_inject;
  REPORT     *report;

  int nerrors;

  int nelements;       /* Count of each element type in input file */
  int nbrain;
  int ncsh;
  int ncolumn;
  int nlsh;
  int nlayer;
  int ncmp;
  int ncell;
  int nchannel;
  int nsynapse;
  int nsyn_fd;
  int nsyn_learn;
  int nsyn_data;
  int nsyn_psg;
  int nspike;
  int nstimulus;
  int nst_inject;
  int nreport;
} PARSEDINPUT;

/* function definitions */

BRAIN      *makebrain (int);
COLSHELL   *makecsh (int);
COLUMN     *makecolumn (int);
LAYSHELL   *makelsh (int);
LAYER      *makelayer (int);
COMPART    *makecmp (int);
CELL       *makecell (int);
CHAN_Km    *makechanKm  (int);
CHAN_Kahp  *makechanKahp (int);
CHAN_Ka    *makechanKa  (int);
SYNAPSE    *makesynapse (int);
SYN_FD     *makesyn_fd (int);
SYN_LEARN  *makesyn_learn (int);
SYN_DATA   *makesyn_data (int);
SYN_PSG    *makesyn_psg (int);
SPIKESHAPE *makespike (int);
ST_INJECT  *makesti (int);
STIMULUS   *makestim (int);
REPORT     *makereport (int);

int makeBrConn (BRAIN *, int, char *, char *, char *, char *,
                              char *, char *, char *, char *,
                              char *, double, int);

int makeColConn (COLUMN *, int line, char *, char *, char *,
                                     char *, char *, char *,
                                     char *, double, int);

int makeLayConn (LAYER *, int line, char *, char *,
                                    char *, char *, char *, double, int);

int makeCellConn (CELL *, int, char *, char *, int, double);

int makeCellCmp (CELL *, int, char *, char *, double, double);

char *TK2name (int);
#endif

/*-------------------------------------------------------------------------

These structures are used for initial input of brain structure definitions
from the input file.  As it scans through the input file, the parser creates 
a linked list of each type of brain element, allocating a new object for each 
new element it finds.  The structures here parallel those in InitStruct.h, 
but contain additional members for the links, and have string identifiers 
which will be converted to the numeric indexes used in the rest of the program.

As it scans through the input file, the parser creates a linked list of each 
type of brain element, allocating a new object for each new element it finds. 

To avoid conflicts, the element type is the code set in the #define statements
in parse.h (the header file output by yacc/bison).

-------------------------------------------------------------------------*/

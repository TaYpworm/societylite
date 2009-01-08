/* Here are most of the #defines used in the program.  They're moved into */
/* a single file to prevent errors from the same name being used in different */
/* contexts with different values.                                            */


#ifdef _IS_C
  int LearnCode (char *);
  int FamilyCode (char *);
  int ModeCode (char *);
  int PatternCode (char *);
  int TimingCode (char *);
  int ReportCode (char *);
  
  char *NameDefine (int);
#else
  extern "C" char *NameDefine (int);
#endif

void Abort (char *, int, int);

/* Misc. defines */

#ifndef _ISOC99_SOURCE
  #define _ISOC99_SOURCE
#endif

#include <math.h>
#define PI M_PI

#define EPSILON 0.0001

#define LSTR(L) (L ? "true" : "false")

#define MAXFNLEN  256      /* Max total length of a file name or path */
#define NAMELEN    32      /* Max length of a name in input (including trailing \0) */
#define HOSTLEN    32
#define MAXXFER    16      /* Max items transferred to create report */
#define INVALID   -1

#define FIREWIND 0.0025     /* The duration of the stimulus firing window, in seconds */

#define MAX_SYNDELAY 10     /* Maximum synapse delay, in case it's not calculated */

/* Names for message types - formerly in Message.h  */

#define MSG_STIMV         0xF0     /* sent by the stimulus object */
#define MSG_STIMC         0xF1     /* sent by the stimulus object */
#define MSG_SPIKE         0xF2     /* a spike / action potential */
#define MSG_STATE_SAVE    0xF3
#define MSG_STATE_RESET   0xF4
#define MSG_SYNC          0xFF     /* Sent at end of timestep */

/* Sync flag for message passing - or'd with message count */

#define SYNC_MASK     0x010000

/* Names for the channel families - formerly in ChannelDefine.h */

#define FAM_NA    0x0100
#define FAM_CA    0x0101
#define FAM_KM    0x0102
#define FAM_KA    0x0103
#define FAM_KDR   0x0104
#define FAM_KNICD 0x0105
#define FAM_KAHP  0x0106

/* Names for report types - formerly in ReportDefine.h */

#define NREPORTS 10                /* Number of different things that can be reported - not used anymore*/

#define VOLTAGE         0x0200     /* Shared with stimulus */
#define NET_CURRENT     0x0201
#define STIM_CURRENT    0x0202
#define SYN_CURRENT     0x0203
#define LEAK_CURRENT    0x0204
#define ADJ_CURRENT     0x0205
#define CHANNEL_CURRENT 0x0206
#define CHANNEL_KM      0x0207
#define CHANNEL_KA      0x0208
#define CHANNEL_KAHP    0x0209
#define SYNAPSE_USE     0x020A
#define CHANNEL_RPT     0x020B
#define SYNAPSE_RSE     0x020C
#define SYNAPSE_UF      0x020D
#define SYNAPSE_SA      0x020E
#define SYNAPSE_CA      0x020F
#define FIRE_COUNT      0x0210

/* More report constants */

#define SELECT_FRONT    0x01  //Indicates the report should select cells from front of list

/* Report options - or'd together */

#define E_NOTATION      0x01  //ASCII reports should use scientific notation
#define AVERAGE_SYN     0x02  //Synapse reports should average per parent cell
#define INTEGER_CAST    0x04  //Record the number of spikes in a timestep
#define HIDE_STEP       0x08  //Used to hide the timestep

/* Names for stimulus types - formerly in StimulusDefine.h */

#define MODE_CURRENT      0x0300
#define MODE_VOLTAGE      0x0301

/* #define PAT_LINEAR        0x0302
#define PAT_SINE          0x0303
#define PAT_ONGOING       0x0304
#define PAT_PULSE         0x0305
#define PAT_NOISE         0x0306
#define PAT_FILE          0x0307 */

#define TIMING_EXACT      0x0308
#define TIMING_URAND      0x0309
#define TIMING_POISSON    0x030A

/* More defines for stimulus constant values - from stimulus.h */

/*#define VOLTAGE     80  */      /* Already defined in report types */
#define CURRENT     0x0401
#define LINEAR      0x0403
#define SINE        0x0404
#define ONGOING     0x0405
#define PULSE       0x0406
#define NOISE       0x0407
#define EXACT       0x0408
#define URAND       0x0409
#define POISSON     0x040A
#define FILEBASED   0x040B
#define INPUTSTIM   0x040C

/* Defines for synapses - formerly in Synapse*.h */

#define RSE_FACIL   0x0501
#define RSE_DEPR    0x0502
#define RSE_BOTH    0x0503
#define RSE_NONE    0x0504

#define LEARN_POS   0x01
#define LEARN_NEG   0x02

/* Tags for MPI message types   */

#define TAG_PACKET    0x0900

#define TAG_DONE      0x0913
#define TAG_PIDINFO   0x0920
                      
/* Brain configuration flags */
/* These should only be powers of 2, since they will be bit flags */

#define USE_DISTANCE_FLAG        0x01
#define OUTPUT_CELLS_FLAG        0x02
#define OUTPUT_CONNECT_MAP_FLAG  0x04
#define WARNINGS_OFF_FLAG        0x08

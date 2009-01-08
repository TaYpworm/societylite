/* Header file for timing/statistics routines.  If the QQ_ENABLE compile     */
/* flag is not set, the routines are all defined as nops, so their effect    */
/* can be eliminated by a recompile.                                         */

/* Note that all "times" are in fact clock cycle counts, as returned by the  */
/* Intel RDTSC instruction.  Need to adjust for different processor speeds.  */

#ifdef QQ_ENABLE

#ifdef _QQSource

/* This defines structures used for timing/profiling */

#define TRUE  1
#define FALSE 0

#define MAXKEYS 1024
#define KEYLEN    32

#define QQ_STATE 1
#define QQ_MARK  2
#define QQ_COUNT 3
#define QQ_VALUE 4

#define QQ_OFF   0
#define QQ_ON    1

/* Assembly language definition of function (inline code?) for RDTSC         */
/* instruction.  Puts count into argument pointer as "long long".  This was  */
/* copied from the PCL source - I don't really understand it, but it works.  */

#define rdtsc(valptr) asm volatile ("rdtsc" : \
"=a"(*(unsigned int *)valptr), "=d"(*(((unsigned int *)valptr)+1)))

/* This is the basic event record */

typedef struct _QQItem
{
  int Key;                  /* Code for name of this item */
  union
  {
    int State;              /* 0 = off, 1 = on   */
    int Count;
    double Value;
  } Val;
  unsigned long long Time;      /* Time of event */
} QQItem;

/* This is a table of key-name pairs.  */

typedef struct _QQKey
{
  int Key;                  /* Code for name of this item */
  int Type;                 /* one of #defines above  */
  char *Name;
} QQKey;

/* This struct is just used to output the node entries.  */

typedef struct _QQNode
{
  size_t offset;                 /* Where data starts in output file */
  int nItems;                    /* Number of items for this node */
  unsigned long long TBase;      /* Base tick count on node */
  double MHz;                    /* Speed of processor */
} QQNode;

typedef struct _QQBase
{
  unsigned long long TBase;   /* Base time */
  int record;                 /* Record only if this flag is true */

  QQItem *table;              /* Array of QQItems */

  int nkeys;                  /* Index of next key to install */
  int keysize;                /* Size of key table */
  QQKey *keys;                /* Data for keys */
} QQBase;

#endif


void QQInit (int);
void QQBaseTime (int);
int QQAddState (char *);
int QQAddMark (char *);
int QQAddCount (char *);
int QQAddValue (char *);
static int QQAddName (char *, int);
void QQStateOn (int);
void QQStateOff (int);
void QQMark (int);
void QQCount (int, int);
void QQValue (int, double);
//void QQOut (char *, int);
void QQOut (char *, int, int);
void QQRecord (int);

#else
   #define QQInit(dummy)
   #define QQBaseTime(dummy)
   #define QQAddState(dummy) 0
   #define QQAddMark(dummy)  0
   #define QQAddCount(dummy) 0
   #define QQAddValue(dummy) 0
   #define QQAddName(dummy1, dummy2)
   #define QQStateOn(dummy)
   #define QQStateOff(dummy)
   #define QQMark(dummy)
   #define QQCount(dummy1, dummy2) 
   #define QQValue(dummy1, dummy2)
// #define QQOut(dummy1, dummy2)
   #define QQOut(dummy1, dummy21, dummy3)
   #define QQRecord(dummy)
#endif

/* These structures are used by the memory reporting functions.              */

/* This struct records the info about memory allocated to each key */

typedef struct _MEM
{
  int key;
  char *name;

  int size;         /* Size of the item (if an object) */
  int count;        /* Number created (if object) = calls to constructor */
  int ndelete;      /* Number deleted */
  int objMem;       /* Bytes currently allocated to this key by constructor */
  int objMax;       /* High mark of mem allocated to this key by constructor */

  int nalloc;       /* number of malloc calls associated with this item */
  int nfree;        /* number of free calls associated with this item */
  int alloc;        /* bytes assigned to this key by malloc calls */
  int memMax;       /* High mark ever allocated to this kind of item */

  int total;        /* total bytes associated with this key */
  int max;          /* high bytes associated with this key */
} MEMITEM;


/* This holds info about each allocation */

typedef struct _ALLOC
{
  int size;         /* Number of bytes allocated */
  int key;          /* MEM_KEY that allocated it */
} ALLOC;

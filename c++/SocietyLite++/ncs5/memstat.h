/* Macros to enable tracking of memory usage.  If the _MEMSTAT compile flag  */
/* is not set, they all reduce to nops, or to the *alloc cal they replace    */


#ifdef MEM_STATS

  void MemInit ();
  void MemAddObj (int, void *, int);
  void MemFreeObj (int, void *);

  void *MemMalloc (int, int);
  void *MemCalloc (int, int, int);
  void *MemRealloc (int, void *, int);
  void MemFree (void *);

  char *MemKeyName (int);
  void MemPrint ();
  
enum MemKeys {KEY_BRAIN,     KEY_BUILDBRAIN, KEY_CELL,         KEY_CELLMGR,     
              KEY_CHANNEL,   KEY_CMP,        KEY_CONNECT,      KEY_DISTRIBUTE, 
              KEY_DOREPORT,  KEY_DOSTIM,     KEY_FILE,         KEY_INPUT,        
              KEY_KILLFILE,  KEY_LEARNTABLE, KEY_MAIN,         KEY_MESSAGE,     
              KEY_MESSAGEBUS,KEY_MESSAGEMGR, KEY_PORT,         KEY_RANDOM,      
              KEY_RANDOMMGR, KEY_REPORT,     KEY_STIMULUS,     KEY_SYNAPSE,     
              KEY_OUTPUT,     KEY_OTHER};

  #ifndef MEM_SELF
    #define MEMINIT(dummy) MemInit ()
    #define MEMADDOBJ(key) MemAddObj (key, this, sizeof (*this))
    #define MEMFREEOBJ(key) MemFreeObj (key, this)
    #define MEMPRINT(dummy) MemPrint ()

    #define malloc(arg) MemMalloc (MEM_KEY, arg)
    #define calloc(arg1,arg2) MemCalloc (MEM_KEY, arg1, arg2)
    #define realloc(arg1,arg2) MemRealloc (MEM_KEY, arg1, arg2)
    #define free(arg1) MemFree (arg1)
  #endif

#else
  #define MEMINIT(dummy)
  #define MEMADDOBJ(name)
  #define MEMFREEOBJ(key)
  #define MEMPRINT(dummy) 
#endif


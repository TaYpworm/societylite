/* Just a prototype for the ReadFile functions */

#ifndef READFILE_DEF
#define READFILE_DEF

#ifdef __cplusplus
  extern "C" char *ReadFile (char *, int, int *);
  extern "C" double *ReadFPNFile (char *, int, int *, int);
#else
  char *ReadFile (char *, int, int *);
  double *ReadFPNFile (char *, int, int *, int);
#endif 

#endif 

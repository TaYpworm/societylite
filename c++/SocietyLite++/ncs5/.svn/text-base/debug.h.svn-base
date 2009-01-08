/* To turn off the debug print function, just comment out the lines below, */
/* and recompile                                                           */

void dprintf (char *, ...);

#define printf dprintf

/* These make it possible to keep a count of how many files are open.  See   */
/* File.cpp for details.  Not needed in production code.                     */

void GetMaxFiles ();

#ifdef FILE_COUNT
  #include <stdio.h>
  
  #define fopen(name, mode) Qfopen (name, mode, __FILE__, __LINE__)
  #define fclose(fp) Qfclose (fp, __FILE__, __LINE__)
  
  int NumOpenFiles ();
  FILE *Qfopen (char *, char *, char *, int);
  FILE *Qfdopen (char *, char *, char *, int);
  int Qfclose (FILE *, char *, int);
#endif

/* This structure is used to hold a record of all the files that are or have */
/* been opened for input (assuming that some sort of include mechanism is    */
/* implemented.  Each one holds the file name, line number, and (possibly)   */
/* column being read from, plus a pointer back to the file that included     */
/* this file.  The info needs to be kept at least until the end of input     */
/* processing, as elements will contain a pointer to the file struct for     */
/* error reporting.                                                          */

/* There will also be a global pointer to the currently-open file, to keep   */
/* from having to pass arguments around.                                     */

#ifndef INFILE_DEF
#define INFILE_DEF 1

typedef struct _Infile
{
  struct _infile *from;   /* Pointer to file that called this file */
  char *file;             /* Future: The file in which it was defined */
  char *parentFile;       /* The name of the file that included this one */
  int line;               /* The line of the file where it was defined */
  int col;                /* Only used for CELL_TYPE */
} INFILE;

#endif

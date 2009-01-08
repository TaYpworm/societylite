/* Returns a string containing the active compile options */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void GetCompileOpts ()
{
  char str [256];

  str [0] = '\0';
#ifdef SAME_PSC
  strcat (str, "  SAME_PSC");
#endif

#ifdef USE_AIO
  strcat (str, "  USE_AIO");
#endif

#ifdef QQ_ENABLE
  strcat (str, "  QQ_ENABLE");
#endif

#ifdef USE_FLOAT
  strcat (str, "  USE_FLOAT");
#endif

  printf ("Compile options are: %s\n", str);
}

/* Utility functions that return the #defined value from a string, or    */
/* vice versa                                                            */

#include "string.h"

#include "../defines.h"
#include "input.h"

extern TMP_INPUT *TIN;       

int LearnCode (char *str)
{
  int rc;

  rc = 0;
  if (strcasecmp (str, "+HEBBIAN") == 0)
    rc = LEARN_POS;
  else if (strcasecmp (str, "-HEBBIAN") == 0)
    rc = LEARN_NEG;
  else if (strcasecmp (str, "BOTH") == 0)
    rc = LEARN_POS | LEARN_NEG;
  else if (strcasecmp (str, "NONE") != 0)
  {
    printerr ("%s:%d: Invalid Learning type '%s'\n", TIN->file, TIN->line, str);
    rc = INVALID;
    TIN->nParseErr++;
  }
  return (rc);
}

/*----------------------------------------------------------------------------*/

int FamilyCode (char *str)
{
  int rc;

  if (strcasecmp (str, "Na") == 0)
    rc = FAM_NA;
  else if (strcasecmp (str, "Ca") == 0)
    rc = FAM_CA;
  else if (strcasecmp (str, "Km") == 0)
    rc = FAM_KM;
  else if (strcasecmp (str, "Ka") == 0)
    rc = FAM_KA;
  else if (strcasecmp (str, "Kdr") == 0)
    rc = FAM_KDR;
  else if (strcasecmp (str, "Knicd") == 0)
    rc = FAM_KNICD;
  else if (strcasecmp (str, "Kahp") == 0)
    rc = FAM_KAHP;
  else 
  {
    printerr ("%s:%d: Invalid family type '%s'\n", TIN->file, TIN->line, str);
    rc = INVALID;
    TIN->nParseErr++;
  }

  return (rc);
}

/*----------------------------------------------------------------------------*/

int ModeCode (char *str)
{
  int rc;

  if (strcasecmp (str, "CURRENT") == 0)
    rc = MODE_CURRENT;
  else if (strcasecmp (str, "VOLTAGE") == 0)
    rc = MODE_VOLTAGE;
  else 
  {
    printerr ("%s:%d: Invalid mode type '%s'\n", TIN->file, TIN->line, str);
    rc = INVALID;
    TIN->nParseErr++;
  }

  return (rc);
}

/*----------------------------------------------------------------------------*/

int SFDCode (char *str)
{
  int rc;

  if (strcasecmp (str, "FACIL") == 0)
    rc = RSE_FACIL;
  else if (strcasecmp (str, "DEPR") == 0)
    rc = RSE_DEPR;
  else if (strcasecmp (str, "BOTH") == 0)
    rc = RSE_BOTH;
  else if (strcasecmp (str, "NONE") == 0)
    rc = RSE_NONE;
  else 
  {
    printerr ("%s:%d: Invalid RSE (SFD?) type '%s'\n", TIN->file, TIN->line, str);
    rc = INVALID;
    TIN->nParseErr++;
  }

  return (rc);
}

/*----------------------------------------------------------------------------*/

int PatternCode (char *str)
{
  int rc;

  if (strcasecmp (str, "LINEAR") == 0)
    rc = LINEAR;
  else if (strcasecmp (str, "SINE") == 0)
    rc = SINE;
  else if (strcasecmp (str, "ONGOING") == 0)
    rc = ONGOING;
  else if (strcasecmp (str, "PULSE") == 0)
    rc = PULSE;
  else if (strcasecmp (str, "NOISE") == 0)
    rc = NOISE;
  else if (strcasecmp (str, "FILE_BASED") == 0)
    rc = FILEBASED;
  else if (strcasecmp (str, "INPUT") == 0)
    rc = INPUTSTIM;
  else 
  {
    printerr ("%s:%d: Invalid stimulus pattern type '%s'\n", TIN->file, TIN->line, str);
    rc = INVALID;
    TIN->nParseErr++;
  }

  return (rc);
}

/*----------------------------------------------------------------------------*/

int TimingCode (char *str)
{
  int rc;

  if (strcasecmp (str, "EXACT") == 0)
    rc = TIMING_EXACT;
  else if (strcasecmp (str, "RANDOM") == 0)
    rc = TIMING_URAND;
  else if (strcasecmp (str, "POISSON") == 0)
    rc = TIMING_POISSON;
  else 
  {
    printerr ("%s:%d: Invalid timing type '%s'\n", TIN->file, TIN->line, str);
    rc = INVALID;
    TIN->nParseErr++;
  }

  return (rc);
}

/*----------------------------------------------------------------------------*/

int ReportCode (char *str)
{
  int rc;

  if (strcasecmp (str, "VOLTAGE") == 0)
    rc = VOLTAGE;
  else if (strcasecmp (str, "NET_CURRENT") == 0)
    rc = NET_CURRENT;
  else if (strcasecmp (str, "STIM_CURRENT") == 0)
    rc = STIM_CURRENT;
  else if (strcasecmp (str, "SYN_CURRENT") == 0)
    rc = SYN_CURRENT;
  else if (strcasecmp (str, "LEAK_CURRENT") == 0)
    rc = LEAK_CURRENT;
  else if (strcasecmp (str, "ADJ_CURRENT") == 0)
    rc = ADJ_CURRENT;
  else if (strcasecmp (str, "CHANNEL_CURRENT") == 0)
    rc = CHANNEL_CURRENT;
/*else if (strcasecmp (str, "CHANNEL_Km") == 0)
    rc = CHANNEL_KM;
  else if (strcasecmp (str, "CHANNEL_Ka") == 0)
    rc = CHANNEL_KA;
  else if (strcasecmp (str, "CHANNEL_Kahp") == 0)
    rc = CHANNEL_KAHP;                           */
  else if (strcasecmp (str, "CHANNEL_RPT") == 0)
    rc = CHANNEL_RPT;
  else if (strcasecmp (str, "SYNAPSE_USE") == 0)
    rc = SYNAPSE_USE;
  else if (strcasecmp (str, "SYNAPSE_RSE") == 0)
    rc = SYNAPSE_RSE;
  else if (strcasecmp (str, "SYNAPSE_UF") == 0)
    rc = SYNAPSE_UF;
  else if (strcasecmp (str, "SYNAPSE_SA") == 0)
    rc = SYNAPSE_SA;
  else if (strcasecmp (str, "SYNAPSE_CA") == 0)
    rc = SYNAPSE_CA;
  else if (strcasecmp (str, "FIRE_COUNT") == 0)
    rc = FIRE_COUNT;
  else 
  {
    printerr ("%s:%d: Invalid report type '%s'\n", TIN->file, TIN->line, str);
    rc = INVALID;
    TIN->nParseErr++;
  }

  return (rc);
}

/**
 * Used to set flags for the brain object.
 * @param currentState has all flags that have already been set
 * @token token string to determine which flag needs to be turned on
 * @return The final value with the new flag set to on
 */
int setFlag( int currentState, char *token )
{
    int final = currentState;
    
    //look at token, and set appropriate flag in return
    if( strcasecmp( token, "OUTPUT_CELLS" ) == 0 )
        final |= OUTPUT_CELLS_FLAG;
    else if( strcasecmp( token, "USE_DISTANCE" ) == 0 )
        final |= USE_DISTANCE_FLAG;
    else if( strcasecmp( token, "OUTPUT_CONNECT_MAP" ) == 0 )
        final |= OUTPUT_CONNECT_MAP_FLAG;
    else if( strcasecmp( token, "WARNINGS_OFF" ) == 0 )
        final |= WARNINGS_OFF_FLAG;
    
    return final;
}   

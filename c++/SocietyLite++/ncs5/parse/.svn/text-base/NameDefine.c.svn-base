/* This function takes one of the #define'd constants in defines.h, and  */
/* returns the corresponding string.  Mainly for debugging               */

#include "../defines.h"

/*----------------------------------------------------------------------------*/

char *NameDefine (int code)
{
  int prefix;
  char *str;

  prefix = (code & 0xFF00) >> 8;

  switch (prefix)
  {
    case 0x00:  switch (code)             /* Messages */
                {
                  case MSG_STIMV:         str = "MSG_STIMV";
                      break;
                  case MSG_STIMC:         str = "MSG_STIMC";
                      break;
                  case MSG_SPIKE:         str = "MSG_SPIKE";
                      break;
                  case MSG_STATE_SAVE:    str = "MSG_STATE_SAVE";
                      break;
                  case MSG_STATE_RESET:   str = "MSG_STATE_RESET";
                      break;
                  default:                str = "UNKNOWN MSG TYPE";
                }
        break;
    case 0x01: switch (code)      /* Channel */
               {
                 case FAM_NA:     str = "Na";
                     break;
                 case FAM_CA:     str = "Ca";
                     break;
                 case FAM_KM:     str = "Km";
                     break;
                 case FAM_KA:     str = "Ka";
                     break;
                 case FAM_KDR:    str = "Kdr";
                     break;
                 case FAM_KNICD:  str = "Knicd";
                     break;
                 case FAM_KAHP:   str = "Kahp";
                     break;
                 default:         str = "UNKNOWN CHANNEL TYPE";
               }
        break;
    case 0x02: switch (code)             /* Report */
               {
                 case VOLTAGE:           str = "VOLTAGE";
                     break;
                 case NET_CURRENT:       str = "NET_CURRENT";
                     break;
                 case STIM_CURRENT:      str = "STIM_CURRENT";
                     break;
                 case SYN_CURRENT:       str = "SYN_CURRENT";
                     break;
                 case LEAK_CURRENT:      str = "LEAK_CURRENT";
                     break;
                 case ADJ_CURRENT:       str = "ADJ_CURRENT";
                     break;
                 case CHANNEL_CURRENT:   str = "CHANNEL_CURRENT";
                     break;
/*               case CHANNEL_KM:        str = "CHANNEL_Km";
                     break;
                 case CHANNEL_KA:        str = "CHANNEL_Ka";
                     break;
                 case CHANNEL_KAHP:      str = "CHANNEL_Kahp";
                     break;                                    */
                 case SYNAPSE_USE:       str = "SYNAPSE_USE";
                     break;
                 case CHANNEL_RPT:       str = "CHANNEL_RPT";
                     break;
                 default:                str = "UNKNOWN REPORT TYPE";
               }
        break;
    case 0x03: switch (code)             /* Stimulus */
               {                         
                 case MODE_CURRENT:      str = "MODE_CURRENT";
                     break;
                 case MODE_VOLTAGE:      str = "MODE_VOLTAGE";
                     break;
                 case TIMING_EXACT:      str = "TIMING_EXACT";
                     break;
                 case TIMING_URAND:      str = "TIMING_URAND";
                     break;
                 case TIMING_POISSON:    str = "TIMING_POISSON";
                     break;
                 default:                str = "UNKNOWN STIM CODE";
               }
        break;
    case 0x04: switch (code)      /* Stimulus */
               {
                 case CURRENT:    str = "CURRENT";
                     break;
                 case LINEAR:     str = "LINEAR";
                     break;
                 case SINE:       str = "SINE";
                     break;
                 case ONGOING:    str = "ONGOING";
                     break;
                 case PULSE:      str = "PULSE";
                     break;
                 case NOISE:      str = "NOISE";
                     break;
                 case EXACT:      str = "EXACT";
                     break;
                 case URAND:      str = "URAND";
                     break;
                 case POISSON:    str = "POISSON";
                     break;
                 case FILEBASED:  str = "FILEBASED";
                     break;
                 case INPUTSTIM:  str = "INPUT";
                     break;
                 default:         str = "UNKNOWN STIM TYPE";
               }
        break;
    case 0x05: switch (code)      /* Synapse RSE */
               {
                 case RSE_FACIL:  str = "RSE_FACIL";
                     break;
                 case RSE_DEPR:   str = "RSE_DEPR";
                     break;
                 case RSE_BOTH:   str = "RSE_BOTH";
                     break;
                 case RSE_NONE:   str = "RSE_NONE";
                     break;
                 default:         str = "UNKNOWN RSE TYPE";
               }
        break;
    case 0x06: switch (code)         /* Synapse Learning */
               {
                 case 0:    str = "LEARN_NONE";
                     break;
                 case 1:    str = "LEARN_POS";
                     break;
                 case 2:    str = "LEARN_NEG";
                     break;
                 case 3:    str = "LEARN_BOTH";
                     break;
                 default:            str = "UNKNOWN LEARN TYPE";
               }
        break;
    case 0x09:  switch (code)
                {
                  case TAG_PACKET:    str = "TAG_PACKET";
                      break;
                  case TAG_DONE:      str = "TAG_DONE";
                      break;
                  case TAG_PIDINFO:   str = "TAG_PIDINFO";
                      break;
                  default:            str = "UNKNOWN TAG";
                }
        break;
                                         
    default:   str = "UNKNOWN";
  }
  return (str);
}

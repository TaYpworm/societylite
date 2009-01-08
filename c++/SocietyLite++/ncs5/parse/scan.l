%{
  #include "input.h"
  #include "parse.h"

  extern TMP_INPUT *TIN;       /* This must be global so parse.y can access it */
%}

DIGIT    [0-9]
NAME     [a-zA-Z0-9_+\-\.>/]+
E        [Ee][+-]?{DIGIT}+

%%

#.*\n               { TIN->line++; }      /* This is a comment */
                      

\-?{DIGIT}+            { yylval.ival = atoi (yytext); return (INTEGER); }

\-?{DIGIT}+"."{DIGIT}*({E})? { yylval.rval = atof (yytext); return (REAL); }    /* catches 1. */
\-?{DIGIT}*"."{DIGIT}+({E})? { yylval.rval = atof (yytext); return (REAL); }    /* catches .01 */
\-?{DIGIT}+{E}         { yylval.rval = atof (yytext); return (REAL); }

ABSOLUTE_USE           { return (TK_ABSOLUTE_USE); }
ALPHA                  { return (TK_ALPHA); }
ALPHA_SCALE_FACTOR_H   { return (TK_ALPHA_SCALE_H); }
ALPHA_SCALE_FACTOR_M   { return (TK_ALPHA_SCALE_M); }
AMP_END                { return (TK_AMP_END); }
AMP_START              { return (TK_AMP_START); }
ASCII                  { return (TK_ASCII); }
AUTO                   { return (TK_AUTO); }
AUGMENTATION_DELAY     { return (TK_AUGMENTATION_DELAY); }
AUGMENTATION_INIT      { return (TK_AUGMENTATION_INIT); }
AUGMENTATION_TAU       { return (TK_AUGMENTATION_TAU); }
AVERAGE_SYN            { return (TK_AVERAGE_SYN); }
BETA_SCALE_FACTOR_H    { return (TK_BETA_SCALE_H); }
BETA_SCALE_FACTOR_M    { return (TK_BETA_SCALE_M); }
BRAIN                  { return (TK_BRAIN); }
CA_EXP_FACTOR          { return (TK_CA_EXP); }
CA_EXP_GAUSS           { return (TK_CA_EXP); }
CA_EXP_RANGE           { return (TK_CA_EXP_RANGE); }
CA_EXTERNAL            { return (TK_CA_EXTERNAL); }
CA_HALF_MIN            { return (TK_CA_HALF_MIN); }
CA_INTERNAL            { return (TK_CA_INTERNAL); }
CA_SCALE_FACTOR        { return (TK_CA_SCALE); }
CA_SPIKE_INCREMENT     { return (TK_CA_SPIKE_INC ); }
CA_TAU                 { return (TK_CA_TAU); }
CA_TAU_DECAY           { return (TK_CA_TAU_DECAY); }
CA_TAU_SCALE_FACTOR    { return (TK_CA_TAU_SCALE); }
CELL                   { return (TK_CELL); }
CELLS                  { return (TK_CELLS); }
CELL_TYPE              { return (TK_CELL_TYPE); }
CHANNEL                { return (TK_CHANNEL); }
COLUMN                 { return (TK_COLUMN); }
COLUMN_SHELL           { return (TK_CSHELL); }
COLUMN_TYPE            { return (TK_COLUMN_TYPE); }
COMPARTMENT            { return (TK_CMP); }
MAX_CONDUCT            { return (TK_MAX_G); }
CONNECT                { return (TK_CONNECT); }
DATA_LABEL             { return (TK_DATA_LABEL); }
DELAY                  { return (TK_DELAY); }
DEPR_TAU               { return (TK_DEPR_TAU); }
DISTANCE               { return (TK_DISTANCE); }
DURATION               { return (TK_DURATION); }
END_BRAIN              { return (TK_END_BRAIN); }
END_CELL               { return (TK_END_CELL); }
END_CHANNEL            { return (TK_END_CHANNEL); }
END_COLUMN             { return (TK_END_COLUMN); }
END_COLUMN_SHELL       { return (TK_END_CSHELL); }
END_COMPARTMENT        { return (TK_END_CMP); }
END_LAYER              { return (TK_END_LAYER); }
END_LAYER_SHELL        { return (TK_END_LSHELL); }
END_REPORT             { return (TK_END_REPORT); }
END_SPIKESHAPE         { return (TK_END_SPIKE); }
END_STIMULUS           { return (TK_END_STIMULUS); }
END_STIMULUS_INJECT    { return (TK_END_ST_INJECT); }
END_SYNAPSE            { return (TK_END_SYNAPSE); }
END_SYN_AUGMENTATION   { return (TK_END_SYN_AUGMENTATION); }
END_SYN_DATA           { return (TK_END_SYN_DATA); }
END_SYN_FACIL_DEPRESS  { return (TK_END_SYN_FD); }
END_SYN_LEARNING       { return (TK_END_SYN_LEARN); }
END_SYN_PSG            { return (TK_END_SYN_PSG); }
EXP                    { return (TK_EXP); }
E_HALF_MIN_H           { return (TK_E_HALF_MIN_H); }
E_HALF_MIN_M           { return (TK_E_HALF_MIN_M); }
FACIL_TAU              { return (TK_FACIL_TAU); }
FILENAME               { return (TK_FILENAME); }
FREQUENCY              { return (TK_FREQUENCY); }
FREQ_ROWS              { return (TK_FREQ_ROWS); }
FREQ_START             { return (TK_FREQ_START); }
FSV                    { return (TK_FSV); }
HEIGHT                 { return (TK_HEIGHT); }
HIDE_TIMESTEP          { return (TK_HIDE_TIMESTEP); }
H_INITIAL              { return (TK_H_INITIAL); }
H_POWER                { return (TK_H_POWER); }
IGNORE_EMPTY           { return (TK_IGNORE_EMPTY); }
INCLUDE                { strncpy (yylval.sval, yytext, STRLEN-1); return (TK_INCLUDE); }
INJECT                 { return (TK_INJECT); }
INTERACTIVE            { return (TK_INTERACTIVE); }
LAYER                  { return (TK_LAYER); }
LAYER_SHELL            { return (TK_LSHELL); }
LAYER_TYPE             { return (TK_LAYER_TYPE); }
LEAK_CONDUCTANCE       { return (TK_LEAK_G ); }
LEAK_REVERSAL          { return (TK_LEAK_REVERSAL); }
LEARNING               { return (TK_LEARN); }
LEARN_LABEL            { return (TK_LEARN_LABEL); }
LOAD                   { return (TK_LOAD); };
LOCATION               { return (TK_LOCATION); }
LOWER                  { return (TK_LOWER); }
MAX_AUGMENTATION       { return (TK_MAX_AUGMENTATION); }
MAX_G                  { return (TK_MAX_G); }
MODE                   { return (TK_MODE); }
M_INITIAL              { return (TK_M_INITIAL); }
M_POWER                { return (TK_M_POWER); }
NEG_HEB_PEAK_DELTA_USE { return (TK_NEG_HEB_PEAK_DELTA_USE); }
NEG_HEB_PEAK_TIME      { return (TK_NEG_HEB_PEAK_TIME); }
NEG_HEB_WINDOW         { return (TK_NEG_HEB_WINDOW); }
OUTPUT_CELLS           { return (TK_OUTPUT_CELLS); }
OUTPUT_CONNECT_MAP     { return (TK_OUTPUT_CONNECT_MAP); }
OPTION                 { return (TK_OPTION); }
PATTERN                { return (TK_PATTERN); }
PHASE_SHIFT            { return (TK_PHASE_SHIFT); }
POS_HEB_PEAK_DELTA_USE { return (TK_POS_HEB_PEAK_DELTA_USE); }
POS_HEB_PEAK_TIME      { return (TK_POS_HEB_PEAK_TIME); }
POS_HEB_WINDOW         { return (TK_POS_HEB_WINDOW); }
PREV_SPIKE_RANGE       { return (TK_PREV_SPIKE_RANGE); }
PROB                   { return (TK_PROB); }
RECURRENT_CONNECT      { return (TK_RECURRENT_CONNECT); }
RELOAD_SYN             { return (TK_RELOAD_SYN); }
REPORT                 { return (TK_REPORT); }
REPORT_ON              { return (TK_REPORT_ON); }
REVERSAL_POTENTIAL     { return (TK_REVERSAL); }
RSE                    { return (TK_RSE); }
RSE_LABEL              { return (TK_RSE_LABEL); }
RSE_INIT               { return (TK_RSE_INIT); }
R_MEMBRANE             { return (TK_R_MEMBRANE); }
SAMESEED               { return (TK_SAMESEED); }
SAVE                   { return (TK_SAVE); }
SAVE_SYN               { return (TK_SAVE_SYN); }
SEED                   { return (TK_SEED); }
SINGLE                 { return (TK_SINGLE); }
SELECT_FRONT           { return (TK_SELECT_FRONT); }
SERVER                 { return (TK_SERVER); }
SERVER_PORT            { return (TK_SERVER_PORT); }
SLOPE_FACTOR_H         { return (TK_SLOPE_H); }
SLOPE_FACTOR_H_STDEV   { return (TK_SLOPE_H_STDEV); }
SLOPE_FACTOR_M         { return (TK_SLOPE_M); }
SLOPE_FACTOR_M_STDEV   { return (TK_SLOPE_M_STDEV); }
SPIKESHAPE             { return (TK_SPIKE); }
SPIKE_HALFWIDTH        { return (TK_SPIKE_HW); }
STIMULUS               { return (TK_STIMULUS); }
STIMULUS_INJECT        { return (TK_ST_INJECT); }
STIM_TYPE              { return (TK_STIM_TYPE); }
STRENGTH               { return (TK_STRENGTH); }
STRENGTH_GAUSS         { return (TK_STRENGTH); }
STRENGTH_RANGE         { return (TK_STRENGTH_RANGE); }
SYNAPSE                { return (TK_SYNAPSE); }
SYNAPSE_RSE            { return (TK_SYNAPSE_RSE); }
SYNAPSE_UF             { return (TK_SYNAPSE_UF); }
SYN_AUGMENTATION       { return (TK_SYN_AUGMENTATION); }
SYN_CALCIUM            { return (TK_SYN_CALCIUM); }
SYN_DATA               { return (TK_SYN_DATA); }
SYN_FACIL_DEPRESS      { return (TK_SYN_FD); }
SYN_LEARNING           { return (TK_SYN_LEARN); }
SYN_PSG                { return (TK_SYN_PSG); }
SYN_REVERSAL           { return (TK_SYN_REVERSAL); }
TAU_MEMBRANE           { return (TK_TAU_MEMBRANE); }
TAU_SCALE_FACTOR_H     { return (TK_TAU_SCALE_H); }
TAU_SCALE_FACTOR_M     { return (TK_TAU_SCALE_M); }
THRESHOLD              { return (TK_THRESHOLD); }
TIME_END               { return (TK_TIME_END); }
TIME_START             { return (TK_TIME_START); }
TIMING                 { return (TK_TIMING); }
TYPE                   { return (TK_TYPE); }
UNITARY_G              { return (TK_UNITARY_G); }
UPPER                  { return (TK_UPPER); }
VERT_TRANS             { return (TK_VERT_TRANS); }
VMREST                 { return (TK_VMREST); }
VOLTAGES               { return (TK_VOLTAGES); }
V_TAU_VALUE_H          { return (TK_VTAU_VAL_H); }
V_TAU_VOLTAGE_H        { return (TK_VTAU_VOLT_H); }
V_TAU_VALUE_M          { return (TK_VTAU_VAL_M); }
V_TAU_VOLTAGE_M        { return (TK_VTAU_VOLT_M); }
V_TAU_VALUE_H_STDEV    { return (TK_VAL_H_STDEV); }
V_TAU_VOLTAGE_H_STDEV  { return (TK_VOLT_H_STDEV); }
V_TAU_VALUE_M_STDEV    { return (TK_VAL_M_STDEV); }
V_TAU_VOLTAGE_M_STDEV  { return (TK_VOLT_M_STDEV); }
VERSION                { return (TK_VERSION); }
WARNINGS_OFF           { return (TK_WARNINGS_OFF); }
WIDTH                  { return (TK_WIDTH); }
SFD                    { return (TK_SFD); }
SFD_LABEL              { return (TK_SFD_LABEL); }
PSG_FILE               { return (TK_PSG_FILE); }
CELLS_PER_FREQ         { return (TK_CELLS_PER_FREQ); }
TIME_FREQ_INCREMENT    { return (TK_TIME_FREQ_INC); }
DYN_RANGE              { return (TK_DYN_RANGE); }
PORT                   { return (TK_PORT); }
JOB                    { return (TK_JOB); }
DISTRIBUTE             { return (TK_DISTRIBUTE); }
CONNECT_REPORT         { return (TK_CONNECT_RPT); }
SPIKE_REPORT           { return (TK_SPIKE_RPT); }

Km           { strncpy (yylval.sval, yytext, STRLEN-1); return (TK_Km); }
Kahp         { strncpy (yylval.sval, yytext, STRLEN-1); return (TK_Kahp); }
Ka           { strncpy (yylval.sval, yytext, STRLEN-1); return (TK_Ka); }
Na           { strncpy (yylval.sval, yytext, STRLEN-1); return (TK_Na); }
Knicd        { strncpy (yylval.sval, yytext, STRLEN-1); return (TK_Knicd); }

FALSE        { yylval.ival = 0; return (LOGICAL); }
NO           { yylval.ival = 0; return (LOGICAL); }
TRUE         { yylval.ival = 1; return (LOGICAL); }
YES          { yylval.ival = 1; return (LOGICAL); }


{NAME}       { strncpy (yylval.sval, yytext, STRLEN-1); return (NAME); }

[ \t\x0d]+    /* eat up whitespace - including accidental ^M's from MSWord! */

[\n]        { TIN->line++; }

%%

/*------------------------------------------------------------------------*/

scanner (char *input, int nbytes)
{
  input [nbytes] = input [nbytes+1] = YY_END_OF_BUFFER_CHAR;

  yy_scan_buffer (input, nbytes+2);

  yyparse ();
}

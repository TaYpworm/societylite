/* ncs input parser, v1.  See comments at end of file */

%{
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#include "input.h"
#include "proto.h"
#include "FileStack.h"
#include "../defines.h"

#define TRUE  1
#define FALSE 0
#define VSIZE 100

extern TMP_INPUT *TIN;       /* This must be global so parse.y can access it */

T_BRAIN           *brain;           /* These are temporary pointers, to whatever */
T_CSHELL          *csh;             /* element the parser is working on          */
T_COLUMN          *column;
T_LSHELL          *lsh;
T_LAYER           *layer;
T_CELL            *cell;
T_CMP             *cmp;
T_CHANNEL         *chan;
T_SYNAPSE         *syn;
T_SYN_FD          *syn_fd;
T_SYNLEARN        *syn_learn;
T_SYNDATA         *syn_data;
T_SYNPSG          *syn_psg;
T_SYNAUGMENTATION *syn_augmentation;
T_SPIKE           *spike;
T_STIMULUS        *stim;
T_STINJECT        *sti;
T_REPORT          *report;

FileStack fileStack;       /* incase there are multiple files being parsed */
int initFlag = 0;          /* flag to indicate if the file stack is ever used */
extern char* currentFile;  /* name of file currently being parsed */

int i, nval;               /* For reading a list of numbers of unknown length */
double vlist [VSIZE];
double *twoptr;
double *allocVlist (int, double *);
double *allocRVlist (int, double *);
%}

%union
{
  double rval;
  int    ival;
  char   sval [STRLEN];
}


%token <rval> REAL
%token <ival> INTEGER LOGICAL
%token <sval> NAME

/* Tokens that match input file keywords are all named with the prefix "TK_"*/
/* This is so the obvious names can be used in the C code without conflicts */

%token TK_ABSOLUTE_USE   TK_AMP_END        TK_AMP_START      TK_ASCII
%token TK_BRAIN          TK_CA_EXP         TK_CA_EXTERNAL    TK_CA_HALF_MIN
%token TK_CA_INTERNAL    TK_CA_SCALE       TK_CA_SPIKE_INC   TK_CA_TAU
%token TK_CA_SPIKE_INC   TK_CA_TAU         TK_CA_TAU_SCALE   TK_CELL
%token TK_CELLS          TK_CELLS_PER_FREQ TK_CELL_TYPE      TK_CHANNEL
%token TK_COLUMN         TK_CSHELL         TK_COLUMN_TYPE    TK_CMP
%token TK_G              TK_CONNECT        TK_DATA_LABEL     TK_DELAY
%token TK_DEPR_TAU       TK_DURATION       TK_DYN_RANGE      TK_END_BRAIN    
%token TK_END_COLUMN     TK_END_CSHELL     TK_END_CMP        TK_END_CELL
%token TK_END_CHANNEL    TK_END_LAYER      TK_END_LSHELL     TK_END_REPORT   
%token TK_END_SPIKE      TK_END_STIMULUS   TK_END_ST_INJECT  TK_END_SYNAPSE
%token TK_END_SYN_DATA   TK_END_SYN_FD     TK_END_SYN_PSG    TK_END_SYN_LEARN
%token TK_E_HALF_MIN_H   TK_E_HALF_MIN_M   TK_FACIL_TAU      TK_FILENAME
%token TK_FREQUENCY      TK_FREQ_ROWS      TK_FREQ_START     TK_FSV
%token TK_HEIGHT         TK_H_INITIAL      TK_H_POWER        TK_IGNORE_EMPTY
%token TK_INJECT         TK_INTERACTIVE    TK_LAYER          TK_LSHELL
%token TK_LAYER_TYPE     TK_LEAK_G         TK_LEAK_REVERSAL  TK_LEARN
%token TK_LEARN_LABEL    TK_LOCATION       TK_LOWER          TK_MAX_G
%token TK_MODE           TK_M_INITIAL      TK_M_POWER        TK_NEG_HEB_WINDOW
%token TK_PATTERN        TK_POS_HEB_WINDOW TK_PROB           TK_PSG_FILE      
%token TK_RELOAD_SYN     TK_REPORT         TK_REPORT_ON      TK_REVERSAL       
%token TK_RSE            TK_RSE_LABEL      TK_R_MEMBRANE     TK_SAMESEED
%token TK_SAVE_SYN       TK_SEED TK_SFD    TK_SFD_LABEL      TK_SLOPE_H
%token TK_SLOPE_M        TK_SPIKE          TK_STIMULUS       TK_SPIKE_HW
%token TK_ST_INJECT      TK_STIM_TYPE      TK_SYNAPSE        TK_STRENGTH
%token TK_SYN_DATA       TK_SYN_FD         TK_SYN_LEARN      TK_SYN_PSG
%token TK_SYN_REVERSAL   TK_TAU_MEMBRANE   TK_TAU_SCALE_M    TK_TAU_SCALE_H
%token TK_THRESHOLD      TK_TIME_END       TK_TIME_START     TK_TIME_FREQ_INC
%token TK_TIMING         TK_TYPE           TK_UPPER          TK_UNITARY_G
%token TK_VMREST         TK_VOLTAGES       TK_VTAU_VAL_M     TK_VTAU_VAL_H
%token TK_PORT           TK_WIDTH          TK_JOB            TK_DISTRIBUTE
%token TK_VAL_M_STDEV    TK_VOLT_M_STDEV   TK_SLOPE_M_STDEV
%token TK_VAL_H_STDEV    TK_VOLT_H_STDEV   TK_SLOPE_H_STDEV
%token TK_NEG_HEB_PEAK_DELTA_USE   TK_NEG_HEB_PEAK_TIME      TK_VTAU_VOLT_M
%token TK_POS_HEB_PEAK_DELTA_USE   TK_POS_HEB_PEAK_TIME      TK_VTAU_VOLT_H
%token TK_INCLUDE        TK_RSE_INIT       TK_VERT_TRANS     TK_PREV_SPIKE_RANGE
%token TK_CONNECT_RPT    TK_SPIKE_RPT      TK_SERVER         TK_SINGLE
%token TK_CA_EXP_RANGE   TK_PHASE_SHIFT    TK_STRENGTH_RANGE TK_SYNAPSE_RSE
%token TK_ALPHA_SCALE_H  TK_ALPHA_SCALE_M  TK_BETA_SCALE_H   TK_BETA_SCALE_M
%token TK_SAVE           TK_LOAD           TK_DISTANCE       TK_OUTPUT_CONNECT_MAP
%token TK_OUTPUT_CELLS   TK_AUTO           TK_SERVER_PORT    TK_VERSION
%token TK_SYNAPSE_UF     TK_RECURRENT_CONNECT TK_ALPHA
%token TK_SYN_AUGMENTATION      TK_END_SYN_AUGMENTATION      TK_MAX_AUGMENTATION
%token TK_AUGMENTATION_INIT     TK_AUGMENTATION_TAU          TK_SYN_CALCIUM
%token TK_CA_TAU_DECAY   TK_EXP            TK_SELECT_FRONT   TK_OPTION
%token TK_AVERAGE_SYN    TK_AUGMENTATION_DELAY               TK_WARNINGS_OFF
%token TK_HIDE_TIMESTEP

/* Tokens for channel family types */

%token TK_Km  TK_Kahp  TK_Ka  TK_Na  TK_Knicd

%% /* Grammar rules and actions follow */

input:   element 
       | input element
       | input error
;

element:  brain      
        | column     
        | colshell
        | layer      
        | lshell
        | cell
        | compart
        | channel    
        | synapse
        | syn_psg
        | syn_fd
        | syn_learn
        | syn_data
        | syn_augmentation
        | spikeshape 
        | stimulus
        | stinject
        | report
        | TK_INCLUDE NAME { multiInput( $<sval>2 ); }
; 

/*--------------- Input for the BRAIN structure ------------*/

brain:    TK_BRAIN { brain = makebrain (); } brainvars TK_END_BRAIN
;

brainvars: brainvar
         | brainvars brainvar
;

brainvar:  TK_TYPE         NAME    { brain->L.name      = strdup ($<sval>2); }
         | TK_JOB          NAME    { brain->job         = strdup ($<sval>2); }
         | TK_DISTRIBUTE   NAME    { brain->distribute  = strdup ($<sval>2); }
         | TK_CONNECT_RPT          { brain->ConnectRpt  = TRUE; }
         | TK_SPIKE_RPT            { brain->SpikeRpt    = TRUE; }
         | TK_FSV          value   { brain->FSV         = $<rval>2; }
         | TK_DURATION     value   { brain->Duration    = $<rval>2; }
         | TK_SEED         INTEGER { brain->Seed        = $<ival>2; }
         | TK_COLUMN_TYPE  NAME    { brain->ColumnNames = AddName (brain->ColumnNames, $<sval>2, 0);
                                     brain->nColumns++; }
         | TK_ST_INJECT    NAME    { brain->StInjNames  = AddName (brain->StInjNames,  $<sval>2, 0);
                                     brain->nStInject++; }
         | TK_REPORT       NAME    { brain->ReportNames = AddName (brain->ReportNames, $<sval>2, 0);
                                     brain->nReports++; }
         | TK_CONNECT      NAME NAME NAME NAME NAME NAME NAME NAME NAME value value
               { brain->CnList = makeConnect (brain->CnList, &(brain->nConnect),
                                               $<sval>2, $<sval>3, $<sval>4, $<sval>5,
                                               $<sval>6, $<sval>7, $<sval>8, $<sval>9,
                                               $<sval>10, $<rval>11, $<rval>12);
               }
         | TK_CONNECT      NAME NAME NAME NAME NAME NAME NAME NAME NAME value value value
               { brain->CnList = makeDecayingConnect ( brain->CnList, &(brain->nConnect),
                                               $<sval>2, $<sval>3, $<sval>4, $<sval>5,
                                               $<sval>6, $<sval>7, $<sval>8, $<sval>9,
                                               $<sval>10, $<rval>11, $<rval>12, $<rval>13 );
               }
         | TK_RECURRENT_CONNECT NAME NAME NAME NAME NAME NAME NAME NAME NAME value value
               {
                 brain->recurrentList = makeRecurrentConnect( brain->recurrentList, &(brain->nRecurrent),
                                               $<sval>2, $<sval>3, $<sval>4, $<sval>5,
                                               $<sval>6, $<sval>7, $<sval>8, $<sval>9,
                                               $<sval>10,
 $<rval>11,
 $<rval>12
 );
               }

         | TK_INTERACTIVE  LOGICAL { unused (TK_INTERACTIVE); }
         | TK_IGNORE_EMPTY LOGICAL { unused (TK_IGNORE_EMPTY); }
         | TK_SAVE_SYN     NAME    { unused (TK_SAVE_SYN); }
         | TK_SAVE         NAME    value { brain->savefile = strdup( $<sval>2 ); brain->savetime = $<rval>3; }
         | TK_LOAD         NAME    { brain->loadfile = strdup( $<sval>2 ); }
         | TK_PORT         INTEGER { brain->Port = $<ival>2; }
         | TK_PORT         TK_AUTO { brain->Port = -1; }
         | TK_PORT         TK_SINGLE { brain->Port = -2; }
         | TK_SERVER       NAME    { brain->HostName = strdup( $<sval>2 ); }
         | TK_SERVER_PORT  INTEGER { brain->HostPort = $<ival>2; }
         | TK_DISTANCE     LOGICAL { if( $<ival>2 ) brain->flag = setFlag( brain->flag, "USE_DISTANCE" ); }
         | TK_OUTPUT_CELLS LOGICAL { if( $<ival>2 ) brain->flag = setFlag( brain->flag, "OUTPUT_CELLS" ); }
         | TK_OUTPUT_CONNECT_MAP LOGICAL  { if( $<ival>2 ) brain->flag = setFlag( brain->flag, "OUTPUT_CONNECT_MAP" ); }
         | TK_OPTION       TK_WARNINGS_OFF  { brain->flag = setFlag( brain->flag, "WARNINGS_OFF" );  }
;

/*--------------- Input for the COLUMN_SHELL structure ------------*/

colshell: TK_CSHELL { csh = makecsh (); } cshvars TK_END_CSHELL
;

cshvars: cshvar
       | cshvars cshvar
;

cshvar: TK_TYPE     NAME        { csh->L.name = strdup ($<sval>2); }
      | TK_WIDTH    value       { csh->width  = $<rval>2; }
      | TK_HEIGHT   value       { csh->height = $<rval>2; }
      | TK_LOCATION value value { csh->x      = $<rval>2; csh->y = $<rval>3; }
;

/*--------------- Input for the COLUMN structure ------------*/

column:   TK_COLUMN { column = makecolumn (); } colvars TK_END_COLUMN
;

colvars: colvar
       | colvars colvar
;

colvar: TK_TYPE       NAME { column->L.name     = strdup ($<sval>2); }
      | TK_CSHELL     NAME { column->shellName  = strdup ($<sval>2); }
      | TK_LAYER_TYPE NAME { column->LayerNames = AddName (column->LayerNames, $<sval>2, 0);
                             column->nLayers++; }
      | TK_CONNECT    NAME NAME NAME NAME NAME NAME NAME value value
               { column->CnList = makeConnect (column->CnList, &(column->nConnect),
                                                NULL, $<sval>2, $<sval>3, $<sval>4,
                                                NULL, $<sval>5, $<sval>6, $<sval>7,
                                                $<sval>8, $<rval>9, $<rval>10);
               }
      | TK_CONNECT    NAME NAME NAME NAME NAME NAME NAME value value value
               { column->CnList = makeDecayingConnect (column->CnList, &(column->nConnect),
                                                NULL, $<sval>2, $<sval>3, $<sval>4,
                                                NULL, $<sval>5, $<sval>6, $<sval>7,
                                                $<sval>8, $<rval>9, $<rval>10, $<rval>11);
               }
       | TK_RECURRENT_CONNECT NAME NAME NAME NAME NAME NAME NAME value value
               {
                 column->recurrentList = makeRecurrentConnect( column->recurrentList, &(column->nRecurrent),
                                               NULL, $<sval>2, $<sval>3, $<sval>4,
                                               NULL, $<sval>5, $<sval>6, $<sval>7,
                                               $<sval>8, $<rval>9, $<rval>10 );
               }
;

/*--------------- Input for the LAYER_SHELL structure ------------*/

lshell:   TK_LSHELL { lsh = makelsh (); } lsvars TK_END_LSHELL
;

lsvars: lsvar
      | lsvars lsvar
;

lsvar: TK_TYPE  NAME  { lsh->L.name = strdup ($<sval>2); }
     | TK_LOWER value { lsh->Lower  = $<rval>2; }
     | TK_UPPER value { lsh->Upper  = $<rval>2; }
;

/*--------------- Input for the LAYER structure ------------*/

layer:    TK_LAYER { layer = makelayer (); } lvars TK_END_LAYER
;

lvars:   lvar
       | lvars lvar
;

lvar:   TK_TYPE       NAME         { layer->L.name    = strdup ($<sval>2); }
      | TK_LSHELL     NAME         { layer->shellName = strdup ($<sval>2); }
      | TK_CELL_TYPE  NAME INTEGER { layer->CellNames = AddName (layer->CellNames, $<sval>2, (int) $<ival>3);
                                     layer->nCellTypes++; }
      | TK_CONNECT    NAME NAME NAME NAME NAME value value
               { layer->CnList = makeConnect (layer->CnList, &(layer->nConnect),
                                               NULL, NULL, $<sval>2, $<sval>3,
                                               NULL, NULL, $<sval>4, $<sval>5,
                                               $<sval>6, $<rval>7, $<rval>8);
               }
      | TK_CONNECT    NAME NAME NAME NAME NAME value value value
               { layer->CnList = makeDecayingConnect (layer->CnList, &(layer->nConnect),
                                               NULL, NULL, $<sval>2, $<sval>3,
                                               NULL, NULL, $<sval>4, $<sval>5,
                                               $<sval>6, $<rval>7, $<rval>8, $<rval>9);
               }
      | TK_RECURRENT_CONNECT NAME NAME NAME NAME NAME value value
               {
                 layer->recurrentList = makeRecurrentConnect( layer->recurrentList, &(layer->nRecurrent), 
                                               NULL, NULL, $<sval>2, $<sval>3,
                                               NULL, NULL, $<sval>4, $<sval>5,
                                               $<sval>6, $<rval>7, $<rval>8 );
               }
;

/*--------------- Input for the CELL structure ------------*/

cell: TK_CELL { cell = makecell (); } cellvars TK_END_CELL
;

cellvars: cellvar
        | cellvars cellvar
;

cellvar: TK_TYPE    NAME  { cell->L.name = strdup ($<sval>2); }
       | TK_CMP     NAME NAME value value
                          { cell->CmpNames = AddCmp (cell->CmpNames, $<sval>2, $<sval>3,
                                                     $<rval>4, $<rval>5, 0);
                            //pass 0 for z-coord for compatibility ------^
                            //with older input files
                            cell->nCmp++;
                          }
       | TK_CMP     NAME NAME value value value
                          { cell->CmpNames = AddCmp (cell->CmpNames, $<sval>2, $<sval>3,
                                                     $<rval>4, $<rval>5, $<rval>6 );
                            cell->nCmp++;
                          }

       | TK_CONNECT NAME NAME value value value
                          { cell->CnList = makeCmpConn (cell->CnList, $<sval>2, $<sval>3, 
                                                         $<rval>4, $<rval>5, $<rval>6); 
                            cell->nConnect++;
                          }
;

/*--------------- Input for the COMPARTMENT structure ------------*/

compart: TK_CMP  { cmp = makecmp (); } cmpvars TK_END_CMP
;

cmpvars: cmpvar
       | cmpvars cmpvar
;

cmpvar:  TK_TYPE          NAME     { cmp->L.name     = strdup ($<sval>2); }
       | TK_SEED          INTEGER  { cmp->Seed       = $<ival>2; }
       | TK_SPIKE         NAME     { cmp->SpikeName  = strdup ($2); }
       | TK_CHANNEL       NAME     { cmp->ChannelNames = AddName (cmp->ChannelNames, $<sval>2, 0);
                                     cmp->nChannels++; }
       | TK_SPIKE_HW      { twoptr = cmp->Spike_HW; }      twovalue
       | TK_TAU_MEMBRANE  { twoptr = cmp->Tau_Membrane; }  twovalue
       | TK_R_MEMBRANE    { twoptr = cmp->R_Membrane; }    twovalue
       | TK_THRESHOLD     { twoptr = cmp->Threshold; }     twovalue
       | TK_LEAK_REVERSAL { twoptr = cmp->Leak_Reversal; } twovalue
       | TK_LEAK_G        { twoptr = cmp->Leak_G; }        twovalue
       | TK_VMREST        { twoptr = cmp->VMREST; }        twovalue
       | TK_CA_INTERNAL   { twoptr = cmp->CaInt; }         twovalue
       | TK_CA_EXTERNAL   { twoptr = cmp->CaExt; }         twovalue
       | TK_CA_TAU        { twoptr = cmp->CaTau; }         twovalue
       | TK_CA_SPIKE_INC  { twoptr = cmp->CaSpikeInc; }    twovalue
;

/*--------------- Input for the CHANNEL structure ------------*/
/* There are several types of channels that can be input.  The type is
   specified by a family name following the TK_CHANNEL keyword */


channel: TK_CHANNEL TK_Km   { chan = makechan ($<sval>2); } Kmvars   TK_END_CHANNEL
       | TK_CHANNEL TK_Kahp { chan = makechan ($<sval>2); } Kahpvars TK_END_CHANNEL
       | TK_CHANNEL TK_Ka   { chan = makechan ($<sval>2); } Kavars   TK_END_CHANNEL
       | TK_CHANNEL TK_Na   { chan = makechan ($<sval>2); } Navars   TK_END_CHANNEL
       | TK_CHANNEL TK_Knicd { chan = makechan ($<sval>2); } Knicdvars TK_END_CHANNEL
;

Kmvars: chvar
      | Kmvars chvar
      | Kmvars Kmvar
;

Kahpvars: chvar
        | Kahpvars chvar
        | Kahpvars Kahpvar
;

Kavars: chvar
      | Kavars chvar
      | Kavars Kavar
;

Navars: chvar
      | Navars chvar
      | Navars Navar
;

Knicdvars: chvar
         | Knicdvars chvar
;

/* chvars are those that are common to all families */

chvar: TK_TYPE       NAME     { chan->L.name    = strdup ($<sval>2); }
     | TK_M_POWER    { twoptr = chan->mPower; }     twovalue
     | TK_UNITARY_G  { twoptr = chan->unitaryG; }   twovalue
     | TK_STRENGTH   { twoptr = chan->strength ; }  twovalue
     | TK_STRENGTH_RANGE   { twoptr = chan->strength_range; }  twovalue
     | TK_M_INITIAL  { twoptr = chan->M_Initial; }  twovalue
     | TK_REVERSAL   { twoptr = chan->ReversePot; } twovalue 
     | TK_SEED       INTEGER  { chan->Seed = $<ival>2; }
;

Kmvar: TK_E_HALF_MIN_M  { twoptr = chan->eHalfMinM; } twovalue 
     | TK_TAU_SCALE_M   { twoptr = chan->tauScaleM; } twovalue
     | TK_SLOPE_M       value value value { chan->slopeM [0] = $<rval>2;
                                            chan->slopeM [1] = $<rval>3;
                                            chan->slopeM [2] = $<rval>4; }
     | TK_SLOPE_M_STDEV value { chan->slopeM_stdev = $<rval>2; }
;

Kahpvar: TK_CA_SCALE     { twoptr = chan->CA_SCALE; }     twovalue
       | TK_CA_EXP       { twoptr = chan->CA_EXP; }       twovalue
       | TK_CA_EXP_RANGE { twoptr = chan->CA_EXP_RANGE; } twovalue
       | TK_CA_HALF_MIN  { twoptr = chan->CA_HALF_MIN; }  twovalue
       | TK_CA_TAU_SCALE { twoptr = chan->CA_TAU_SCALE; } twovalue
;

Kavar: TK_H_INITIAL      { twoptr = chan->H_Initial; } twovalue
     | TK_H_POWER        { twoptr = chan->hPower;    } twovalue
     | TK_E_HALF_MIN_M   { twoptr = chan->eHalfMinM;  } twovalue
     | TK_E_HALF_MIN_H   { twoptr = chan->eHalfMinH;  } twovalue
     | TK_SLOPE_M          value { chan->slopeM [0] = $<rval>2; }
     | TK_SLOPE_M_STDEV    value { chan->slopeM_stdev = $<rval>2; }
     | TK_SLOPE_H          value { chan->slopeH [0] = $<rval>2; }
     | TK_SLOPE_H_STDEV    value { chan->slopeH_stdev = $<rval>2; }

     | TK_VAL_M_STDEV      value { chan->ValM_stdev  = $<rval>2; }
     | TK_VOLT_M_STDEV     value { chan->VoltM_stdev = $<rval>2; }
     | TK_VAL_H_STDEV      value { chan->ValH_stdev  = $<rval>2; }
     | TK_VOLT_H_STDEV     value { chan->VoltH_stdev = $<rval>2; }

     | TK_VTAU_VAL_M  { nval = 0; } values { chan->nValM  = nval; chan->TauValM  = allocVlist (nval, vlist); }
     | TK_VTAU_VOLT_M { nval = 0; } values { chan->nVoltM = nval; chan->TauVoltM = allocVlist (nval, vlist); }
     | TK_VTAU_VAL_H  { nval = 0; } values { chan->nValH  = nval; chan->TauValH  = allocVlist (nval, vlist); }
     | TK_VTAU_VOLT_H { nval = 0; } values { chan->nVoltH = nval; chan->TauVoltH = allocVlist (nval, vlist); }
;

Navar: TK_E_HALF_MIN_M  { twoptr = chan->eHalfMinM; } twovalue
     | TK_E_HALF_MIN_H  { twoptr = chan->eHalfMinH;  } twovalue
     | TK_H_INITIAL      { twoptr = chan->H_Initial; } twovalue
     | TK_H_POWER        { twoptr = chan->hPower;    } twovalue
     | TK_TAU_SCALE_M   { twoptr = chan->tauScaleM; } twovalue
     | TK_TAU_SCALE_H   { twoptr = chan->tauScaleH; } twovalue
     | TK_SLOPE_M       value value { chan->slopeM [0] = $<rval>2;
                                      chan->slopeM [1] = $<rval>3; }
     | TK_SLOPE_H       value value { chan->slopeH [0] = $<rval>2;
                                      chan->slopeH [1] = $<rval>3; }
     | TK_SLOPE_M_STDEV    value { chan->slopeM_stdev = $<rval>2; }
     | TK_SLOPE_H_STDEV    value { chan->slopeH_stdev = $<rval>2; }
     | TK_ALPHA_SCALE_M { twoptr = chan->alphaScaleFactorM; } twovalue
     | TK_BETA_SCALE_M { twoptr = chan->betaScaleFactorM; } twovalue
     | TK_ALPHA_SCALE_H { twoptr = chan->alphaScaleFactorH; } twovalue
     | TK_BETA_SCALE_H { twoptr = chan->betaScaleFactorH; } twovalue
;

/*--------------- Input for the SYNAPSE structure ------------*/

synapse: TK_SYNAPSE { syn = makesynapse (); } synvars TK_END_SYNAPSE
;

synvars: synvar
       | synvars synvar
;

synvar: TK_TYPE           NAME    { syn->L.name    = strdup ($<sval>2); }
      | TK_SEED           INTEGER { syn->Seed      = $<ival>2; }
      | TK_SFD_LABEL      NAME    { syn->SfdName   = strdup ($<sval>2); }
      | TK_LEARN_LABEL    NAME    { syn->LearnName = strdup ($<sval>2); }
      | TK_DATA_LABEL     NAME    { syn->DataName  = strdup ($<sval>2); 
                                    deprecate (TK_DATA_LABEL); }
      | TK_SYN_AUGMENTATION NAME  { syn->AugmentationName = strdup ( $<sval>2 ); }
      | TK_SYN_PSG        NAME    { syn->PsgName   = strdup ($<sval>2); }
      | TK_ABSOLUTE_USE  { twoptr = syn->USE; } twovalue
      | TK_DELAY         { twoptr = syn->SynDelay; } twovalue
      | TK_SYN_REVERSAL  { twoptr = syn->SynRever; } twovalue
      | TK_MAX_G         { twoptr = syn->MaxG; } twovalue
      | TK_RSE_INIT  { twoptr = syn->InitRSE; } twovalue
      | TK_PREV_SPIKE_RANGE { twoptr = syn->InitDeltaT; } twovalue
;

/*--------------- Input for the SYN_INPUT_PSG structure ------------*/

syn_psg: TK_SYN_PSG { syn_psg = makesyn_psg (); } spvars  TK_END_SYN_PSG
;

spvars: spvar
      | spvars spvar
;

spvar: TK_TYPE     NAME { syn_psg->L.name = strdup ($<sval>2); }
     | TK_PSG_FILE NAME { syn_psg->File   = strdup ($<sval>2); }
;

/*--------------- Input for the SYN_FACIL_DEPRESS structure ------------*/

syn_fd:  TK_SYN_FD { syn_fd = makesyn_fd (); } sfvars TK_END_SYN_FD
;

sfvars: sfvar
      | sfvars sfvar
;

sfvar: TK_TYPE      NAME     { syn_fd->L.name = strdup ($<sval>2); }
     | TK_SEED      INTEGER  { syn_fd->Seed   = $<ival>2; }
     | TK_SFD       NAME     { syn_fd->SFD    = SFDCode ($<sval>2); }
     | TK_FACIL_TAU { twoptr = syn_fd->Facil_Tau; } twovalue 
     | TK_DEPR_TAU  { twoptr = syn_fd->Depr_Tau; }  twovalue
;

/*--------------- Input for the SYN_LEARNING structure ------------*/

syn_learn: TK_SYN_LEARN { syn_learn = makesyn_learn (); } slvars  TK_END_SYN_LEARN
;

slvars: slvar
      | slvars slvar
;

slvar: TK_TYPE                   NAME     { syn_learn->L.name = strdup ($<sval>2); }
     | TK_SEED                   INTEGER  { syn_learn->Seed = $<ival>2; }
     | TK_LEARN                  NAME     { syn_learn->Learning = LearnCode ($<sval>2); }
     | TK_NEG_HEB_WINDOW         { twoptr = syn_learn->DownWind; }               twovalue 
     | TK_POS_HEB_WINDOW         { twoptr = syn_learn->UpWind; }                 twovalue 
     | TK_NEG_HEB_PEAK_DELTA_USE { twoptr = syn_learn->Neg_Heb_Peak_Delta_Use; } twovalue 
     | TK_POS_HEB_PEAK_DELTA_USE { twoptr = syn_learn->Pos_Heb_Peak_Delta_Use; } twovalue 
     | TK_NEG_HEB_PEAK_TIME      { twoptr = syn_learn->Neg_Heb_Peak_Time; }      twovalue
     | TK_POS_HEB_PEAK_TIME      { twoptr = syn_learn->Pos_Heb_Peak_Time; }      twovalue
;

/*--------------- Input for the SYN_DATA structure ------------*/

syn_data: TK_SYN_DATA { syn_data = makesyn_data (); 
                        deprecate (TK_SYN_DATA); } sdvars  TK_END_SYN_DATA
;

sdvars: sdvar
      | sdvars sdvar
;

sdvar: TK_TYPE         NAME     { syn_data->L.name = strdup ($<sval>2); }
     | TK_SEED         INTEGER  { syn_data->Seed   = $<ival>2; }
     | TK_MAX_G        { twoptr = syn_data->MaxG; } twovalue
     | TK_DELAY        { twoptr = syn_data->SynDelay; } twovalue 
     | TK_SYN_REVERSAL { twoptr = syn_data->SynRever; } twovalue 
;

/*--------------- Input for the SYN_AUGMENTATION structure ------------*/

syn_augmentation: TK_SYN_AUGMENTATION { syn_augmentation = makesyn_augmentation (); } savars TK_END_SYN_AUGMENTATION
;

savars: savar
      | savars savar

savar: TK_TYPE                 NAME    { syn_augmentation->L.name = strdup ($<sval>2 ); }
     | TK_CA_INTERNAL          { twoptr = syn_augmentation->CA_init; } twovalue
     | TK_CA_TAU_DECAY         { twoptr = syn_augmentation->CA_decay; } twovalue
     | TK_CA_TAU               { twoptr = syn_augmentation->CA_tau; } twovalue
     | TK_CA_SPIKE_INC         { twoptr = syn_augmentation->CA_increment; } twovalue
     | TK_MAX_AUGMENTATION     { twoptr = syn_augmentation->MaxSA; } twovalue
     | TK_ALPHA                { twoptr = syn_augmentation->Alpha; } twovalue
     | TK_AUGMENTATION_INIT    { twoptr = syn_augmentation->Augmentation_init; } twovalue
     | TK_AUGMENTATION_TAU     { twoptr = syn_augmentation->Augmentation_tau; } twovalue
     | TK_AUGMENTATION_DELAY   { twoptr = syn_augmentation->SA_delay; } twovalue
;

/*--------------- Input for the SPIKESHAPE structure ------------*/

spikeshape: TK_SPIKE { spike = makespike (); } spikevars TK_END_SPIKE
;

spikevars: spikevar 
         | spikevars spikevar
;

spikevar: TK_TYPE NAME { spike->L.name = strdup ($<sval>2); }
        | TK_VOLTAGES  { nval = 0; } values { spike->nVoltages = nval;
                                              spike->Voltages  = allocRVlist (nval, vlist); }
;

/*--------------- Input for the STIMULUS structure ------------*/

stimulus: TK_STIMULUS { stim = makestim (); } stvars TK_END_STIMULUS
;

stvars: stvar 
      | stvars stvar
;

stvar: TK_TYPE           NAME     { stim->L.name          = strdup ($<sval>2); }
     | TK_MODE           NAME     { stim->MODE            = ModeCode ($<sval>2); }
     | TK_PATTERN        NAME     { stim->PATTERN         = PatternCode ($<sval>2); }
     | TK_VERT_TRANS     value    { stim->VERT_TRANS      = $<rval>2; }
     | TK_PHASE_SHIFT    value    { stim->PHASE     = $<rval>2; }
     | TK_TIMING         NAME     { stim->TIMING          = TimingCode ($<sval>2); }
     | TK_FILENAME       NAME     { stim->FileName        = strdup ($<sval>2); }
     | TK_PORT           INTEGER  { stim->Port            = $<ival>2; }
     | TK_PORT           TK_AUTO  { stim->Port            = -1; }
     | TK_SAMESEED       LOGICAL  { stim->SAME_SEED       = $<ival>2; }
     | TK_SEED           INTEGER  { stim->Seed            = $<ival>2; }
     | TK_FREQ_ROWS      INTEGER  { stim->nFreqs          = $<ival>2; }
     | TK_CELLS_PER_FREQ INTEGER  { stim->CellsPerFreq    = $<ival>2; }
     | TK_TIME_FREQ_INC  { twoptr = stim->Time_Freq_Incr; } twovalue 
     | TK_DYN_RANGE      { twoptr = stim->DynRange; } twovalue 
     | TK_AMP_START      value    { stim->AMP_Start       = $<rval>2; }
     | TK_AMP_END        value    { stim->AMP_End         = $<rval>2; }
     | TK_WIDTH          value    { stim->WidthSec        = $<rval>2; }
     | TK_FREQ_START     value    { stim->FREQ_Start      = $<rval>2; }
     | TK_TIME_START { nval = 0; } values { stim->nTStart = nval; 
                                            stim->TStart  = allocVlist (nval, vlist); }
     | TK_TIME_END   { nval = 0; } values { stim->nTStop  = nval;
                                            stim->TStop   = allocVlist (nval, vlist); }
;

/*--------------- Input for the ST_INJECT structure ------------*/

stinject: TK_ST_INJECT { sti = makesti (); } stivars TK_END_ST_INJECT
;

stivars: stivar 
       | stivars stivar
;

stivar: TK_TYPE      NAME { sti->L.name      = strdup ($<sval>2); }
      | TK_STIM_TYPE NAME { sti->StimName    = strdup ($<sval>2); }
      | TK_INJECT    NAME NAME NAME NAME value
                          { sti->ColName  = strdup ($<sval>2);
                            sti->LayName  = strdup ($<sval>3);
                            sti->CellName = strdup ($<sval>4);
                            sti->CmpName  = strdup ($<sval>5);
                            sti->Prob     = $<rval>6; }
;

/*--------------- Input for the REPORT structure ------------*/

report: TK_REPORT { report = makereport (); } reportvars TK_END_REPORT
;

reportvars: reportvar 
          | reportvars reportvar
;

reportvar: TK_TYPE       NAME    { report->L.name    = strdup ($<sval>2); }
         | TK_CELLS      NAME NAME NAME NAME
                                 { report->ColName   = strdup ($<sval>2);
                                   report->LayName   = strdup ($<sval>3);
                                   report->CellName  = strdup ($<sval>4);
                                   report->CmpName   = strdup ($<sval>5); }
         | TK_FILENAME   NAME    { report->FileName  = strdup ($<sval>2); }
         | TK_PORT       INTEGER { report->Port      = $<ival>2; }
         | TK_PORT       TK_AUTO { report->Port      = -1; }
         | TK_ASCII              { report->ASCII     = TRUE; }
         | TK_ASCII      TK_EXP  { report->reportFlag |= 1;
                                   report->ASCII     = TRUE; }
         | TK_CHANNEL    NAME    { report->ReportOn  = ReportCode ("CHANNEL_RPT");
                                   report->Name      = strdup ($<sval>2); 
                                   printf ("Report Channel: code = %x, name = '%s'\n", report->ReportOn, report->Name); }
         | TK_REPORT_ON  NAME    { report->ReportOn  = ReportCode ($<sval>2); }
         | TK_PROB       value   { report->Prob      = $<rval>2; }
         | TK_FREQUENCY  value   { report->Frequency = $<rval>2; }
         | TK_TIME_START { nval = 0; } values 
                                 { report->nTStart = nval;
                                   report->TStart  = allocVlist (nval, vlist); }
         | TK_TIME_END   { nval = 0; } values 
                                 { report->nTStop  = nval;
                                   report->TStop   = allocVlist (nval, vlist); }
         | TK_SYNAPSE    NAME    { report->ReportOn = ReportCode ("SYNAPSE_USE");
                                   report->Name     = strdup ($<sval>2); }
         | TK_SYNAPSE_RSE NAME   {
                                   report->ReportOn = ReportCode ("SYNAPSE_RSE");
                                   report->Name     = strdup ($<sval>2);
                                 }
         | TK_SYNAPSE_UF  NAME   {
                                   report->ReportOn = ReportCode ("SYNAPSE_UF");
                                   report->Name     = strdup ($<sval>2);
                                 }
         | TK_SYN_AUGMENTATION NAME  {
                                       report->ReportOn = ReportCode ("SYNAPSE_SA");
                                       report->Name     = strdup ($<sval>2);
                                     }
         | TK_SYN_CALCIUM NAME       {
                                       report->ReportOn = ReportCode ("SYNAPSE_CA");
                                       report->Name     = strdup ($<sval>2);
                                     }
         | TK_SEED INTEGER       { report->Seed = $<ival>2; }
         | TK_SEED TK_SELECT_FRONT  { report->Seed = SELECT_FRONT; }
         | TK_VERSION INTEGER    { report->reportFlag |= $<ival>2; }
         | TK_OPTION reportoptions { }
;

/* -------------Input for the Option of Report---------------------------------*/
reportoptions: singleoption
             | reportoptions singleoption
;

singleoption: TK_AVERAGE_SYN { report->reportFlag |= AVERAGE_SYN; }
            | TK_HIDE_TIMESTEP { report->reportFlag |= HIDE_STEP; }
;

/*--------------- These rules collect a list of numeric values ------------*/

values: value         { if (nval < VSIZE) vlist [nval++] = $<rval>1; }
      | values value  { if (nval < VSIZE) vlist [nval++] = $<rval>2; }
;

twovalue: value value  { twoptr [0] = $<rval>1; twoptr [1] = $<rval>2; }
        | value        { twoptr [0] = $<rval>1; twoptr [1] = 0.0; }
;

value: INTEGER { $<rval>$ = (double) $<ival>1; }
     | REAL    { $<rval>$ = $<rval>1; }
;

%%

int multiInput( char *fname )
{
  INFILE latest;

  if( !initFlag )  //is this the first time we're including another input file?
  {
    initFileStack( &fileStack );
    initFlag = 1;
  }

  latest.from = NULL;
  latest.file = strdup( fname );//filename to be opened
  latest.parentFile = strdup( currentFile );
  latest.line = TIN->line;//remember where in the current input file this was included
  latest.col = 0;

  pushFileStack( &fileStack, &latest );

  if( latest.file ) free( latest.file );
}

int yywrap(void)
{
  INFILE *next = NULL;
  struct stat fs;
  int nbytes;
  FILE *in;

  if( initFlag )
  {
    next = popFileStack( &fileStack, next );
    while( next != NULL )
    {
      //bring in new info

      stat( next->file, &fs );
      nbytes = fs.st_size;
      in = fopen( next->file, "r" );

      if( !in )
      {
        fprintf( stderr, "Error: could not open file %s\n", next->file );
        fprintf( stderr, "\tdefined in file %s, line %d\n", next->parentFile, next->line );

        printf( "Error: could not open file %s\n", next->file );
        printf( "\tdefined in file %s, line %d\n", next->parentFile, next->line );

        free( next->parentFile );
        free( next->file );
        free( next );
        next = popFileStack( &fileStack, next );
        continue;
      }

      free( currentFile );
      currentFile = strdup( next->file );
      TIN->line = 0;
      yy_switch_to_buffer( yy_create_buffer( in, fs.st_size ) );  //should I store the return value somewhere?

      //these lines make it break, but why? I need to deallocate this memory, don't I?
      //free( next->parentFile );
      //free( next->file );
      //free( next );

      return 0;  //continue parsing
    }

    //next == NULL if I get here, so the fileStack is empty
    destroyFileStack( &fileStack );
    return 1;  //end parsing
  } 
  
  return 1;  //end parsing
}

yyerror (char *str)
{
  printerr ("%s:%d: %s\n", TIN->file, TIN->line, str);
  TIN->nParseErr++;
}

unused (int kind)
{
  printerr ("%s:%d: Warning: Keyword '%s' no longer used\n",
            TIN->file, TIN->line, TK2name (kind));
}

deprecate (int kind)
{
//printerr ("%s:%d: Warning: Keyword '%s' is deprecated\n",
//          TIN->file, TIN->line, TK2name (kind));
}

/*-----------------------------------------------------------------------

This is the parser for ncs input files.  It accepts tokens from the lexical
analyzer (scan.l).  Tokens may be known keywords (as listed in the %tokens)
section, or variables, which can be INTEGER, REAL, LOGICAL, or STRING.  The
parser creates input structures and assigns values to them according to the
rules of the input language.

The format for most brain elements is the same: the parser sees the token
for an element, creates a structure for that element (entering it into the
list of structures), then recognizes keyword-value sets allowed for that
element and assigns the values to the structure elements.


TODO:

  Need more informative error messages for parse errors.

  Add INCLUDE file capability.

  Someone who knows more about YACC/Bison might be able to improve on this
  set of grammar rules.
-----------------------------------------------------------------------*/

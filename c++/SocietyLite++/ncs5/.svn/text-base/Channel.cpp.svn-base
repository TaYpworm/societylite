#include "Channel.h"
#include "memstat.h"

#define MEM_KEY  KEY_CHANNEL

Channel::Channel ()
{
}

Channel::Channel (T_CHANNEL *iChan, Random *RN)
{
  MEMADDOBJ (MEM_KEY);

  source = iChan;
  family = iChan->family;

  mPower   = RN->GaussNum (iChan->mPower);
  unitaryG = RN->GaussNum (iChan->unitaryG);
  strength = RN->GaussNum (iChan->strength);

  //Initialize values even if they are not used
  nTauM = nTauH = 0;

/* compute pre-computed factors relevant to all types */
/* Gives very different results when used!?!?  - precision problem? */

/*pcf1 = unitaryG * strength;*/

/*printf ("Channel::Channel: Family = %s\n", NameDefine (family));  */
  switch (family)
  {
    case FAM_NA:     
        break;
    case FAM_CA:     
        break;
    case FAM_KM:    slopeFactorM  = RN->GaussArray (iChan->slopeM, 1, iChan->slopeM_stdev);
                    eHalfMinM [0] = RN->GaussNum (iChan->eHalfMinM);
                    
                    tauScaleM    = RN->GaussNum (iChan->tauScaleM);
                    nTauM        = iChan->nValM;
                    vTauValueM   = RN->GaussArray (iChan->TauValM,  iChan->nValM, iChan->ValM_stdev);
                    vTauVoltageM = RN->GaussArray (iChan->TauVoltM, iChan->nVoltM, iChan->VoltM_stdev);

                    nTauH        = iChan->nValH;
                    vTauValueH   = RN->GaussArray (iChan->TauValH,  iChan->nValH, iChan->ValH_stdev);
                    vTauVoltageH = RN->GaussArray (iChan->TauVoltH, iChan->nVoltH, iChan->VoltH_stdev);
        break;
    case FAM_KA:    hPower        = RN->GaussNum (iChan->hPower);
                    eHalfMinM [0] = RN->GaussNum (iChan->eHalfMinM);
                    eHalfMinH [0] = RN->GaussNum (iChan->eHalfMinH);

                    slopeFactorM  = RN->GaussArray (iChan->slopeM, 3, iChan->slopeM_stdev);
                    slopeFactorH  = RN->GaussArray (iChan->slopeH, 3, iChan->slopeH_stdev);

                    tauScaleM    = RN->GaussNum (iChan->tauScaleM);
                    nTauM        = iChan->nValM;
                    vTauValueM   = RN->GaussArray (iChan->TauValM,  iChan->nValM, iChan->ValM_stdev);
                    vTauVoltageM = RN->GaussArray (iChan->TauVoltM, iChan->nVoltM, iChan->VoltM_stdev);

                    nTauH        = iChan->nValH;
                    vTauValueH   = RN->GaussArray (iChan->TauValH,  iChan->nValH, iChan->ValH_stdev);
                    vTauVoltageH = RN->GaussArray (iChan->TauVoltH, iChan->nVoltH, iChan->VoltH_stdev);
//                  tauScaleH    = iChan->tauScaleH [0] + RN->Gauss () * iChan->tauScaleH [1];
        break;
    case FAM_KDR:    
        break;
    case FAM_KNICD:
        break;
    case FAM_KAHP:  Ca_Exp        = RN->GaussNum (iChan->CA_EXP);
                    Ca_Scale      = RN->GaussNum (iChan->CA_SCALE);
                    Ca_Half_Min   = RN->GaussNum (iChan->CA_HALF_MIN);
                    Ca_Tau_Scale  = RN->GaussNum (iChan->CA_TAU_SCALE);
        break;
  }
  
  m = RN->GaussNum (iChan->M_Initial);

  h = RN->GaussNum (iChan->H_Initial);

  ReversePot = RN->GaussNum (iChan->ReversePot);
}

Channel::~Channel ()
{   
  MEMFREEOBJ (MEM_KEY);
}

/*------------------------------------------------------------------------*/
//Added by C Wilson July, 2000
//called by the compartment - interfaces with the function pointer to getthe current values 

double Channel::GetCurrent (Compartment *Cmp) 
{
  double f;

  switch (family)
  {
    case FAM_NA:    f = CalculateCurrent_Na (Cmp);
        break;
    case FAM_CA:    f = CalculateCurrent_Ca (Cmp);
        break;
    case FAM_KM:    f = CalculateCurrent_Km (Cmp);
        break;
    case FAM_KA:    f = CalculateCurrent_Ka (Cmp);
        break;
    case FAM_KDR:   f = CalculateCurrent_Kdr (Cmp);
        break;
    case FAM_KNICD: f = CalculateCurrent_Knicd (Cmp);
        break;
    case FAM_KAHP:  f = CalculateCurrent_Kahp (Cmp);
        break;
    default:        f = 0.0;
  }
 
  return f;
}

/*------------------------------------------------------------------------*/

double Channel::CalculateCurrent_Na (Compartment *Cmp) //fast Na current
{
//double a_m, b_m, a_h, b_h, CmpV, dt;

// Activation

/*CmpV = Cmp->GetMembraneVoltage ();
  dt   = Cmp->GetDT ();
  
  a_m  =  (mMultiplier [0] * (CmpV + eHalfMinM [0]) /
          (1.0 - exp (-(CmpV + eHalfMinM [0]) / slopeFactorM [0])));
  b_m  = (-mMultiplier [1] * (CmpV + eHalfMinM [1]) /
          (1.0 - exp ((CmpV + eHalfMinM [1]) / slopeFactorM [1])));
  t_m  = tauMultiplierM [0] / (a_m + b_m );
  m_oo = t_m * a_m / 2;
  m   += (m_oo - m) * dt / t_m;

// Inactivation

  a_h  = (-hMultiplier [0] * (CmpV + eHalfMinH [0]) / 
  h_oo = 1/ (1 + exp ((CmpV - eHalfMinH [0]) / slopeFactorH [0]));

  b_h  = (hMultiplier [1]/ (1.0 + exp (-CmpV / slopeFactorH [1])));
  h_oo = a_h / (a_h + b_h );
  t_h  = tauMultiplierH [0] / (a_h + b_h );
  h += (h_oo - h) * dt / t_h;
  I = unitaryG * strength * pow (m, mPower) * pow (h, hPower)* (E - CmpV); */

  I = 0.0;
  return (I);
}

/*------------------------------------------------------------------------*/

double Channel::CalculateCurrent_Ca (Compartment *Cmp) //fast Ca current
{
//double a_m, b_m, a_h, b_h, CmpV, dt;

//Activation

/*CmpV = Cmp->GetMembraneVoltage ();
  dt   = Cmp->GetDT ();
      
  a_m  = (mMultiplier [0] * (CmpV + eHalfMinM [0]) / 
         (1.0-exp (- (CmpV + eHalfMinM [0]) / slopeFactorM [0])));
  b_m  = (-mMultiplier [1] * (CmpV + eHalfMinM [1]) / 
         (1.0 - exp ((CmpV + eHalfMinM [1]) / slopeFactorM [1])));
  t_m  = tauMultiplierM [0]/ (a_m + b_m );
  m_oo = t_m * a_m / 2.0;
  m   += (m_oo - m) * dt / t_m;

  //Inactivation

  a_h  = (-hMultiplier [0] * (CmpV + eHalfMinH [0]) / 
         (1.0-exp (- (CmpV+ eHalfMinH [0])/slopeFactorH [0])));
  b_h  = (hMultiplier [1] / (1.0 + exp (-CmpV / slopeFactorH [1])));
  h_oo = a_h / (a_h + b_h );
  t_h  = tauMultiplierH [0] / (a_h + b_h );
  h   += (h_oo - h) * dt / t_h;
  I    = unitaryG * strength * pow (m, mPower) * pow (h, hPower) * 
         (CmpV - 12.5 * log10 (Cmp->GetCalciumExternal_mean () / Cmp->GetCalciumInternal_mean ())); */

  I = 0.0;
  return (I);
}

/*------------------------------------------------------------------------*/
// DEFAULTS:
// unitaryG 5-18pS
// eHalfMinM [0] = 35
// slopeFactorM [0] = 40, [1] = 20, [2] = 10
// tauScaleFactor = 0.1 (or 1/10)

double Channel::CalculateCurrent_Km (Compartment *Cmp)
{
  double CmpV;
 
  CmpV = Cmp->GetMembraneVoltage ();

// Activation

  t_m = tauScaleM * 1.0 / (exp ((CmpV - eHalfMinM [0]) / slopeFactorM [0])
                        + exp (-(CmpV - eHalfMinM [0]) / slopeFactorM [1]));
  
  m_oo = 1.0 / (1.0 + exp (-(CmpV - eHalfMinM [0]) / slopeFactorM [2]));
  
  m += (m_oo - m) * Cmp->GetDT () / t_m;
  if (m < 0.0)
  {
    m = 0.0;
  }
  else if (m > 1.0)
  {
    m = 1.0;
  }
 
  I = unitaryG * strength * pow (m, mPower) * (ReversePot - CmpV);

  return (I);  
}

/*------------------------------------------------------------------------*/
//  DEFAULTS:
//  unitary channel conductance ~=5-23pS (22degC)
//  eHalfMinM [0] = 42, [1] = 110
//  slopeFactorM [0] = 13, [1] = 18
//  eHalfMinH [0] = 110
//  slopeFactorH [0] = 18
//  tauH = 50 when <-80 and 150 when greater

double Channel::CalculateCurrent_Ka (Compartment *Cmp)
{
  double CmpV, dt;

// Activation

/*  printf ("Channel::CalculateCurrent_Ka\n");  fflush (stdout); */

  CmpV = Cmp->GetMembraneVoltage ();
  dt = Cmp->GetDT ();

// The flag previously checked here is no longer used, so just call the function - JK   

  t_m  = GetTauM (CmpV); 

  m_oo = 1.0 / (1.0 + exp (-(CmpV - eHalfMinM [0]) / slopeFactorM [0]));
  m += (m_oo - m) * dt / t_m;
  if (m < 0.0)
  {
    m = 0.0;
  }
  else if (m > 1.0)
  {
    m = 1.0;
  }

  //Inactivation

  t_h  = GetTauH (CmpV);
  h_oo = 1/ (1 + exp ((CmpV - eHalfMinH [0]) / slopeFactorH [0]));
  h += (h_oo - h) * dt / t_h;

  if (h < 0.0)
  {
    h = 0.0;
  }
  else if (h > 1.0)
  {
    h = 1.0;
  }

  I = unitaryG * strength * pow (m, mPower) * pow (h, hPower) * (ReversePot - CmpV);

  return (I);
}

/*------------------------------------------------------------------------*/
// g = 1.17uS, m_oo_midpoint = -12.1mV, m_oo_^2_midpoint -1.6mV, h_oo_midpoint = -25mV
// unitary channel conductance ~= 20pS

double Channel::CalculateCurrent_Kdr (Compartment *Cmp)
{
/*double a_m, b_m, CmpV, dt;

  CmpV = Cmp->GetMembraneVoltage ();
  dt   = Cmp->GetDT ();
  
  //Activation

  a_m  = (-mMultiplier [0] * (CmpV + eHalfMinM [0]) / 
         (exp (-(CmpVage + eHalfMinM [0]) / slopeFactorM [0]) - 1.0));
  b_m  = (exp (- (CmpV + eHalfMinM [1])/slopeFactorM [1]));
  t_m  = tauMultiplierM [0] / (a_m + b_m );
  m_oo =  (-mMultiplier [0] * (CmpV + eHalfMinM [0] - 20.0) / 
          (exp (-(CmpV + eHalfMinM [0] - 20).0/slopeFactorM [0]) - 1.0)) / 
          ((-mMultiplier [0] * (CmpV + eHalfMinM [0] - 20.0) / 
          (exp (-(CmpV + eHalfMinM [0]) / slopeFactorM [0]) - 1.0)) + 
          (exp (-(CmpV + eHalfMinM [1])/slopeFactorM [1])));
  m   += (m_oo - m) * dt / t_m;

// Inactivation

  h_oo = 1.0 / (1.0 + exp ((CmpV + eHalfMinH [0]) / slopeFactorH [0]));
  t_h  =  (CmpV < -25.0) ? tauMultiplierH [0] : tauMultiplierH [1];
  h += (h_oo - h) * dt / t_h;
  I = unitaryG * strength * pow (m, mPower)*pow (h, hPower) * (CmpV - E); */

  I = 0.0;
  return (I); 
}

/*------------------------------------------------------------------------*/
// g = 1.2uS, unitary channel conductance ~=130-240pS

double Channel::CalculateCurrent_Knicd (Compartment *Cmp)
{
  double funct_m, b_m, CmpV;

// Activation

  CmpV = Cmp->GetMembraneVoltage ();

  funct_m = 250.0 * Cmp->GetCalciumInternal () * exp (CmpV / 24.0);
  b_m     = 0.1 * exp (-CmpV / 24.0);
  t_m     = 1 / (funct_m + b_m );
  m_oo    = funct_m / (funct_m + b_m);
  m      += (m_oo - m) * Cmp->GetDT () / t_m;
  if (m < 0.0)
  {
    m = 0.0;
  }
  else if (m > 1.0)
  {
    m = 1.0;
  }

  //  No inactivation equations (by definition)

  I = unitaryG * strength * pow (m, mPower) * pow (h, hPower) * (ReversePot - CmpV);

  return (I);
}

/*------------------------------------------------------------------------*/
// g = 0.054uS, m_oo_^2_midpoint = 220nM

double Channel::CalculateCurrent_Kahp (Compartment *Cmp)
{
  double funct_m; //local temp values

/*printf ("Channel::CalculateCurrent_Kahp\n");*/

  //Activation

  funct_m = Ca_Scale * pow (Cmp->GetCalciumInternal (), Ca_Exp); //based on shell/layers stuff

  t_m  = Ca_Tau_Scale / (funct_m + Ca_Half_Min);

  m_oo = funct_m / (funct_m + Ca_Half_Min);

  m += (m_oo - m) * Cmp->GetDT () / t_m;

  if (m < 0.0)
  {
    m = 0.0;
  }
  else if (m > 1.0)
  {
    m = 1.0;
  }

// No inactivation equations (by definition)
// where is the h coming from and why is it here, when it shouldnt be for ahp channel
 
   I = unitaryG * strength * pow (m, mPower) * (ReversePot - Cmp->GetMembraneVoltage ());

  return (I);
}

/*------------------------------------------------------------------------*/

double Channel::GetTauH (double V)
{
  int i;
  
  for (i = 0; i < nTauH; i++)
  {
    if (V < vTauVoltageH [i])
      return (vTauValueH [i]);
  }
  return (vTauValueH [i]); //if we get this far, then return the last Value
}

/*------------------------------------------------------------------------*/

double Channel::GetTauM (double V)
{
  int i;
  
  for (i = 0; i < nTauM; i++)
  {
    if (V < vTauVoltageM [i])
      return (vTauValueM [i]);
  }
  return (vTauValueM [i]);
}

/*------------------------------------------------------------------------*/
/* Returns number of items this channel's report reports                     */

int Channel::GetNumItems ()
{
  int num;

  switch (family)
  {
    case FAM_NA:    num = 0;
        break;
    case FAM_CA:    num = 0;
        break;
    case FAM_KM:    num = 5;
        break;
    case FAM_KA:    num = 8;
        break;
    case FAM_KDR:   num = 0;
        break;
    case FAM_KNICD: num = 0;
        break;
    case FAM_KAHP:  num = 7;
        break;
    default:        num = 0;
  }
  return (num);
}

/*------------------------------------------------------------------------*/

void Channel::Print ()
{
  printf ("Channel '%s', family '%s'\n", source->L.name, NameDefine (source->family));
  printf ("      Variable    nominal  actual\n");
  printf ("    mPower    %f   %f\n", source->mPower [0], mPower);
  printf ("    hPower    %f   %f\n", source->hPower [0], hPower);
  printf ("    unitaryG  %f   %f\n", source->unitaryG [0], unitaryG);
  printf ("    strength  %f   %f\n", source->strength [0], strength);

  printf ("    tauScaleM  %f   %f\n", source->tauScaleM);
//printf ("    tauScaleH  %f   %f\n", source->tauScaleH, tauScaleH);
                         
  printf ("    slopeM     %f   %f\n", source->slopeH [0], slopeFactorM [0]);
  printf ("    slopeH     %f   %f\n", source->slopeM [0], slopeFactorM [0]);

//printf ("    tauVoltageM, tauValueM: ";
//for (i = 0; i < nTauM; i++);
//  printf ("   %3d: %f  %f\n", i, vTauVoltageM [i], vTauValueM [i]);

//printf ("    tauVoltageH, tauValueH: ";
//for (i = 0; i < nTauH; i++);
//  printf ("   %3d: %f  %f\n", i, vTauVoltageH [i], vTauValueH [i]);

}

/*------------------------------------------------------------------------*/
/*
 My goal should be to compute how big an entire cluster is so that later, I can
 have some nodes skip over areas that don't belong to them. this should cut the amount of time
 it takes to load while allowing for any number of nodes to be used rather than
 forcing the user to use the same number as before
 */

int Channel::Save( FILE *out )
{
  int sum = 0;   //the final tally of bytes written
  short int helper;

  fwrite( this, sizeof( Channel ), 1, out );  //wastes 7 ptrs - 28 bytes

  //remember, there is one more value than voltage

  if( nTauM > 0 )
  {
    fwrite( vTauVoltageM, sizeof( double ), nTauM, out );
    fwrite( vTauValueM, sizeof( double ), nTauM+1, out );
  }
  if( nTauH > 0 )
  {
    fwrite( vTauVoltageH, sizeof( double ), nTauH, out );
    fwrite( vTauValueH, sizeof( double ), nTauH+1, out );
  }

  //slope factor also needs to be written.  different families will need more/fewer bytes
  helper = getSlopeFactorSize( 'h' );
  fwrite( slopeFactorH, sizeof( double ), helper, out );
  sum += sizeof( double ) * helper;

  helper = getSlopeFactorSize( 'm' );
  fwrite( slopeFactorM, sizeof( double ), helper, out );
  sum += sizeof( double ) * helper;

  sum += sizeof( Channel ) + sizeof( double ) * ( nTauM + nTauM + nTauH + nTauH + 2 );
  //return sum;

  return sum;
}

/*------------------------------------------------------------------------*/
//load a channel from a file into clean structure

void Channel::Load( FILE *in, char *nothing )
{
  int helper;
  fread( this, sizeof( Channel ), 1, in );
  source = NULL;                               //this may get assigned if the original
                                               // T_CHANNEL is loaded as well
  vTauValueM = vTauValueH = NULL;
  vTauVoltageM = vTauVoltageH = NULL;

  ///remember, there is one more value than voltage

  //allocate space and read data
  if( nTauM > 0 )
  {
    vTauVoltageM = (double *) realloc (vTauVoltageM, nTauM*sizeof (double));
    vTauValueM = (double *) realloc (vTauValueM, (1+nTauM)*sizeof (double));
    fread( vTauVoltageM, sizeof( double), nTauM, in);
    fread( vTauValueM, sizeof( double), nTauM+1, in);
  }
  if( nTauH > 0 )
  {
    vTauVoltageH = (double *) realloc (vTauVoltageH, nTauH*sizeof (double));
    vTauValueH = (double *) realloc (vTauValueH, (1+nTauH)*sizeof (double));
    fread( vTauVoltageH, sizeof( double), nTauH, in);
    fread( vTauValueH, sizeof( double), nTauH+1, in);
  }

  //slope factor - find out how many m and h slots this family has, allocate and read
  helper = getSlopeFactorSize( 'h' );
  slopeFactorH = (double *) malloc ( helper*sizeof( double) );
  fread( slopeFactorH, sizeof( double ), helper, in );
  helper = getSlopeFactorSize( 'm' );
  slopeFactorM = (double*) malloc ( helper*sizeof( double ) );
  fread( slopeFactorM, sizeof( double ), helper, in );
}

/*------------------------------------------------------------------------*/
//load channel from file over exising channel

void Channel::ChannelReset ( FILE *in,  char *nothing)
{
  Channel tempChannel;

  fread( &tempChannel, sizeof( Channel), 1, in);

  tempChannel.vTauValueM = vTauValueM;
  fread( vTauValueM, sizeof( double), nTauM, in);
  tempChannel.vTauVoltageM = vTauVoltageM;
  fread( vTauVoltageM, sizeof( double), nTauM, in);
  tempChannel.vTauValueH = vTauValueH;
  fread( vTauValueH, sizeof( double), nTauH, in);
  tempChannel.vTauVoltageH = vTauVoltageH;
  fread( vTauVoltageH, sizeof( double), nTauH, in);
  //all podoubleer done?
  *this = tempChannel;
}

short int Channel::getSlopeFactorSize( char particle )
{
  short int size = 0;

  if( particle == 'M' || particle == 'm' )
  {
    switch( family )
    {
      case FAM_NA:    size = 2;
          break;
      case FAM_CA:    size = 0;
          break;
      case FAM_KM:    size = 3;
          break;
      case FAM_KA:    size = 1;
          break;
      case FAM_KDR:   size = 0;
          break;
      case FAM_KNICD: size = 0;
          break;
      case FAM_KAHP:  size = 0;
          break;
      default:        size = 0;
    }
  }
  else if( particle == 'H' || particle == 'h' )
  {
       switch( family )
    {
      case FAM_NA:    size = 2;
          break;
      case FAM_CA:    size = 0;
          break;
      case FAM_KM:    size = 0;
          break;
      case FAM_KA:    size = 1;
          break;
      case FAM_KDR:   size = 0;
          break;
      case FAM_KNICD: size = 0;
          break;
      case FAM_KAHP:  size = 0;
          break;
      default:        size = 0;
    }
  }

  return size;
}


// these functions are from the book "Numercial Recipes in C"
// "'Minimal' random number generator of Park and Miller with Bays-Durham
// shuffle and added safeguards. Returns a uniform random deviate between 0.0
// and 1.0 (exclusive of the endpoint values). Call with IDUM a negative
// integer to initialize; thereafter, do not alter idum between
// successive deviates in a sequence. RNMAX should approximate the
// largest double value that is less than 1.0"

#include <stdio.h>
#include "Random.h"
#include "debug.h"
#include "memstat.h"

#define MEM_KEY  KEY_RANDOM

Random::Random ()
{
  MEMADDOBJ (MEM_KEY);
  j = k = iy = idum = 0;
  haveExtra = false;
  Extra = 0.0;
}

Random::~Random ()
{
  MEMFREEOBJ (MEM_KEY);
}

/*------------------------------------------------------------------------*/

void Random::SeedIt (int IDUM)
{
  idum = IDUM;

  if (-idum < 1) 
    idum = 1;                    //to prevent idum = zero
  else
    idum = -idum;

  for (j = NTAB+7; j >= 0; j--) //load the shuffle table after 8 warm ups
  {
    k = (idum) / IQ;
    idum = IA * (idum - k * IQ) - IR * k;
    if (idum < 0)
      idum += IM;
    if (j < NTAB)
      iv [j] = idum;
  }
  iy = iv [0];
}

/*------------------------------------------------------------------------*/

double Random::Rand ()
{
  double temp;

  //compute idum = (IA*idum)%IM without overflows by Schrage's method

  k = idum / IQ;
  idum = IA * (idum - k * IQ) - IR * k;
  if (idum < 0)
    idum += IM;

  j = iy / NDIV;         // will be in the range 0..NTAB-1
  iy = iv [j];           // Output previously stored value and refill shuffle table
  iv [j] = idum;

  if ((temp = AM * iy) > RNMAX)
    return (RNMAX);      // no endpiont values
  else
    return (temp);
}

/*------------------------------------------------------------------------*/

int Random::iRand ()
{
  k = idum / IQ;
  idum = IA * (idum - k * IQ) - IR * k;
  if (idum < 0)
    idum += IM;

  j = iy / NDIV;         // will be in the range 0..NTAB-1
  iy = iv [j];           // Output previously stored value and refill shuffle table
  iv [j] = idum;

  return (iy);
}

/*---------------------------------------------------------------------------*/
/* This function encapsulates the code to initialize a variable with a       */
/* random value uniformly distributed within a range.  input is a two        */
/* element array that is the start and end of the range.  It is assumed that */
/* input [0] < input [1]: this error check should be done in the input.      */

double Random::RandRange (double *input)
{
  double r, rn, del;

  rn = Rand ();
  del = input [1] - input [0];
  r = input [0] + rn * del;

  return (r);
}

/*---------------------------------------------------------------------------*/
/* This routine is from "Numerical Recipes in C".  Returns a normally        */
/* distributed deviate with zero mean and unit variance, using Rand as the   */
/* source of uniform deviates.  Each call actually creates two deviates: the */
/* extra is saved for the next call.                                         */

double Random::Gauss ()
{
  double fac, rsq, v1, v2, dev;

  if (haveExtra) 
  {
    haveExtra = false;
    dev = Extra;
  }
  else         //we have an extra deviate handy to return it and reset the flag
  {
    do
    {
      v1 = 2.0 * Rand () - 1.0;    // pick two uniform numbers in the square
      v2 = 2.0 * Rand () - 1.0;    // extending from -1 to +1 in each direction
      rsq = v1 * v1 + v2 * v2;     // see if they are unit circle
    }
    while (rsq >= 1.0 || rsq == 0.0);    // and if they are not, try again

    fac = sqrt (-2.0 * log (rsq) / rsq);

    //now make the Box-Muller transofrmation to get two normal deviates
    //Return one and save the other for the next time

    Extra = v1 * fac;
    dev   = v2 * fac;
    haveExtra = true;            
  }
  return (dev);
}

/*---------------------------------------------------------------------------*/
/* This function encapsulates the code to initialize a variable with a       */
/* gaussian random distribution.  Most variables that can have a gaussian    */
/* applied have a common format: mean and standard deviation in a            */
/* two-element array.  (If stdev is zero, result is just the mean.)          */

double Random::GaussNum (double *input)
{
  double result, fac, rsq, v1, v2;

  if (input [1] == 0.0)
    result = input [0];
  else
  {
    if (haveExtra)
    {
      haveExtra = false;
      result = input [0] + input [1] * Extra;
    }
    else         
    {
      do
      {
        v1 = 2.0 * Rand () - 1.0;  // pick two uniform numbers in the square
        v2 = 2.0 * Rand () - 1.0;  // extending from -1 to +1 in each direction
        rsq = v1 * v1 + v2 * v2;   // see if they are unit circle
      }
      while (rsq >= 1.0 || rsq == 0.0);   // and if they are not, try again
  
      fac = sqrt (-2.0 * log (rsq) / rsq);
      result = input [0] + input [1] * v1 * fac;
      Extra  = v2 * fac;
      haveExtra = true;
    }
  }
  return (result);
}

/*---------------------------------------------------------------------------*/

float Random::GaussNum( float* input )
{
  double temp[2];

  temp[0] = input[0];
  temp[1] = input[1];
  return (float) GaussNum( temp );
}

/*---------------------------------------------------------------------------*/
/* Given an array of numbers and a standard deviation, applies gaussian      */
/* function to them.  If the standard deviation is zero, simply returns a    */
/* pointer to a the original array, otherwise allocates a new array and      */
/* returns a pointer to it.  (This is to save memory.)                       */

double *Random::GaussArray (double *array, int num, double stdev)
{
  double *out, fac, rsq, v1, v2;
  int i, num2;

  if (stdev == 0.0)
    out = array;
  else
  {
    out = (double *) malloc (num * sizeof (double));

/* Since the gaussian function generates pairs of deviates, this fills in    */
/* the array up to an even number by pairs, then handles the possible last   */
/* member separately.                                                        */

    num2 = (num / 2) * 2;
    for (i = 0; i < num2; i += 2)
    {
      do
      {
        v1 = 2.0 * Rand () - 1.0;  // pick two uniform numbers in the square
        v2 = 2.0 * Rand () - 1.0;  // extending from -1 to +1 in each direction
        rsq = v1 * v1 + v2 * v2;   // see if they are unit circle
      }
      while (rsq >= 1.0 || rsq == 0.0);   // and if they are not, try again
  
      fac = sqrt (-2.0 * log (rsq) / rsq);
      out [i]   = array [i]   + stdev * v1 * fac;
      out [i+1] = array [i+1] + stdev * v2 * fac;
    }

    if (num % 2 == 1)
    {
      if (haveExtra)
      {
        haveExtra = false;
        out [i] = array [i] + stdev * Extra;
      }
      else         
        out [i] = array [i] + stdev * Gauss ();
    }
  }
  return (out);
}

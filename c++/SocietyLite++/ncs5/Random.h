#ifndef RANDOM_H
#define RANDOM_H

#include <stdlib.h>
#include <math.h>

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+ (IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMAX (1.0-EPS)

/**
 * Encapsule the functions from the book "Numercial Recipes in C"
 * 'Minimal' random number generator of Park and Miller with Bays-Durham
 * shuffle and added safeguards. Returns a uniform random deviate between 0.0
 * and 1.0 (exclusive of the endpoint values). Call with IDUM a negative
 * integer to initialize; thereafter, do not alter idum between
 * successive deviates in a sequence. RNMAX should approximate the
 * largest double value that is less than 1.0"
 */
class Random
{
 private:
  //random number generator parameters, this is for a uniform deviate generator
  int j;
  int k;
  int iy;
  int iv [NTAB];
  int idum;

// these parameters are for the gaussian generator
  ///An extra number for a guassian generator is pending for use
  bool haveExtra;
  ///The extra guassian number
  double Extra;

 public:
  Random ();
  ~Random ();

  /**
   * Seed the random number generator.  Note that if a negative number is not sent for
   * the seed value, it will default to one.
   * @param seed The value to use for idum; must be negative.
   */
  void SeedIt (int seed);
  /**
   * Request a double precision value between 0 and 1
   * @return Value between zero and one (exclusive)
   */
  double Rand ();
  /**
   * Request an integer value from 0 to integer max-1.  The user should use the modulus
   * operator to limit the number to the appropriate range. ex. Rand()%20 for 0 to 19
   * @return Value between 0 and integer max-1
   */
  int iRand ();

  /**
   * This function encapsulates the code to initialize a variable with a
   * random value uniformly distributed within a range.  input is a two
   * element array that is the start and end of the range.  It is assumed that
   * input [0] < input [1]: this error check should be done in the input.
   * @param input Two-element array containing the minimum value and maximum value
   * @return A value that exists between the minimum and maximum value
   */
  double RandRange (double *input);

  /**
   * This routine is from "Numerical Recipes in C".  Returns a normally
   * distributed deviate with zero mean and unit variance, using Rand as the
   * source of uniform deviates.  Each call actually creates two deviates: the
   * extra is saved for the next call.
   * @return The value selected as the next random number
   */
  double Gauss ();

  /**
   * This function encapsulates the code to initialize a variable with a
   * gaussian random distribution.  Most variables that can have a gaussian
   * applied have a common format: mean and standard deviation in a
   * two-element array.  (If stdev is zero, result is just the mean.)
   * @param input Two-element array containing the mean and standard deviation
   * @return the resulting value (mean + Guass()*stddev)
   */
  double GaussNum (double *input);

 /**
  * Overloaded version of double GaussNum.  This function merely typecasts the
  * float parameters into double containers so that it can call the original
  * GaussNum function.  The result is then typecast back to a float and returned.
  *
  * @param input Two-element float array contain the mean and standard deviation
  * @return The resulting value (mean + Guass()*stddev)
  */
  float GaussNum( float *intpu );

 /**
  * Given an array of numbers and a standard deviation, applies gaussian
  * function to them.  If the standard deviation is zero, simply returns a
  * pointer to a the original array; otherwise, allocates a new array and
  * returns a pointer to it.  (This is to save memory.)
  * @param array The original array of numbers
  * @param num Number of elements in the array
  * @param stdev The standard deviation that will be applied to each element of the array
  * @return Pointer to the array with the final values
  */
  double *GaussArray (double *array, int num, double stdev);
};
#endif

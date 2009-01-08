#ifndef CIRCLELIST
#define CIRCLELIST

//#include <vector.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//using namespace std;

/**
 * encapsulate a double, so that it can change later with minimal fuss
 */
struct CmpVoltage
{
  ///membrane voltage
  double MVoltage;
};

/**
 * Encapsulate an array to act as a circular list
 */
class CircleList
{
  private:
    CmpVoltage* clist;

    ///index of the element 
    long start;

    ///index of last element
    long end;

    ///the number of elements used in the list - may not be needed
    long used;

    ///the number of allocated units int the array
    long maxSize;

  public:
    ///default constructor.  Creates a circular list with capacity of 10 elements
    CircleList();

    /**
     * constructor.  Creates a circular list and sets initial size of list
     * 
     * @param size number of elements to allocate for circular list
     */
    CircleList( int maxsize );

    ///destructor
    ~CircleList();

    ///get the maximum size of the circular list
    unsigned int capacity();

    ///get the current number of elements in the circular list
    unsigned int size();

    /**
     * insert an element onto the end of the circular list
     *
     * @param data the data to be inserted into the list
     */
    void push_back( CmpVoltage data );

    /**
     * insert an element onto the start of the circular list
     *
     * @param data the data to be inserted into the list
     */
    void push_front( CmpVoltage data );

    /**
     *  retreive data at specified location
     *
     *  @param offset number of elements from start of circular list; accepts pos or neg values; wraps around list as necessary
     *  @return a copy of the data at the location
     */
    CmpVoltage at( int offset );

    /**
     *  resize the circular list to have a differnet capacity
     */
    int resize( int maxsize );

    /**
     *  write circle list data to a file (buffer?)
     *  @param out pointer to destination file
     *  @return the number of bytes written
     */
    int Save( FILE *out );

    /**
     *  Read in circle list data from a file (buufer?)
     *  @param in source file containing data
     *  @param nothing pointer to character string, not used but passed for consistency
     */
    void Load( FILE *in, char *nothing );
};

#endif

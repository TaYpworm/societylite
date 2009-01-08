//Jim King
//serverconsts.h
//general location for constants to be used throughout the program

#ifndef CONSTANTS
#define CONSTANTS

const int MaxLength = 1024;

//Path to mpich, ethernet and myrinet versions
const char mpich[2][1024] = { "/opt/mpich/gnu/bin/mpirun -nolocal",
                              "/opt/mpich/myrinet/gnu/bin/mpirun" };

#endif


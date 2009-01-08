#ifndef AUGMENTATION_H
#define AUGMENTATION_H

#include "InitStruct.h"
#include "Random.h"
#include <deque>

using namespace std;

/**
 * Provide a Synapse with additional parameters controlling facilitation.
 * Specifically intended to help represent the pre-frontal cortex area, synaptic
 * augmentation defines Calsium properties for a synapse.  These will affect
 * facilitation of a synapse by making it able to recover faster, and spike
 * higher than normal.
 */
class Augmentation
{
    public:
                ///The last TimeStep this Augmentation object updated values (via spike arrival or reporting)
        int lastUpdate;
                ///Level of augentation provided to the parent synapse
        double augmentation;
                ///Decay rate of the augmentation as time progresses
        double augmentation_Tau;
                ///Level of Calcium Concentration in the Synapse
        double Calcium;
                ///Time Constant determining rate of decrease of Calcium each timestep
        double Calcium_Tau;
                ///Maximum Synaptic Augmentation that can occur no matter how much Calcium exists
        double MaxSA;
                /**
                 * When Calcium concentrations are high, a Decay > 0 will cause the Calsium_Tau to become
                 * shorter, so that CA will drop faster
                 */
        double Calcium_TauDecay;
                ///Amount to increment Calcium whenever a spike wave occurs on the Synapse
        double Calcium_Increment;
                ///Scaler value modifying Calcium increment (necessary?)
        double Alpha;
        int previousTime;
                ///Time at which a spike arrived, so that Calcium can be incremented after a delay
        deque<int> PendingCalcium;
                ///Calcium concentrations to be used in future Augmentation updates
        deque<double> CalciumForAugmentation;
                ///Time when the calcium concentrations need to be used to update Augmentation
        deque<int> PendingAugmentationFromCalcium;
                ///Number of timesteps to delay the increment of calcium caused by a spike
        const static int CalciumDelay = 5;
                ///Number of timesteps to delay the use of Calcium to update Augmentation
        int augmentationDelay;

    public:
        /**
         * Default Constructor creates augmentation object.  This should be used
         * during a Brain load, so that the structure is created, and then the 
         * member data can be filled by a call to the load function
         */
        Augmentation();

        /**
         * Constructor creates a synaptic augmentation object from Input file
         * to be stored inside a synapse.
         *
         * @param iAugmentation Defining input object with parameters used to create this Augmentation object
         * @param RN Random Number generator so that any mean/standard deviation pairs can be combined
         */
        Augmentation( T_SYNAUGMENTATION *iAugmentation, Random *RN );
    
        /**
         * Compute the synaptic augmentation for the timestep.
         *
         * @param TimeStep The current TimeStep of the simulation
         * @param deltaT The number of timesteps that have elapsed since the previous spike
         * @return The value of Synaptic Augmentation
         */
        double getAugmentation( int TimeStep, int deltaT );

        /**
         * 
         */
        void update( int TimeStep );

        /**
         * Write Augmentation object to disk.  Information related to timing will be
         * written out based on time differences.  For example, lastUpdate keeps track
         * of the TimeStep when the Augmentation object wa last updated.  After a load,
         * that specific timestep would be meaningless.  By storing the difference between
         * lastUpdate and the current Timestep, that information could be used after a load.
         * @param out Destination File
         * @param TimeStep current TimeStep of simulation
         * @return Number of bytes written
         */
        int Save( FILE *out, int TimeStep );

        /**
         * Read Augmentation object from disk.  Timing information will need to be treated
         * as negative.  For example, if the lastUpdate of the augmentation object had been
         * at time 1234 in the last simulation, and saved at time 1434, a time of 200 was saved.
         * After loading, TimeStep restarts at zero (0) instead of 1434, so the time of lastUpdate
         * must be -200.
         * @param in Source File
         * @param nothing Character pointer to serve as place holder (may not be used)
         */
        void Load( FILE *in, char *nothing );
};

#endif

//Synaptic Augmentation implementation

#include "Augmentation.h"
#include "Managers.h"

Augmentation::Augmentation()
{
   //wait for load function
}

//----------------------------------------------------------------------

Augmentation::Augmentation( T_SYNAUGMENTATION *iAugmentation, Random *RN )
{
    augmentation = RN->GaussNum( iAugmentation->Augmentation_init )-1;
      if( augmentation < 0.0 )
        augmentation = 0.0;
    augmentation_Tau = NI->FSV*RN->GaussNum( iAugmentation->Augmentation_tau );
    Calcium = RN->GaussNum( iAugmentation->CA_init );
    Calcium_Tau = NI->FSV*RN->GaussNum( iAugmentation->CA_tau );
    Calcium_TauDecay = RN->GaussNum( iAugmentation->CA_decay );
    MaxSA = RN->GaussNum( iAugmentation->MaxSA )-1.0;
      if( MaxSA < 0.0 )
        MaxSA = 0.0;   //remember, add 1 in getAugmentation function
    Calcium_Increment = RN->GaussNum( iAugmentation->CA_increment );
    Alpha = RN->GaussNum( iAugmentation->Alpha );
    previousTime = 0;
    lastUpdate = 0;
    
    augmentationDelay = (int) (NI->FSV * RN->GaussNum( iAugmentation->SA_delay ));
}

//----------------------------------------------------------------------

double Augmentation::getAugmentation( int TimeStep, int deltaT )
{
    update( TimeStep );

    //This spike's calcium will enter the synapse after a delay
    PendingCalcium.push_back( TimeStep+CalciumDelay );

    return 1+augmentation;
}

//---------------------------------------------------------------------------

void Augmentation::update( int TimeStep )
{
    double increase;
    double activeCAtau = Calcium_Tau;
    int lastTime=0, nextTime=0;
    
    if( lastUpdate >= TimeStep ) //already been updated
        return;

    if( Calcium > 0.0 )
       activeCAtau = Calcium_Tau/(1.0 + Calcium_TauDecay/Calcium );

    //adjust calcium to account for lost time
      //The value of calcium will affect Augmentation after a delay
    if( PendingCalcium.size() > 0 && PendingCalcium.front() <= TimeStep )
    {
      lastTime = lastUpdate;

      while( PendingCalcium.size() > 0 && PendingCalcium.front() <= TimeStep )
      {
        //we are integrating a calcium -> we will need to add an SA onto it's queue(deque)
        
        nextTime = PendingCalcium.front();
          PendingCalcium.pop_front();

          //decay
        Calcium *= exp( -(nextTime-lastTime)/activeCAtau ); //Calcium_Tau );
          //increase
        Calcium += Calcium_Increment;

        if( Calcium <= 0.0 )
          Calcium = 0.0;

        PendingAugmentationFromCalcium.push_back( lastTime + augmentationDelay );
        CalciumForAugmentation.push_back( Calcium );

        lastTime = nextTime;
      }

      //decay any remainder
      Calcium *= exp( -(TimeStep-lastTime)/activeCAtau );
    }
    else //just decay calcium
        Calcium *= exp( -(TimeStep-lastUpdate)/activeCAtau );

    //adjust augmentation, but do it one spike at a time so that Calcium's effect
      //on augmentation is not decayed too rapidly
    if( PendingAugmentationFromCalcium.size() > 0 && 
        PendingAugmentationFromCalcium.front() <= TimeStep )
    {
      lastTime = lastUpdate;

      //allow any pending Calcium increments to occur
      while( PendingAugmentationFromCalcium.size() > 0 && PendingAugmentationFromCalcium.front() <= TimeStep )
      {
        nextTime = PendingAugmentationFromCalcium.front();
          PendingAugmentationFromCalcium.pop_front();
        //if( PendingAugmentationFromCalcium.size() > 0 && 
          //  PendingAugmentationFromCalcium.front() <= TimeStep )
          //nextTime = PendingAugmentationFromCalcium.front();
        //else
          //nextTime = TimeStep;

        //increase first, then determine increase after decay
        augmentation *= exp(-(nextTime-lastTime)/augmentation_Tau);

        increase = (MaxSA-augmentation)*(CalciumForAugmentation.front()*Alpha);
          CalciumForAugmentation.pop_front();
        augmentation += increase;

        if( augmentation > MaxSA )
          augmentation = MaxSA;

        lastTime = nextTime;
      }

      //final decay
      augmentation *= exp(-(TimeStep-lastTime)/augmentation_Tau);
    }
    else  //just need to decay augmentation
        augmentation *= exp(-(TimeStep-lastUpdate)/augmentation_Tau);

    if( augmentation > MaxSA )
      augmentation = MaxSA;

    lastUpdate = TimeStep;
}

//---------------------------------------------------------------------------
// Save / Load functions
//---------------------------------------------------------------------------

int Augmentation::Save( FILE *out, int TimeStep )
{
    int bytes = 0;
    const int dSize = 8;
    const int iSize = 4;
    double dHolder[dSize], *dTemp;
    int iHolder[iSize], *iTemp;
    int i;

    //four integers: lastUpdate, augmentationDelay, and size of data deques
    i = 0;
    iHolder[i++] = lastUpdate - TimeStep;
    iHolder[i++] = augmentationDelay;
    iHolder[i++] = PendingCalcium.size();
    iHolder[i++] = PendingAugmentationFromCalcium.size();
      fwrite( iHolder, sizeof(int), i, out );
      bytes += sizeof(int)*i;
  
    //doubles
    i=0;
    dHolder[i++] = augmentation;
    dHolder[i++] = augmentation_Tau;
    dHolder[i++] = Calcium;
    dHolder[i++] = Calcium_Tau;
    dHolder[i++] = MaxSA;
    dHolder[i++] = Calcium_TauDecay;
    dHolder[i++] = Calcium_Increment;
    dHolder[i++] = Alpha;
      fwrite( dHolder, sizeof(double), i, out );
      bytes += sizeof(double)*i;

    //contents of pending data arrays
    if( iHolder[2] + iHolder[3] > 0 )
    {
        iTemp = new int[ iHolder[2] + iHolder[3] ];

        i = 0;
        for( deque<int>::iterator it = PendingCalcium.begin(); it != PendingCalcium.end(); it++ )
            iTemp[i++] = *it - TimeStep;
        for( deque<int>::iterator it = PendingAugmentationFromCalcium.begin(); it != PendingAugmentationFromCalcium.end(); it++ )
            iTemp[i++] = *it - TimeStep;
        fwrite( iTemp, sizeof( int ), i, out );
        bytes += sizeof( int ) * i;

        delete [] iTemp;
    }

    //  (CalciumForAugmentation and PendingAugmentationFromCalcium are the same
    //  size, so they can use the same entry from iHolder
    if( iHolder[3] > 0 )
    {
        dTemp = new double[ iHolder[3] ];

        i=0;
        for( deque<double>::iterator it = CalciumForAugmentation.begin(); it != CalciumForAugmentation.end(); it++ )
            dTemp[i++] = *it;
        fwrite( dTemp, sizeof( double ), i, out );
        bytes += sizeof( double ) * i;

        delete [] dTemp;
    }

    return bytes;
}

void Augmentation::Load( FILE *in, char *nothing )
{
    const int iSize = 4;
    const int dSize = 8;
    int iHolder[iSize], *iTemp;
    double dHolder[dSize], *dTemp;
    int i;

    //ints (last two are deque sizes)
    i = 0;
    fread( iHolder, sizeof(int), iSize, in );
    lastUpdate = iHolder[i++];
    augmentationDelay = iHolder[i++];
    
    //doubles
    i=0;
    fread( dHolder, sizeof(double), dSize, in );
    augmentation = dHolder[i++];
    augmentation_Tau = dHolder[i++];
    Calcium = dHolder[i++];
    Calcium_Tau = dHolder[i++];
    MaxSA = dHolder[i++];
    Calcium_TauDecay = dHolder[i++];
    Calcium_Increment= dHolder[i++];
    Alpha = dHolder[i++];

    //populate the deques - These have time info, needs to adjust for new TimeStep
    if( iHolder[2] + iHolder[3] > 0 )
    {
      iTemp = new int[ iHolder[2] + iHolder[3] ];
      fread( iTemp, sizeof(int), iHolder[2] + iHolder[3], in );
 
      i = 0;
      for( int j=0; j<iHolder[2]; j++ )
        PendingCalcium.push_back( iTemp[i++] );
      for( int j=0; j<iHolder[3]; j++ )
        PendingAugmentationFromCalcium.push_back( iTemp[i++] );

      delete [] iTemp;
    }
    if( iHolder[3] > 0 )
    {
      dTemp = new double[ iHolder[3] ];
      fread( dTemp, sizeof(double), iHolder[3], in );

      for( i=0; i<iHolder[3]; i++ )
        CalciumForAugmentation.push_back( dTemp[i] );

      delete [] dTemp;
    }
} 


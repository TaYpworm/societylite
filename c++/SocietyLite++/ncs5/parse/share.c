//share.c

//These functions are accessed by both ConvertIn.c and ../LoadStruct.cpp
//This common file allows both to receive changes without making those
//changes in two different locations


void buildCellCmpConnects( T_CELL *Cel )
{
    double xdiff, ydiff, zdiff, distance;
    int n, *tempforward, *tempreverse;

    if (Cel->nConnect <= 0)                //Any Compartment connections present?
      return;

    //further preprocessing:
        //each compartment needs to calculate the longest delay it must wait -
        //this determines how long its previous state circle buffer will be
        //forward connections flow out of the active cmp into another cmp (source to dest)
        //reverse connections flow into the active cmp from another cmp (dest from source)
        //both need to be kept tracked of

    if( Cel->nCmp > 0 )
    {
        //counters to keep track of totale number of connections
        Cel->nforward = ( int * ) calloc ( Cel->nCmp, sizeof( int ) );
        Cel->nreverse = ( int * ) calloc ( Cel->nCmp, sizeof( int ) );

        //temporary counters to keep track of the most recently added connection
        tempforward = ( int * ) calloc ( Cel->nCmp, sizeof( int ) );
        tempreverse = ( int * ) calloc ( Cel->nCmp, sizeof( int ) );

        //allocate the array of arrays of pointers - one array per compartment
        Cel->forwardConn = ( T_CMPCONNECT *** ) calloc ( Cel->nCmp, sizeof( T_CMPCONNECT** ) );
        Cel->reverseConn = ( T_CMPCONNECT *** ) calloc ( Cel->nCmp, sizeof( T_CMPCONNECT** ) );
    }

    //intitial scan to count connections
    //for each connection, increment the appropriate values in nforward, nreverse
    for( n=0; n<Cel->nConnect; n++ )
    {
        Cel->nforward[ Cel->Connect[n]->FromCmp ]++;
        Cel->nreverse[ Cel->Connect[n]->ToCmp ]++;
    }

    //allocate the array of pointers
    for( n=0; n<Cel->nCmp; n++ )
    {
        if( Cel->nforward[n] > 0 )
            Cel->forwardConn[n] = ( T_CMPCONNECT ** ) calloc ( Cel->nforward[n], sizeof( T_CMPCONNECT* ) );
        if( Cel->nreverse[n] > 0 )
            Cel->reverseConn[n] = ( T_CMPCONNECT ** ) calloc ( Cel->nreverse[n], sizeof( T_CMPCONNECT* ) );
    }

    //configure pointers
    for( n=0; n<Cel->nConnect; n++ )
    {
        Cel->forwardConn[ Cel->Connect[n]->FromCmp ][ tempforward[ Cel->Connect[n]->FromCmp ]++ ] = Cel->Connect[n];
        Cel->reverseConn[ Cel->Connect[n]->ToCmp ][ tempreverse[ Cel->Connect[n]->ToCmp ]++ ] = Cel->Connect[n];

        //should I calculate distance at this moment and store the value?
        xdiff = Cel->Xpos[ Cel->Connect[n]->FromCmp ] - Cel->Xpos[ Cel->Connect[n]->ToCmp ];
        ydiff = Cel->Ypos[ Cel->Connect[n]->FromCmp ] - Cel->Ypos[ Cel->Connect[n]->ToCmp ];
        zdiff = Cel->Zpos[ Cel->Connect[n]->FromCmp ] - Cel->Zpos[ Cel->Connect[n]->ToCmp ];
        distance = sqrt( xdiff*xdiff + ydiff*ydiff + zdiff*zdiff );

        //double delay = distance / Cel->Connect[n]->Speed;  //delay is in seconds, need to convert to ticks

        Cel->Connect[n]->delay = distance / (Cel->Connect[n]->Speed*1000);   //speed in mm/ms -> mm/s
    }

    //free temp counter arrays
    free( tempforward );
    free( tempreverse );
}


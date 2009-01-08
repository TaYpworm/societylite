/*
 *  main.c
 *  Thesis
 *
 *  Created by James King on Sun Feb 15 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */
 
    //create a socket that listens on port xxxx
    //when someone connects, they should send info either:
     //requesting to connect to a existing port or
     //the creation of a new port

//@Include: servertype.h
//@Include: manage_server.h

#include <fstream>
#include <iostream>
#include <vector>
#include <queue>
#include "server_listen.h"
#include "manage_server.h"
#include "servertype.h"

#ifdef QQ_ENABLE
  #include <mpi.h>
  #include "QQ.h"

  int DB_Node = 0;
#endif

using namespace std;

int configureServers( char* &file, vector<Server> &writeServers, vector<Server> &readServers );

int main( int argc, char **argv )
{
  int errorNum;

#ifdef QQ_ENABLE
  int node, nodes;

  //need MPI initializations called
  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &node);
  MPI_Comm_size (MPI_COMM_WORLD, &nodes);

  cerr<<"nNodes: "<<nodes<<" my rank: "<<node<<endl;
#endif

    int globalVerbose = 0;
    int i = 0;
    char *arg=NULL;

	//open config file (argv[1]) and parse it to find
	//port numbers, and data managers(ascii vs binary)
    int serverSocket = -1;
    int serverPort = 20001;
	vector <Server> writeServers;  //clients will conect to write data
	vector <Server> readServers;   //clients will conect to read data

    i = 1;
    while (i < argc)
    {
        arg = argv [i];
        if (*arg == '-')
        {
            switch (*(arg+1))
            {
                case 'p': i++;
                    if ( i > (argc - 1) ) 
                    {
                        cerr<<"Please specify the port number."<<endl;
                        exit(0);
                    }   
                    serverPort = atoi( argv[i] );//20001;
                    if (serverPort == 0)
                    { 
                        cerr<<"Please specify a number with the '-p' option."<<endl;
                        exit(0);
                    }
                    break;

                case 'f': i++;
                    if ( i > (argc - 1) )
                    {
                        cerr<<"Please specify the file name."<<endl;
                        exit(0);
                    }
                    if( configureServers( argv[i], writeServers, readServers ) < 0 )
                    { 
                        cerr<<"Error: Counld not configure Servers from the config file specified."<<endl;
                        exit(0);
                    }
                    break;

                case 'v': globalVerbose = 1;
                    break; 

                case 'h':              
                    cerr<<"Example inputs:"<<endl;
                    cerr<<"  server -p 20003"<<endl;
                    cerr<<"  server -f config.txt"<<endl;
                    cerr<<"  server -p 20001 -f conf.txt"<<endl;
                    cerr<<"  server -p 20003 -f config.txt -v"<<endl; 
                    cerr<<"  server -v"<<endl<<endl;
                    cerr<<"Valid options:"<<endl;
                    cerr<<"  -p arg        : specify a port number arg"<<endl;
                    cerr<<"                  without specified port number, the default one will be 4694"<<endl;
                    cerr<<"  -f arg        : specify a configuration file arg"<<endl;   
                    cerr<<"  -v            : print extra information"<<endl;
                    cerr<<"  -h            : help"<<endl;
                    exit(0);
                    break;

                default: cerr<<"Type 'server -h' for usage."<<endl;
                    exit(0);
                    break;
            }
        }
        else
        {
            cerr<<"Type 'server -h' for usage."<<endl;
            exit(0);  
        }
        i++;
    }
  
    if( (errorNum = serverSocket = startServer( serverPort )) < 0 )
        cerr<<errorNum<<": Error opening command port; continuing without it\n";
        //now listen on socket forever
    cerr<<"Waiting on port "<<serverPort<<" for connections\n";

    //start up each server
    for( int index=0; index<writeServers.size(); index++ )
    {
	cerr<<"Opening ports "<<writeServers[index].port<<" and "<<readServers[index].port<<endl;
	//start up the writer port
	writeServers[index].socketHandle = startServer( writeServers[index].port );
	
	//start up the reader port
	readServers[index].socketHandle = startServer( readServers[index].port );
	
	//confirm no errors
	if( writeServers[index].socketHandle < 0 ||
		readServers[index].socketHandle < 0 )
	{
  	    cerr<<"Error opening connection using ports "
			<<writeServers[index].port<<" "<<readServers[index].port<<endl;
	    exit( 0 );
        }
    }

    //QQInit( 1000 );
	
    manageServer( serverSocket, writeServers, readServers, globalVerbose );
    
    return 0;
}

int configureServers( char* &file, vector<Server> &writeServers, vector<Server> &readServers )
{
	//open file, parse contents for socket info
	//format:
	
	//N
	//inputPort1 inputFormat outputPort1 outputFormat
	//inputPort2 inputFormat outputPort2 outputFormat
	//...
	//inputPortN inputFormat outputPortN outputFormat
	
	int connectionCount;
	Server currentServer;
	ifstream fin( file );
	
	if( !fin )  //opened file alright?
	{
		cerr<<"Error: Could not open configuration file "<<file<<endl;
		return -1;
	}
	
	//how many connections?
	fin>>connectionCount;
	for( int index=0; index<connectionCount; index++ )
	{
		//setup the next writeServer
		fin>>currentServer.port;
		fin>>currentServer.dataFormat;
            if( currentServer.dataFormat == 'A' || currentServer.dataFormat == 'B' )
            {
                cerr<<"port "<<currentServer.port<<" is verbose\n";
                currentServer.setVerbose( true );
            }
            else
                currentServer.setVerbose( false );
		writeServers.push_back( currentServer );
		
		//setup the next readServer
		fin>>currentServer.port;
		fin>>currentServer.dataFormat;
            if( currentServer.dataFormat == 'A' || currentServer.dataFormat == 'B' )
            {
                cerr<<"port "<<currentServer.port<<" is verbose\n";
                currentServer.setVerbose( true );
            }
            else
                currentServer.setVerbose( false );
		readServers.push_back( currentServer );
	}
	
	return 0;
}

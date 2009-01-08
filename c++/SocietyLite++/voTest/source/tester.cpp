/*
 * tester.cpp
 *
 *  Created on: Dec 27, 2008
 *      Author: thibec
 */

#include "tester.h"

using namespace std;

// ----------------------------------------------------------------------------
//							Public Functions
// ----------------------------------------------------------------------------
Tester::Tester(User_Input *inputPtr) {
	_input = inputPtr;
}

Tester::~Tester(){

}

// ----------------------------------------------------------------------------
//							Protected Functions
// ----------------------------------------------------------------------------
void Tester::bail(const char *on_what) {
    fputs(strerror(errno),stderr);
    fputs(": ",stderr);
    fputs(on_what,stderr);
    fputc('\n',stderr);
    exit(1);
}

int Tester::establishConnection(struct sockaddr_in *adr_srvrPtr) {
	int 	len_inet;               	/* length  */
	int 	s;							/* Socket */
	int 	z;
	struct 	sockaddr_in adr_srvr;
	struct hostent *hp = NULL;
	string	hostAddr;
	const char	*host;

	adr_srvr = *adr_srvrPtr;
	memset(&adr_srvr,0,sizeof adr_srvr);

	adr_srvr.sin_family = AF_INET;
	adr_srvr.sin_port = htons(atoi((_input->getPort()).c_str()));
	host = (_input->getHost()).c_str();

	if ((isdigit(host[0]))) {
		adr_srvr.sin_addr.s_addr = inet_addr(host);
		if ( adr_srvr.sin_addr.s_addr == INADDR_NONE )
				bail("bad address (Numeric).");
	} else {
		hp = gethostbyname((_input->getHost()).c_str());
		if ( !hp )
			bail("bad address.");
		if ( hp->h_addrtype != AF_INET )
			bail("bad address (Hostname).");
		adr_srvr.sin_addr = *(struct in_addr *)hp->h_addr_list[0];
	}

	len_inet = sizeof adr_srvr;

	//Create a TCP/IP socket to use :
	s = socket(PF_INET,SOCK_STREAM,0);
	if ( s == -1 )
		bail("socket()");

	// Connect to the server:
	z = connect(s, (const sockaddr*)&adr_srvr, len_inet);
	if ( z == -1 )
		bail("connect(2)");

	#if DEBUG_NETWORK
		 cout << "\n---- Established Network Connection ----"  << endl;
	#endif

	return s;
}

int Tester::getTransmitStream(int s, FILE **tx) {
	int retval = SUCCESS;
	(*tx) = fdopen(s,"w");

	if ( !(*tx) ) {
		close(s);  /* Failed */
		retval = FAIL;
	} else {
		setbuf((*tx), NULL);
	}
	return retval;
}

int Tester::getReceiveStream(int s, FILE **rx) {
	int retval = SUCCESS;
	(*rx) = fdopen(s,"r");

	if ( !(*rx) ) {
		close(s);  /* Failed */
		retval = FAIL;
	} else {
		setbuf((*rx), NULL);
	}
	return retval;
}

int Tester::sendReadRequest(FILE *tx) {
	fprintf(tx, "request\nread\nascii\n%s\n", _input->getInputFileName().c_str());
	fflush(tx);
	return 0;
}

int Tester::sendWriteRequest(FILE *tx) {
	fprintf(tx, "request\nwrite\nascii\n%s\n", _input->getOutputFileName().c_str());
	fflush(tx);
	return 0;
}



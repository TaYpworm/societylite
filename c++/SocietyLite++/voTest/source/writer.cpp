/*
 * writer.cpp
 *
 *  Created on: Dec 28, 2008
 *      Author: thibec
 */

#include "writer.h"

using namespace std;

FILE *Writer::rx = 0;
FILE *Writer::tx = 0;

// ----------------------------------------------------------------------------
//							Public Functions
// ----------------------------------------------------------------------------
Writer::Writer(User_Input *inputPtr):Tester(inputPtr) {}

Writer::~Writer() {}

void Writer::sigfun(int sig)
{
	// Clean up
    cout << "Exiting" << endl;
    // I'm assuming that if we got this far that everything
    // was initialized correctly.  So, close the connection.
    fflush(tx);
    fclose(tx);
    shutdown(fileno(rx), SHUT_RDWR);
    fclose(rx);
    exit(0);
}

int Writer::start() {
	struct sockaddr_in adr_srvr;	/* AF_INET */
	int s;							/* Socket */
	int retval = 0;
	string txInput = "";
	char temp[1024];

	// Setup the Network
	s = establishConnection(&adr_srvr);

	if((getReceiveStream(s,&rx) == SUCCESS) &&
			(getTransmitStream(dup(s),&tx) == SUCCESS)) {
		sendWriteRequest(tx);

		(void) signal(SIGINT, sigfun); // Set the signal to exit voTest on a ctrl-c:
		cout << "Welcome to voTest\nThis test is type Writer\nPress (ctrl-c) to exit" << endl;

		for(;;) {
			cout << "Writer>";
			getline(cin,txInput);
			fprintf(tx, "%d %s",txInput.length() ,txInput.c_str());
			fflush(tx);
			txInput ="";
		}
	} else {
		bail("Failed to Establish rx Stream.");
	}
	return retval;
}

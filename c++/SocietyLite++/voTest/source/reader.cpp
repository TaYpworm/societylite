/*
 * reader.cpp
 *
 *  Created on: Dec 28, 2008
 *      Author: thibec
 */

#include "reader.h"

using namespace std;

FILE *Reader::rx = 0;
FILE *Reader::tx = 0;

// ----------------------------------------------------------------------------
//							Public Functions
// ----------------------------------------------------------------------------
Reader::Reader(User_Input *inputPtr):Tester(inputPtr) {}

Reader::~Reader() {}

void Reader::sigfun(int sig)
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

int Reader::start() {
	struct sockaddr_in adr_srvr;	/* AF_INET */
	int s;							/* Socket */
    fd_set readfds;
    ssize_t tsize;
	int retval = 0;
	int nRead = 0;
	string rxInput = "";
	char temp[1024] = rxInput.c_str();

	// Setup the Network
	s = establishConnection(&adr_srvr);

	if((getReceiveStream(s,&rx) == SUCCESS) &&
			(getTransmitStream(dup(s),&tx) == SUCCESS)) {
		sendReadRequest(tx);

		(void) signal(SIGINT, sigfun); // Set the signal to exit voTest on a ctrl-c:
		cout << "Welcome to voTest\nThis test is type Reader\nPress (ctrl-c) to exit" << endl;

		for(;;) {
			cout << "Reader>";
			// It sucks doing it this way but voServer will not send
			// a newline character.
			FD_ZERO(&readfds);
			FD_SET(s, &readfds);
			select(s+1, &readfds, NULL,NULL,NULL);	// Wait indefinitely for
			if (FD_ISSET(s, &readfds)) {
				tsize = read(s, &temp[0], 1024);
				if(tsize > 0) {
					rxInput = string(temp);
					cout << "Received: " << rxInput << endl;
					memset(&temp,0, sizeof temp);
				}
			}
		}
	} else {
		bail("Failed to Establish rx Stream.");
	}
	return retval;
}

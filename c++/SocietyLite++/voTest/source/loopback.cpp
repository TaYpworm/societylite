/*
 * loopback.cpp
 *
 *  Created on: Dec 29, 2008
 *      Author: thibec
 */

#include "loopback.h"

using namespace std;

FILE *Loopback::rx_input = 0;
FILE *Loopback::tx_input = 0;
FILE *Loopback::rx_output = 0;
FILE *Loopback::tx_output = 0;

// ----------------------------------------------------------------------------
//							Public Functions
// ----------------------------------------------------------------------------
Loopback::Loopback(User_Input *inputPtr):Tester(inputPtr) {}

Loopback::~Loopback() {}

void Loopback::sigfun(int sig)
{
	// Clean up
    cout << "Exiting" << endl;
    // I'm assuming that if we got this far that everything
    // was initialized correctly.  So, close the connection.
    fflush(tx_input);
    fclose(tx_input);
    shutdown(fileno(rx_input), SHUT_RDWR);
    fclose(rx_input);

    fflush(tx_output);
	fclose(tx_output);
	shutdown(fileno(rx_output), SHUT_RDWR);
	fclose(rx_output);
    exit(0);
}

int Loopback::start() {
	struct sockaddr_in adr_srvr_input;	/* AF_INET */
	int s_input;							/* Socket */
	struct sockaddr_in adr_srvr_output;	/* AF_INET */
	int s_output;							/* Socket */
    fd_set readfds;
    ssize_t tsize;
	int retval = 0;
	int nRead = 0;
	string rxInput = "";
	char temp[1024];

	// Setup the Input Stream
	s_input = establishConnection(&adr_srvr_input);
	if((getReceiveStream(s_input,&rx_input) != SUCCESS) ||
			(getTransmitStream(dup(s_input),&tx_input) != SUCCESS)){
		return FAIL;
	}

	// Setup the Output Stream
	s_output = establishConnection(&adr_srvr_output);
	if((getReceiveStream(s_output,&rx_output) != SUCCESS) ||
			(getTransmitStream(dup(s_output),&tx_output) != SUCCESS)){
		return FAIL;
	}

	sendReadRequest(tx_input);
	sendWriteRequest(tx_output);

	(void) signal(SIGINT, sigfun); // Set the signal to exit voTest on a ctrl-c:
	cout << "Welcome to voTest\nThis test is type Loopback\nPress (ctrl-c) to exit" << endl;

	for(;;) {
		cout << "Loopback>";
		// It sucks doing it this way but voServer will not send
		// a newline character.
		FD_ZERO(&readfds);
		FD_SET(s_input, &readfds);
		select(s_input+1, &readfds, NULL,NULL,NULL);
		if (FD_ISSET(s_input, &readfds)) {
			tsize = read(s_input, &temp[0], 1024);
			if(tsize > 0) {
				rxInput = string(temp);
				cout << rxInput << endl;
				fprintf(tx_output, "%d %s",rxInput.length() ,rxInput.c_str());
				fflush(tx_output);

				rxInput ="";
				memset(&temp,0, sizeof temp);
			}
		}
	}

	return retval;
}

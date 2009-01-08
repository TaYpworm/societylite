/*
 * Abstract Base Class Tester
 *
 *  Created on: Dec 27, 2008
 *      Author: thibec
 *
 */

#ifndef TESTER_H_
#define TESTER_H_

#include "include.h"
#include "user_input.h"
// Signal
#include <signal.h>

class Tester {
	public:
		Tester(User_Input *);		// Constructor
		virtual ~Tester();				// Destructor
		// Pure Virtual Function
		virtual int start() = 0;
	protected:
		// Protected Functions
		void bail(const char *on_what);
		virtual int establishConnection(struct sockaddr_in *);
		int getTransmitStream(int, FILE **);
		int getReceiveStream(int, FILE **);
		int sendReadRequest(FILE *);
		int sendWriteRequest(FILE *);

		// Protected Variables
	private:
		// Private Variables
		User_Input *_input;
};

#endif /* TESTER_H_ */

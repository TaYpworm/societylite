/*
 * loopback.h
 *
 *  Created on: Dec 29, 2008
 *      Author: thibec
 */

#ifndef LOOPBACK_H_
#define LOOPBACK_H_

#include "tester.h"

class Loopback:public Tester {
	public:
		Loopback(User_Input *);
		~Loopback();
		virtual int start();
	private:
		static FILE *rx_input;
		static FILE *tx_input;
		static FILE *rx_output;
		static FILE *tx_output;
		static void sigfun(int);
};

#endif /* LOOPBACK_H_ */

/*
 * reader.h
 *
 *  Created on: Dec 28, 2008
 *      Author: thibec
 */

#ifndef READER_H_
#define READER_H_

#include "tester.h"

class Reader:public Tester {
	public:
		Reader(User_Input *);
		~Reader();
		virtual int start();
	private:
		static FILE *rx;
		static FILE *tx;
		static void sigfun(int);
};

#endif /* READER_H_ */

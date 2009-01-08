/*
 * writer.h
 *
 *  Created on: Dec 28, 2008
 *      Author: thibec
 */

#ifndef WRITER_H_
#define WRITER_H_

#include "tester.h"

class Writer:public Tester {
	public:
		Writer(User_Input *);
		~Writer();
		virtual int start();
	private:
		static FILE *rx;
		static FILE *tx;
		static void sigfun(int);
};

#endif /* WRITER_H_ */

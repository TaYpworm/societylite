/*
 * user_input.h
 *
 *  Created on: Dec 27, 2008
 *      Author: thibec
 */

#ifndef USER_INPUT_H_
#define USER_INPUT_H_

#include "include.h"

class User_Input {
	public:
		User_Input();		// Constructor
		~User_Input();		// Destructor
		int parseInput(int, char **);
		std::string getHost();
		std::string getPort();
		std::string getInputFileName();
		std::string getOutputFileName();
		TestType getTestType();
	private:
		std::string _host;
		std::string _port;
		std::string _inputFileName;
		std::string _outputFileName;
		TestType _type;
};

#endif /* USER_INPUT_H_ */

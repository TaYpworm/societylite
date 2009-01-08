/*
 * user_input.cpp
 *
 *  Created on: Dec 27, 2008
 *      Author: thibec
 */

#include "user_input.h"

using namespace std;

// ----------------------------------------------------------------------------
//							Public Functions
// ----------------------------------------------------------------------------
User_Input::User_Input() {
	// Initialize Private Variables to a known state
	_host = "";
	_port = "";
	_inputFileName = "";
	_outputFileName = "";
	_type = NONE;
}

User_Input::~User_Input() {

}

string User_Input::getHost() {
	return _host;
}

string User_Input::getPort() {
	return _port;
}

string User_Input::getInputFileName() {
	return _inputFileName;
}

string User_Input::getOutputFileName() {
	return _outputFileName;
}

TestType User_Input::getTestType() {
	return _type;
}

// This is a very simple NON-ROBUST parser.
int User_Input::parseInput(int argc, char **argv) {

	int retVal = SUCCESS;
	string str;
	string::size_type location;

	if(argc < 2) {
		// There's not enough input?
			retVal = NOT_ENOUGH_INPUT_ARGUMENTS;
	// ----------------------------------------------------------------------------
	} else if(argc < 3) {
			// Is the user looking for help?
			str = string(argv[1]);
			location = str.find("h",0);
			if(location != string::npos) {
				retVal = USER_REQUEST_HELP;
			} else {
				retVal = NOT_ENOUGH_INPUT_ARGUMENTS;
			}
			// ----------------------------------------------------------------------------
	} else if ( argc > 3) {
		// The first argument should be the ipAddr:portNum
		_host = strtok(argv[1],":");
		_port = string(strtok(NULL,"\n"));

		#if DEBUG_INPUT
			cout << "Host is: " << _host << endl;
			cout << "Port is: " << _port << endl;
		#endif
		// ----------------------------------------------------------------------------
		// Next must be the type of tester this is. followed by the FileNames
		// ----------------------------------------------------------------------------
		str = string(argv[2]);

		if(str == "-r" || str == "-R") { 			// Reader
			_type = READER;
			_inputFileName = string(argv[3]);

			#if DEBUG_INPUT
				cout << "Tester is type Reader.\nFileName: " << _inputFileName << endl;
			#endif

		} else if (str == "-w" || str == "-W") {	// Writer
			_type = WRITER;
			_outputFileName = string(argv[3]);

			#if DEBUG_INPUT
				cout << "Tester is type Writer.\nFileName: " << _outputFileName << endl;
			#endif

		} else if (str == "-l" || str == "-L") {	// Writer
			_type = LOOPBACK;
			_inputFileName = string(argv[3]);
			_outputFileName = string(argv[4]);

			#if DEBUG_INPUT
				cout << "Tester is type Loopback." << endl;
				cout << "Input FileName: "<< _inputFileName << endl;
				cout << "Output FileName: " << _outputFileName << endl;
			#endif

		} else {
			_type = NONE;
			retVal = INPUT_ERROR;
		} // if(str == ...)
	} // if(argc >= ...)
	return retVal;
} // parseInput();








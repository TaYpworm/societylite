/*
 * 	voTest.cpp
 *
 *	Description:	This is a quick and dirty implementation of a series of
 *					programs used for testing the voServer.  It is mess of
 *					C and C++ style code.
 *
 *  Created on: Dec 26, 2008
 *      Author: CMT
 */
#include "voTest.h"

using namespace std;

void printHelp() {
	cout << endl << "voTest [HOST]:[PORT] [TYPE] [inputFileName] [outputFileName]" << endl;
	cout << endl << "\tTypes:" << endl;
	cout << "\t\t-r/R\tReader:\t\tReads from given report name. " << endl;
	cout << "\t\t-w/W\tWriter:\t\tWrites user input to given stim name." << endl;
	cout << "\t\t-l/L\tLoopback:\tLoops everything read from the report" << endl;
	cout << "\t\t\t\t\tname to the stim name." << endl << endl;
}

int main(int argc, char **argv)
{
	string ttyInput = "";
	User_Input *input = 0;
	int input_result;
	Tester *test = 0;

	// Parse the input
	input = new User_Input();
	input_result = input->parseInput(argc, argv);
	if(input_result == USER_REQUEST_HELP) {
		printHelp();
		return 1;
	} else if(input_result == NOT_ENOUGH_INPUT_ARGUMENTS) {
		cout << "Usage" << endl << endl;
		printHelp();
		return 1;
	} else {		// SUCCESS
		if(input->getTestType() == READER) {
			test = new Reader(input);
			test->start();
		} else if(input->getTestType() == WRITER) {
			test = new Writer(input);
			test->start();
		} else if(input->getTestType() == LOOPBACK) {
			cout << "Loopback" << endl;
			test = new Loopback(input);
			test->start();
		} else {
			cout << "Wrong type" << endl;
		}
	}

	// Clean up.
	if(input != 0) {
		delete input;
		input = 0;
	}

	if(test != 0) {
		delete test;
		test = 0;
	}

	return 0;

}

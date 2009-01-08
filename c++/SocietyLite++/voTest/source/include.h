/*
 * include.h
 *
 *  Created on: Dec 27, 2008
 *      Author: thibec
 */

#ifndef INCLUDE_H_
#define INCLUDE_H_

// IO
#include <iostream>
#include <string>
#include <sstream>

// Network
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// User input constants
#define USER_REQUEST_HELP 				-1
#define NOT_ENOUGH_INPUT_ARGUMENTS 		-2
#define	INPUT_ERROR						-2

#define SUCCESS							0
#define FAIL							1

// Debug options
#define DEBUG_INPUT		0
#define DEBUG_NETWORK	1

enum TestType {
	NONE,
	READER,
	WRITER,
	LOOPBACK
	};

#endif /* INCLUDE_H_ */

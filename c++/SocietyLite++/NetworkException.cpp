#include "NetworkException.h"

#include <cstdio>
#include <cstring>

NetworkException::NetworkException() {
    message = "Unspecified exception";
}

NetworkException::NetworkException(const char *message) {
    this->message = message;
}

const char *NetworkException::what() const throw() {
    return message;
}

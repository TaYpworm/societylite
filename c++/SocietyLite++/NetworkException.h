#ifndef __NETWORKEXCEPTION_H__
#define __NETWORKEXCEPTION_H__

#include <exception>

using namespace std;

class NetworkException : public exception {
    public:
        const char *message;
    
    public:
        NetworkException();
        NetworkException(const char *message);
        
        const char *what() const throw();
};

#endif

#include "SocietyLite.h"
#include "NetworkException.h"

#include <cstring>
#include <string>
#include <unistd.h>

using namespace std;

int main() {
    string server("localhost");
    string service("test");

    try {
        int retval; 
        SocietyLite test(5555, server, service, 2);
        unsigned char *message = new unsigned char[256];
        sprintf((char *)message, "test");
        
        //int *ints = new int[320*240];
        char *chars = new char[320*240*4];
        for (int i=0; i<(320*240*4); i++)
            chars[i] = (char) (97+(i%26));
            //ints[i] = i;
        
        printf("%c, %c\n", chars[0], chars[320*240*4-1]);
        //for (int i=0; i<5; i++) 
        for (int i=0; i<1000; i++) {
            test.publish((unsigned char*)chars, 320*240*4);
            sleep(1);
        }
    
    } catch (NetworkException e) {
        printf("string, bitch: %s\n", e.what());
    }


    return 0;
}

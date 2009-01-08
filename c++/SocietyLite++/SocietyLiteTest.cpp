#include "SocietyLite.h"
#include "NetworkException.h"

#include <cstring>
#include <string>

using namespace std;

int main() {
    string server("localhost");
    string service("test");
    string sub("sub");

    try {
        int retval; 
        SocietyLite test(5555, server, service, 2);
        unsigned char *message = new unsigned char[256];
        sprintf((char *)message, "test");
        //for (int i=0; i<10000; i++) 
        //    test.publish(message, 4);
        int *ints = new int[128*128];
        for (int i=0; i<(128*128); i++)
            ints[i] = i;
        //for (int i=0; i<20; i++)
        //    test.publish((unsigned char*)ints, sizeof(int)*128*128);
        printf("%d", sizeof(int)*128*128);
        test.subscribe(&sub);

        for (;;) {
            retval = test.poll();
            printf("retval: %d\n", retval);
        }
    } catch (NetworkException e) {
        printf("string, bitch: %s\n", e.what());
    }


    return 0;
}

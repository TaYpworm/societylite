#include "SocietyLite.h"
#include "NetworkException.h"

#include <cstring>
#include <string>

using namespace std;

int main() {
    string server("localhost");
    string sub("test");

    try {
        int retval, msgSize; 
        SocietyLite test(5555, server);
        test.setMaxMessages(101);
        printf("max messages %d\n", test.getMaxMessages());
        unsigned char *message;

        printf ("subscribe to: %s\nsize: %d\n", sub.c_str(), sub.size());
        test.subscribe(sub);
        test.voSubscribe(sub, WRITE);
        char *conv;
        int totalPacks = 0;

        for (;;) {
            retval = test.bpoll();
            conv = (char *)test.popMessage(sub, msgSize);
            
            printf("\nfinal\n");
            printf("message size: %d\n", msgSize);
            //for (int i=0; i<8192; i++) {
            //    printf("%d\n", conv[i]);
            //}
            for (int i=0; i<320*240*4; i++)
                if (conv[i] != (97+i%26))
                    printf("%i\t%c\t%c\n", i, (char)(97+i%26), conv[i]);
            printf("pack recv\n");
            //for (int i=0; i<msgSize/sizeof(int); i++)
            //    printf("%d ", conv[i]);
            //printf("\n\n");
            totalPacks++;
            printf("totalPacks: %d\n", totalPacks);
            delete [] conv;
        }
    } catch (NetworkException e) {
        printf("string, bitch: %s\n", e.what());
    }


    return 0;
}

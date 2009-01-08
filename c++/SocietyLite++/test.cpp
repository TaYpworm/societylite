#include <cstring>
#include <cstdio>
#include <stdint.h>
#include <arpa/inet.h>

int main() {
    char *buffer = new char[3];
    uint16_t num = 16;
    uint16_t test;
    buffer[0] = 1;

    memcpy(htons(buffer[1]), &num, 2);
    memcpy(&test, &buffer[1], 2);
    printf("%d\n", test);

    return 0;
}

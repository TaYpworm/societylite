#include <cstdio>

void foo(int *blah);

int main() {
    int *bar;
    
    foo(bar);
    for (int i=0; i<3; i++)
        printf("%d ", bar[i]);
    printf("\n");
}

void foo(int *&blah) {
    blah = new int[3];
    for (int i=0; i<3; i++)
        blah[i] = i;
}

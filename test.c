#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Data {
    int *n;
};

int main() {
    int *foo;
    int bar;
    struct Data s;

    scanf("%d", &bar);

    s.n = &bar;

    printf("%d\n", *(s.n));
     
    return 0;
}
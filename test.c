#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char temp;

    char s[1024] = {0, };

    char input[1024] = {0, };

    scanf(" %[^\n]s", input);

    printf("strlen: %ld\n", strlen(input));

    strcpy(s, input);

    for (int j = 0; j < strlen(s); j++) {
        for (int i = 0; i < strlen(s) - 1; i++) {
            temp = s[i];
            s[i] = s[i + 1];
            s[i + 1] = temp;
        }

        printf("%s\n", s);
    }    
     
    return 0;
}
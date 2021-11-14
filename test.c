#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Data {
    char msg[40];
    int num;
};

int main() {
    struct Data myData;

    char buf[1024];
    memset(buf, 0, 1024);

    scanf("%[^\n]s", buf);

    // 입력받은 문자열을 data 구조체에 저장
    char *tok = malloc(sizeof(buf));

    if (strlen(buf) > 0) {
        tok = strtok(buf, " ");
        strcpy(myData.msg, tok);
        
        tok = strtok(NULL, " ");
        myData.num = atoi(tok);

    } else {
        return 1;
    }

    printf("%s %d\n", myData.msg, myData.num);
    
    return 0;
}
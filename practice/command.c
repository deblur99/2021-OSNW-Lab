#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    printf("-- START -- %d\n", getpid());
    printf("hit a key "); getchar(); printf("\n");
    execl("./exec_test", "exec_test", NULL); // 이 함수를 실행한 시점에서 command 프로세스는 종료되고,
    printf("-- END - NOT EXECUTE --\n");    // exec_test 프로세스가 시작된다.
    return 1;
} 
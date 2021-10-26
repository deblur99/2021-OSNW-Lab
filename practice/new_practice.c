#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char **argv) {
    int n = 0;

    pid_t pid = fork();

    for (;;) {
        if (pid > 0) { // 부모
            printf("Parent : %d\n", n++);
            sleep(2);
        } else if (pid == 0) { // 자식
            printf("Child : %d\n", n++);
            sleep(2);
        } else { // 프로세스 복사 실패
            printf("failed to build a process\n");
        }
    }

    return 0;
}
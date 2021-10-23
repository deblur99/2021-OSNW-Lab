#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int pid;
    int pstatus;

    printf("process start %d\n", getpid());
    pid = fork();

    if (pid > 0) {
        printf("parent process pid(%d)\n", getpid()); // 부모 프로세스의 PID
        printf("wait for child(%d) to die\n", pid); // 자식 프로세스의 PID

        printf("hit a key"); getchar(); printf("\n");
        pid = wait(&pstatus); // pstatus에는 자식 프로세스의 상태가 저장된다.

        printf("=====================================\n");
        printf("Terminated child process : %d\n", pid);
        printf("return value from child : %d\n", pstatus >> 8);
    } else if (pid == 0) {
        sleep(2);
        printf("I'm zombie %d\n", getpid());
        exit(100);
    }

    return 1;
}
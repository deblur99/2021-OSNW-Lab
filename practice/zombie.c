#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    int pstatus;
    pid_t pid = fork();

    if (pid > 0) {
        printf("부모 프로세스 pid(%d)\n", getpid());
        printf("자식 프로세스 종료를 기다림\n");
        pid = wait(&pstatus);
        printf("===============================\n");
        printf("종료된 자식 프로세스 : %d\n", pid);
        printf("종료 값             : %d\n", pstatus/256);
    } else if (pid == 0) {
        sleep(2);
        printf("I'm zombie %d\n", getpid());
        exit(100);
    }
}
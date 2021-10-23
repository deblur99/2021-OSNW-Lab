#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    pid_t pid;
    int i = 0;
    printf("process start %d\n", getpid());
    pid = fork();
    
    if (pid > 0) {
        printf("parent process id(%d)\n", // parent
        getpid());
        exit(0);
    } else if (pid == 0) {  // child
        sleep(1);
        printf("child process pid(%d) : ppid(%d)\n",
        getpid(), getppid());
        close(0); close(1); close(2); // 데몬 프로세스의 표준 입력, 출력, 에러 닫음
        setsid(); // 세션 생성

        printf("Daemon process created\n");
        i = 1000;

        for (;;) {
            printf("child : %d\n", i);
            i++;
            sleep(2);
        }
    }
    return 1;
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid[3];
    int i = 100;

    printf("Hit any key!"); getchar(); printf("\n");

    for (int i = 0; i < 3; i++) {
        printf("Fork %d!!\n", i);
        pid[i] = fork();
    }
    
    if (pid < 0) {
        printf("Forking has failed!\n");
        return 1;
    }

    // pid > 0이면 복사한 프로세스가 부모 프로세스
    else if (pid > 0) {
        printf("I'm a parent process %d\n", getpid());
        for (;;) {
            printf("P : %d\n", i);
            i++;
            sleep(1); // 1초 대기
        }
    }

    // pid == 0이면 복사한 프로세스가 자식 프로세스
    else {
        printf("I'm Child Process %d\n", getpid());
        for (;;) {
            printf("C: %d\n", i);
            i += 2;
            sleep(1);
        }
    }
}
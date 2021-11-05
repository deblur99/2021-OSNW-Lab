#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define THRESHOLD 10

int main() {
    int fd[2]; // read end, write end
    int buf = 0;
    int pid;
    int i = -1;

    if (pipe(fd) < 0) {
        return 1;
    }

    if ((pid = fork()) < 0) {
        return 1;
    } else if (pid == 0) {
        // child process
        close(fd[0]); // close the read file descriptor

        while (i < THRESHOLD) {
            i++;
            write(fd[1], (void *)&i, sizeof(i));
            sleep(1);
        }

        close(fd[1]);
        exit(0);

    } else {
        // parent process
        close(fd[1]); // close the write file descriptor

        while (buf < THRESHOLD) {
            read(fd[0], (void *)&buf, sizeof(buf));
            printf("> %d\n", buf);
            sleep(1);
        }

        close(fd[0]);
        exit(0);
    }

    // 정리
    // 1) 자식은 부모에 write하기만 하므로 read fd를 닫고,
    // 부모는 자식으로부터 read만 하므로 write fd를 닫는다.

    // 2) while문 탈출 조건은 부모, 자식 모두 일치시켜야 한다.
    // 그렇지 않으면, 무한루프에 빠진다.

    // 3) 부모에서 쓰는 버퍼 변수와 자식에서 쓰는 버퍼 변수는 서로 달라야 한다.
    // 똑같은거 쓰면 마찬가지로 무한루프에 빠진다.
}
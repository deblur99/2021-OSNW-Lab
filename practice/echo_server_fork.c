#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 3600

void do_echo(int connSock);

int main() {
    int connSock, listenSock;
    struct sockaddr_in s_addr, c_addr;
    int len;

    pid_t pid;

    listenSock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(PORT);

    if (bind(listenSock, (struct sockaddr *) &s_addr, sizeof(s_addr)) == -1) {
        printf("Cannot bind\n");
        return -1;
    }

    if (listen(listenSock, 5) == -1) {
        printf("listen Fail\n");
        return -1;
    }

    for (;;) {
        len = sizeof(c_addr);
        connSock = accept(listenSock, (struct sockaddr *) &c_addr, &len);

        if ((pid = fork()) < 0) {
            printf("Cannot fork\n");
            return -1;
        } else if (pid > 0) { // 부모 프로세스에서는 connect 포트 닫기
            close(connSock);
        } else { // 자식 프로세스에서는 listen 포트 닫기
            close(listenSock);
            do_echo(connSock);
        }
    }  
}

void do_echo(int connSock) {
    int n;
    char receiveBuffer[BUFSIZ];

    // read, write 모두 연결 소켓, 버퍼, 버퍼 크기 3개 인자를 받는다.
    while ((n = read(connSock, receiveBuffer, sizeof(receiveBuffer))) != 0) {
        printf("child\n");
        write(connSock, receiveBuffer, n);
    }
}
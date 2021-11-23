#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STR_LENGTH 20
#define MAXLINE 1024
#define MAX_CLIENTS 3

struct Data {
    char str[20];
    int num;
};

void *worker_task_1(void *data);
void *worker_task_2(void *data);
void *worker_task_3(void *data);

int main(int argc, char **argv) {
    // 소켓 관련 변수 선언 및 초기화
    int listen_fd, client_fd;
    int client_len;
    int client_fd_arr[MAX_CLIENTS] = {-1, };

    struct sockaddr_in clientaddr, serveraddr;
    
    client_len = sizeof(clientaddr);

    // 쓰레드 관련 변수 선언 및 초기화
    pthread_t worker_arr[MAX_CLIENTS] = {-1, };

    // 송수신할 데이터 구조체 배열 선언 및 초기화
    struct Data recvData[MAX_CLIENTS];
    memset(recvData, 0, sizeof(recvData));

    char buf[20] = {0, }; // for receiving string

    // listen 소켓 생성
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("socket error");
        return 1;
    }

    // 서버 관련 구조체 초기화
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[1]));

    // 서버 관련 구조체 정보와 listen 소켓을 묶음
    if (bind(listen_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("failed to bind listen socket");
        return 1;
    }

    // 서버에서 연결 대기
    listen(listen_fd, 5);

    // 서버와 3개의 클라이언트를 연결하여 client_fd_arr의 요소에 각각 저장
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_fd_arr[i] = accept(listen_fd, (struct sockaddr *)&clientaddr, &client_len);
        printf("client %d connected\n", client_fd_arr[i]);
    }

    // 클라이언트로부터 값 받기
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (read(client_fd_arr[i], recvData[i].str, sizeof(recvData[i].str)) <= 0) {
            perror("failed to read string from client");
            return 1;
        }

        if (read(client_fd_arr[i], &recvData[i].num, sizeof(recvData[i].num)) <= 0) {
            perror("failed to read integer from client");
            return 1;
        }

        printf("Read Data : %d and %s\n", recvData[i].num, recvData[i].str);
    }


    // debug
    for (int i = 0; i < MAX_CLIENTS; i++) {
        close(client_fd_arr[i]);
    }

    return 0;    
}
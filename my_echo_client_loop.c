#include <sys/socket.h>  /* 소켓 관련 함수 */
#include <arpa/inet.h>   /* 소켓 지원을 위한 각종 함수 */
#include <sys/stat.h>
#include <stdio.h>      /* 표준 입출력 관련 */
#include <stdlib.h>
#include <string.h>     /* 문자열 관련 */
#include <unistd.h>     /* 각종 시스템 함수 */

#define MAXLINE    1024

// 송수신에 사용할 데이터
struct _Data {
    char str[MAXLINE - sizeof(int)];
    int num;
};

int main(int argc, char **argv)
{
    struct sockaddr_in serveraddr;
    int server_sockfd;
    int client_len;

    int fd_num;
    int maxfd = 0, sockfd;
    fd_set readfds, allfds;

    char buf[MAXLINE];
    char *tok = NULL;

    struct _Data data = {{0, }, 0}; // 송수신할 데이터

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
        perror("error :");
        return 1;
    }

    /* 연결요청할 서버의 주소와 포트번호 프로토콜등을 지정한다. */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddr.sin_port = htons(3600);

    client_len = sizeof(serveraddr);

    /* 서버에 연결을 시도한다. */
    if (connect(server_sockfd, (struct sockaddr *)&serveraddr, client_len)  == -1)
    {
        perror("connect error :");
        return 1;
    }

    // socket fd table 초기화
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);
    FD_SET(sockfd, &readfds);

    maxfd = sockfd + 1;

    for (;;) {
        // init
        allfds = readfds;
        fd_num = select(maxfd + 1, &allfds, NULL, NULL, NULL);

        for (int i = 0; i < maxfd + 1; i++) {
            // read() from user and write() to server
            if (FD_ISSET(0, &allfds)) {
                // 문자열 버퍼 및 구조체 초기화
                memset(buf, 0, MAXLINE);
                memset(&data, 0, MAXLINE);

                // 사용자로부터 키 입력 받기
                read(0, buf, MAXLINE);    

                // 입력 버퍼의 내용을 data 구조체에 저장하기 위해 공백 기준으로 분할
                char *tok = (char *)malloc(MAXLINE);
                memset(tok, 0, MAXLINE);

                tok = strtok(buf, " ");

                // 문자열 부분이 quit과 일치하면 프로그램 종료
                if (strcmp(tok, "quit")) {
                    break;  
                }

                strcpy(data.str, tok);  // data 구조체에 문자열 저장

                tok = strtok(NULL, " ");
                data.num = atoi(tok);   // data 구조체에 정수 저장

                free(tok);

                // 서버에 입력받은 구조체 전달
                if (write(server_sockfd, &data, MAXLINE) <= 0) {
                    perror("write error : ");
                    return 1;
                }
            }

            else if (i == server_sockfd) {
                FD_SET(i, &readfds);
                
                memset(&data, 0, MAXLINE);

                // 서버로부터 데이터 읽기
                if (read(i, &data, MAXLINE) <= 0)
                {
                    perror("read error : ");
                    return 1;
                }

                printf("read : %s and %d\n", data.str, data.num);
            }
        }

        /*
        // read() from server
        if (FD_ISSET(sockfd, &allfds)) {
            memset(&data, 0, MAXLINE);

            // 서버로부터 데이터 읽기
            if (read(server_sockfd, &data, MAXLINE) <= 0)
            {
                perror("read error : ");
                return 1;
            }

            printf("read : %s and %d\n", data.str, data.num);
        }
        */
    }

    close(sockfd);

    return 0;
}

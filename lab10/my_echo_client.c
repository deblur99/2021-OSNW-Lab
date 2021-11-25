#include <sys/socket.h>  /* 소켓 관련 함수 */
#include <arpa/inet.h>   /* 소켓 지원을 위한 각종 함수 */
#include <sys/stat.h>
#include <stdio.h>      /* 표준 입출력 관련 */
#include <stdlib.h>
#include <string.h>     /* 문자열 관련 */
#include <unistd.h>     /* 각종 시스템 함수 */

#define STR_LENGTH 20
#define MAXLINE    1024

struct Data {
    char str[STR_LENGTH];
    int num;
};

int main(int argc, char **argv)
{
    // 소켓 관련 변수 선언 및 초기화
    struct sockaddr_in serveraddr;
    int server_sockfd;
    int client_len;
    char buf[MAXLINE];

    // 송수신할 데이터 구조체 선언 및 초기화
    struct Data sendData = {{0, }, 0};
    struct Data recvData = {{0, }, 0};
    
    char str_buf[20] = {0, };

    // 소켓 생성
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

    // 사용자로부터 입력 받아 sendData에 저장
    printf("input string : ");
    scanf("%[^\n]s", buf);

    if (strlen(buf) < STR_LENGTH) {
        strcpy(sendData.str, buf);
    } else {
        printf("receive error");
        return 1;
    }

    printf("input integer : ");
    scanf("%d", &sendData.num);

    /* 입력 받은 데이터를 서버로 전송한다. */
    if (write(server_sockfd, &sendData, sizeof(sendData)) <= 0) {
        perror("write error : ");
        return 1;
    }

    for (int i = 0; i < 1; i++) {
        /* 서버로 부터 데이터를 읽는다. */
        if (read(server_sockfd, &recvData, sizeof(recvData)) <= 0) {
            perror("read error : ");
            return 1;
        }

        printf("read : %s and %d", recvData.str, recvData.num);
    }
    
    return 0;
}

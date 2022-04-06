#include <sys/socket.h>  /* 소켓 관련 함수 */
#include <arpa/inet.h>   /* 소켓 지원을 위한 각종 함수 */
#include <sys/stat.h>
#include <stdio.h>      /* 표준 입출력 관련 */
#include <stdlib.h>
#include <string.h>     /* 문자열 관련 */
#include <unistd.h>     /* 각종 시스템 함수 */

#define MAXLINE    1024

struct _Data {
	char str[MAXLINE - sizeof(int)];
    int num;
};

int main(int argc, char **argv)
{
    // 초기화
	struct _Data data = {{0, }, 0};

    struct sockaddr_in serveraddr;
    int server_sockfd;
    int client_len;
    char buf[MAXLINE];

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

while(1) 
{
    memset(buf, 0, MAXLINE);
    read(0, buf, MAXLINE);    /* 키보드 입력을 기다린다. */

    // 입력 버퍼의 내용을 data 구조체에 저장하기 위해 공백 기준으로 분할
    char *tok = (char *)malloc(MAXLINE);
    memset(tok, 0, MAXLINE);

    tok = strtok(buf, " ");

    // 문자열 부분이 quit과 일치하면 프로그램 종료
    if (strcmp(tok, "quit") == 0)
        break;  
    
    strcpy(data.str, tok);  // data 구조체에 문자열 
    tok = strtok(NULL, " ");
    data.num = atoi(tok);   // data 구조체에 정수 저장
   	
    if (write(server_sockfd, buf, MAXLINE) <= 0) /* 입력 받은 데이터를 서버로 전송한다. */
    {
        perror("write error : ");
        return 1;
    }

    memset(buf, 0x00, MAXLINE);
    /* 서버로 부터 데이터를 읽는다. */
    if (read(server_sockfd, buf, MAXLINE) <= 0)
    {
        perror("read error : ");
        return 1;
    }
    printf("read : %s", buf);
}
    close(server_sockfd);
    return 0;
}

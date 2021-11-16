#include <sys/socket.h>  /* 소켓 관련 함수 */
#include <arpa/inet.h>   /* 소켓 지원을 위한 각종 함수 */
#include <sys/stat.h>
#include <stdio.h>      /* 표준 입출력 관련 */
#include <stdlib.h>
#include <string.h>     /* 문자열 관련 */
#include <unistd.h>     /* 각종 시스템 함수 */
#include <signal.h>

#define MAXBUF    1024

// SIGINT가 발생하면 프로그램 종료
int sig_handler(int signo) {
    exit(0);
}

struct Data {
    char msg[MAXBUF];
    int num;
};

int main(int argc, char **argv) {
    // 시그널 핸들러 함수 호출
    signal(SIGINT, (void *)sig_handler); 

    struct sockaddr_in serveraddr;
    int server_sockfd;
    int client_len;
    struct Data myData = {{0, }, -1};

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("error");
        return 1;
    }

    /* 연결요청할 서버의 주소와 포트번호 프로토콜등을 지정한다. */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddr.sin_port = htons(3600);

    client_len = sizeof(serveraddr);

    /* 서버에 연결을 시도한다. */
    if (connect(server_sockfd, (struct sockaddr *)&serveraddr, client_len)  == -1) {
        perror("connect error");
        return 1;
    }

    // 입력받은 문자열, 정수를 myData 구조체에 저장
    char *buf = malloc(sizeof(MAXBUF));
    printf("input string : ");
    scanf(" %[^\n]s", buf);
    strcpy(myData.msg, buf);
    free(buf);

    printf("input integer : ");
    scanf("%d", &myData.num);

    /* 입력 받은 데이터를 서버로 전송한다. */
    if (write(server_sockfd, &myData, sizeof(myData)) <= 0) {
        perror("write error");
        return 1;
    }

    // 이후에는 인터럽트 호출 전까지 서버로부터 문자열을 읽어온다.
    for (;;) {
        /* 서버로 부터 데이터를 읽는다. */
        if (read(server_sockfd, &myData, sizeof(myData)) <= 0)
        {
            perror("read error"); // 예외
            return 1;
        }

        printf("read : %s and %d\n", myData.msg, myData.num); // 출력
        
        sleep(1);
    }

    return 0;
}

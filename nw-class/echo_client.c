#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE    10000
#define PORT        3888
#define IP          "127.0.0.1"

int main(int argc, char *argv[]) {
    // string buffer
    char *buf = (char *)malloc(sizeof(char) * BUF_SIZE);
    memset(buf, 0, BUF_SIZE);

    int sbyte;

    struct sockaddr_in addr;
    int connect_sockfd;
    
    connect_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect_sockfd == -1) {
        perror("socket error");
        free(buf);
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);

    if (connect(connect_sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect error");
        close(connect_sockfd);
        free(buf);
        return 1;
    }

    for (;;) {
        scanf("%[^\n]s", buf);
        sbyte = write(connect_sockfd, buf, strlen(buf));
        if (sbyte < 0) {
            printf("Failed to write data\n");
            break;        
        } else if (sbyte > 0) {
            printf("Send data : %s\n", buf);
        }
        memset(buf, 0, BUF_SIZE);
        fflush(stdin);
        fflush(stdout);
    }

    close(connect_sockfd);
    free(buf);
    return -1;
}
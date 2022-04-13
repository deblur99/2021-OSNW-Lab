// 1:1 client-server socket program.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE    10000
#define PORT        4000
#define IP          "127.0.0.1"

int main(int argc, char *argv[]) {
    // string buffer
    char *buf = (char *)malloc(sizeof(char) * BUF_SIZE);
    memset(buf, 0, BUF_SIZE);

    int sbyte = 0, wbyte = 0;

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
        
        if ((sbyte = write(connect_sockfd, buf, strlen(buf))) <= 0) {
            printf("Failed to write data\n");
            close(connect_sockfd);
            free(buf);
            return -1;        
        }

        if (strcmp(buf, "exit") == 0) {
            printf("bye\n");
            break;
        }
        printf("Send data : %s\n", buf);

        if ((wbyte = read(connect_sockfd, buf, sizeof(buf))) <= 0) {
            printf("Failed to read data\n");
            break;
        } 

        memset(buf, 0, sizeof(buf));
    }

    close(connect_sockfd);
    free(buf);
    return 0;
}
// 1:1 client-server socket program.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 10000
#define PORT     3888

int main(int argc, int argv) {
    // string buffer
    char *buf = malloc(sizeof(char) * BUF_SIZE);
    memset(buf, 0, BUF_SIZE);
    
    int rbyte = 0;

    struct sockaddr_in client_addr, sock_addr;  // IPv4 Address
    socklen_t addr_len;                         // size of sock_addr
    int listen_sockfd, connect_sockfd;          // socket file description
    
    if ((listen_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() error");
        return 1;
    }

    // set info of listening socket to IPv4 address and port number of server
    memset((void *)&sock_addr, 0x00, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr.sin_port = htons(PORT);
    addr_len = sizeof(sock_addr);

    // after setting info of listening socket, then apply it
    if (bind(listen_sockfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1) {
        perror("bind() error");
        return 1;
    }

    // limit max connection at the same time
    // after calling listen() function, wait for connection request from client
    if (listen(listen_sockfd, 5) == -1) {
        perror("listen() error");
        return 1;
    }

    // if client tries to connect to server,
    // then listening socket creates connecting socket of newly connected client
    // connecting socket makes client and server communicate with each other.
    connect_sockfd = accept(listen_sockfd, (struct sockaddr *)&sock_addr, &addr_len);
    if (connect_sockfd == -1) {
        perror("accept() error");
        return 1;
    }

    // read(), write()
    // read() : get data from opposite side which calls write() function
    // write() : send data to opposite side which calls read() function
    for (;;) {
        rbyte = read(connect_sockfd, buf, BUF_SIZE);
        if (rbyte < 0) {
            printf("Failed to read data\n");
            close(connect_sockfd);
            break;
        } else if (rbyte > 0) {
            printf("Read data : %s\n", buf);
        }
        memset(buf, 0, BUF_SIZE);
    }

    free(buf);
    return 0;
}

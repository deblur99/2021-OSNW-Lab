#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>

void *client_module(void *data) {
    char rBuff[BUFSIZ];
    int readLen;
    int connectSd;

    connectSd = *((int *) data);

    for (;;) {
        readLen = read(connectSd, rBuff, sizeof(rBuff) - 1);
        if (readLen == 0) break;
        rBuff[readLen] = '\0';
        printf("Client(%d): %s\n", connectSd, rBuff);
        write(connectSd, rBuff, strlen(rBuff));
    }

    perror("The error is disconnected");
    close(connectSd);
}

int main(int argc, char **argv) {
    int listenSd, connecSd;
    
    struct sockaddr_in serveraddr, clientaddr;
    int clientaddr_len, strLen;

    struct sigaction act;
    pthread_t thread;

    if (argc != 2) {
        printf("Usage: %s [Port number]\n", argv[0]);
        return -1;
    }

    printf("Server start..\n");
    listenSd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[1]));

    bind(listenSd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
    listen(listenSd, 5);

    clientaddr_len = sizeof(clientaddr);

    for (;;) {
        connecSd = connect(listenSd, (struct sockaddr *)&clientaddr, clientaddr_len);

        if (connecSd == -1) {
            continue;
        } else {
            printf("A client has been connected\n");
        }

        pthread_create(&thread, NULL, client_module, NULL);
        pthread_detach(thread);
    }
    
    close(listenSd);
    return 0;
}
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>

#define MAXLINE 1024
#define PORTNUM 3600
#define SOCK_SETSIZE 1021

// 송수신애 사용할 데이터
struct _Data {
	char str[MAXLINE - sizeof(int)];
    int num;
};

static struct _Data sendData = {{0, }, 0};

int main(int argc, char **argv) {
	// 초기화
	struct _Data recvData = {{0, }, 0};
	
	int listen_fd, client_fd;
	socklen_t addrlen;

	int fd_num;
	int maxfd = 0;

	char buf[MAXLINE];
	fd_set readfds, allfds;

	struct sockaddr_in server_addr, client_addr;

	// 소켓 생성 과정
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		return 1;
	}   

	memset((void *)&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNUM);
	
	if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
		perror("bind error");
		return 1;
	}   

	// 소켓 연결 대기
	if (listen(listen_fd, 5) == -1) {
		perror("listen error");
		return 1;
	}   
	
	// socket fd 테이블 초기화
	FD_ZERO(&readfds);
	FD_SET(listen_fd, &readfds);

	maxfd = listen_fd; // 여기까지가 listen_fd 및 file descriptor table 초기화 부분이었음

	for (;;) {
		// init
		allfds = readfds;

		fd_num = select(maxfd + 1, &allfds, (fd_set *) 0, (fd_set *) 0, NULL);
		memset(&recvData, 0, MAXLINE);

		if (FD_ISSET(listen_fd, &allfds)) {
			addrlen = sizeof(client_addr);
			client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);

			if (client_fd == -1) {
				perror("failed to connect client");
				continue;
			}

			printf("Accept : %s\n", inet_ntoa(client_addr.sin_addr));
			
			FD_SET(client_fd, &readfds);
			
			continue;
		}

		for (int i = 0; i < maxfd; i++) {
			if (FD_ISSET(client_fd, &allfds)) {

				if (read(i, &recvData, MAXLINE) < 0) {
					perror("failed to read");
					continue;
				} else {
					if (strcmp(recvData.str, "quit") == 0) {
						close(i);
					} else {
						printf("from client %d: %s, %d\n", i, recvData.str, recvData.num);

						strcat(sendData.str, recvData.str);
						sendData.num += recvData.num;

						// debug
                		printf("to %d : %s %d\n", i, recvData.str, recvData.num);

						write(i, &sendData, MAXLINE);
					}
				}
			}				
		}
		sleep(1);
	}
}
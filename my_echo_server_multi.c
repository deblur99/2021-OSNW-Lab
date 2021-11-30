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

int main(int argc, char **argv) {
	// 초기화
	struct _Data *recv;
	struct _Data *send;

	int listen_fd, client_fd;
	socklen_t addrlen;

	int fd_num;
	int maxfd = 0, sockfd;

	int i= 0;
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
		fd_num = select(maxfd + 1 , &allfds, NULL, NULL, NULL);

		for (int i = 0; i < maxfd + 1; i++) {
			if (FD_ISSET(listen_fd, &allfds)) {
				if (i == listen_fd) {
					addrlen = sizeof(client_addr);
					client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);

					if (client_fd == -1) {
						perror("failed to connect client");
						continue;
					}

					printf("Accept : %s\n", inet_ntoa(client_addr.sin_addr));
					
					FD_SET(client_fd, &readfds);

					if (maxfd < client_fd) {
						maxfd = client_fd;
					}
				}

				else if (i == client_fd) {
					if (read(i, recv, MAXLINE) == -1) {
						perror("failed to read");
						FD_CLR(i, &readfds);
						close(i);
						continue;
					}

					printf("from client %d: %s, %d\n", i, recv->str, recv->num);

					// 입력 데이터 조작
					if (send == NULL) {
						strcpy(send->str, recv->str);
						send->num = recv->num;
					}

					else {
						strcat(send->str, recv->str);
						send->num += recv->num;
					}

					write(sockfd, send, MAXLINE);
				}
			}
		}

		/*
		// old
		// listen 소켓에서는 클라이언트와의 연결을 수행한다.
		if (FD_ISSET(listen_fd, &allfds))
		{
			addrlen = sizeof(client_addr);
			client_fd = accept(listen_fd,
					(struct sockaddr *)&client_addr, &addrlen);

			FD_SET(client_fd, &readfds);

			if (client_fd > maxfd)
				maxfd = client_fd; // socket fd 테이블에 연결 성공한 connect 소켓 추가

			printf("Accept : %s\n", inet_ntoa(client_addr.sin_addr));

			continue;
		}

		for (i = 0; i <= maxfd; i++) {
			sockfd = i;

			// 테이블 내 소켓들 중 connect 소켓을 찾고, 해당 소켓들에 통신 수행
			if (FD_ISSET(sockfd, &allfds)) {
				memset(recv, 0x00, MAXLINE);

				// 소켓으로부터 데이터 수신을 실패하면 해당 소켓을 닫음
				if (read(sockfd, recv, MAXLINE) <= 0) {
					close(sockfd);
					FD_CLR(sockfd, &readfds);
				}
				
				// connect 소켓에서 데이터 받으면 처리 후 출력
				else {
					// quit 문자열을 입력받으면 해당 소켓을 닫음
					if (strcmp(recv->str, "quit") == 0) {
						close(sockfd);
						FD_CLR(sockfd, &readfds);
					}

					// 구조체 멤버 각각 처리 후 다시 클라이언트에 전송
					else {
						printf("from client %d: %s, %d", sockfd, recv->str, recv->num);

						// 입력 데이터 조작
						if (send == NULL) {
							strcpy(send->str, recv->str);
							send->num = recv->num;
						}

						else {
							strcat(send->str, recv->str);
							send->num += recv->num;
						}

						write(sockfd, send, MAXLINE);
					}
				}

				if (--fd_num <= 0)
					break;
			}	
		}
		*/
	}
}
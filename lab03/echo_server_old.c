#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h> // for wait()
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXBUF 1024
#define MAX_CLIENTS 3

int main(int argc, char **argv) {
	int server_sockfd, client_sockfd;
	int client_len, n;
	char buf[MAXBUF]; // for read(), write()
	struct sockaddr_in clientaddr, serveraddr;
	client_len = sizeof(clientaddr);

	if ((server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		perror("socket error : ");
		exit(0);
	}

	memset(&serveraddr, 0x00, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(atoi(argv[1]));

	bind (server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	listen(server_sockfd, 5);

	// 클라이언트로부터 읽어올 문자열을 연결할 변수 및 연결 횟수를 선언
	char result[MAXBUF] = {0, };

	int pstatus;

	pid_t child_pid_arr[MAX_CLIENTS] = {1, };
	int client_sockfd_arr[MAX_CLIENTS];

	for (int i = 0; i < MAX_CLIENTS; i++) {
		client_sockfd_arr[i] = accept(server_sockfd, (struct sockaddr *)&clientaddr, &client_len);
		printf("New Client[%d] Connect: %s\n", i, inet_ntoa(clientaddr.sin_addr));
		child_pid_arr[i] = fork();
	}

	if (child_pid_arr[0] > 0 && child_pid_arr[1] > 0 && child_pid_arr[2] > 0) {
		for (int i = 0; i < MAX_CLIENTS; i++) {
			close(client_sockfd_arr[i]);
		}
		
		printf("현재 프로세스 : %d, 부모 프로세스 : %d\n", getpid(), getppid()); // debug
		
		int pid = wait(&pstatus);
	}

	// 자식 프로세스들은 각각 클라이언트로부터 입력을 받는다.
	if (child_pid_arr[0] == 0 && child_pid_arr[1] > 0 && child_pid_arr[2] > 0) {
		printf("현재 프로세스 : %d, 부모 프로세스 : %d\n", getpid(), getppid()); // debug

		// 클라이언트로부터 문자열 읽기
		memset(buf, 0, MAXBUF);

		if (read(client_sockfd_arr[0], buf, MAXBUF) > 0) {
			printf("\nfrom Client[%d] : %s\n", 0, buf);

			buf[strlen(buf) - 1] = 32;

			if (result[0] == 0) {
				strcpy(result, buf);
			} else {
				strcat(result, buf);
			}

		} else {
			perror("write error : ");
			close(client_sockfd_arr[0]);
			exit(1);
		}
	}

	if (child_pid_arr[0] > 0 && child_pid_arr[1] == 0 && child_pid_arr[2] > 0) {
		printf("현재 프로세스 : %d, 부모 프로세스 : %d\n", getpid(), getppid()); // debug

		// 클라이언트로부터 문자열 읽기
		memset(buf, 0, MAXBUF);

		if (read(client_sockfd_arr[1], buf, MAXBUF) > 0) {
			printf("\nfrom Client[%d] : %s\n", 1, buf);

			buf[strlen(buf) - 1] = 32;

			if (result[0] == 0) {
				strcpy(result, buf);
			} else {
				strcat(result, buf);
			}

		} else {
			perror("write error : ");
			close(client_sockfd_arr[1]);
			exit(1);
		}
	}

	if (child_pid_arr[0] > 0 && child_pid_arr[1] > 0 && child_pid_arr[2] == 0) {
		printf("현재 프로세스 : %d, 부모 프로세스 : %d\n", getpid(), getppid()); // debug

		// 클라이언트로부터 문자열 읽기
		memset(buf, 0, MAXBUF);

		if (read(client_sockfd_arr[2], buf, MAXBUF) > 0) {
			printf("\nfrom Client[%d] : %s\n", 2, buf);

			buf[strlen(buf) - 1] = 32;

			if (result[0] == 0) {
				strcpy(result, buf);
			} else {
				strcat(result, buf);
			}

		} else {
			perror("write error : ");
			close(client_sockfd_arr[2]);
			exit(1);
		}
	}

	getchar();
	/*
	for (int i = 0; i < 3; i++) {
		if (pid > 0) {
			// debug
			printf("현재 프로세스 : %d, 부모 프로세스 : %d\n", getpid(), getppid());

			client_sockfd = accept(
				server_sockfd, (struct sockaddr *)&clientaddr, &client_len
			);

		} else if (pid == 0) {
			pid = getppid();
			i--;
		}
	}
	
	// debug
	getchar();

	// 부모 프로세스가 클라이언트 연결 및 복사 수행
	for (int i = 0; i < MAX_CLIENTS - 1; i++) {
		client_sockfd = accept(
			server_sockfd, (struct sockaddr *)&clientaddr, &client_len
		);

		printf("New Client[%d] Connect: %s\n", i, inet_ntoa(clientaddr.sin_addr));

		pid = fork();
	}
	/*
		// debug
		getchar();

		sleep(2); // temp

		pid = wait(&pstatus); // 자식 프로세스들이 처리될 때까지 대기. 자식 프로세스들이 모두 실행된다.

		close(client_sockfd);
	}*/
	/*
	if (pid > 0) {}
	// 자식 프로세스는 클라이언트 소켓과의 read, write 수행
	else if (pid == 0) {
		close(server_sockfd);

		// 클라이언트로부터 문자열 읽기
		if (read(client_sockfd, buf, MAXBUF) > 0) {
			printf("\nfrom Client[%d] : %s\n", connected_count++, buf);

			buf[strlen(buf) - 1] = 32;

			if (result[0] == 0) {
				strcpy(result, buf);
			} else {
				strcat(result, buf);
			}

		} else {
			perror("write error : ");
			close(client_sockfd);
			exit(1);
		}
	}
	*/
	return 0;
}

	//int socket_list[3] = {-1, };
/*
	for (connected_count = 0; connected_count < MAX_CLIENTS; connected_count++) {
		client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr,
		&client_len);
		client_arr[connected_count].client_sockfd = client_sockfd;
		client_arr[connected_count].pid = fork(); // 자식 프로세스 각각 생성
		printf("New Client[%d] Connect: %s\n", connected_count, inet_ntoa(clientaddr.sin_addr));
	}
*/
	/*
	for (connected_count = 0; connected_count < MAX_CLIENTS; connected_count++) {
		child_arr[connected_count].pid = fork(); // 자식 프로세스 생성
	}*/

	/*
	for (;;) {
		
		if (child_count >= 3) {
			break;
		}

		pid = fork();
		child_count++;

		if (pid < 0) { // 자식 프로세스 복사 실패
			printf("Fork failure\n");
			exit(1); // 자식 프로세스 종료
			
		} else if (child_arr[connected_count].pid > 0) { // 부모 프로세스일 때
			// 연결이 완료되면, 자식 프로세스가 클라이언트와 통신
			parent.pid = wait(parent.pstatus);
			
		} else { // 자식 프로세스일 때
			// 소켓 연결
			client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr, &client_len);

			printf("New Client[%d] Connect: %s\n", child_count, inet_ntoa(clientaddr.sin_addr));

			memset(buf, 0x00, MAXBUF);

			// 클라이언트로부터 문자열 읽기
			if (read(client_sockfd, buf, MAXBUF) > 0) {
				printf("\nfrom Client[%d] : %s\n", child_count, buf);

				buf[strlen(buf) - 1] = 32;

				if (result[0] == 0) {
					strcpy(result, buf);
				} else {
					strcat(result, buf);
				}

			} else {
				perror("write error : ");
				close(client_sockfd);
				exit(1);
			}

			if (write(client_sockfd, result, MAXBUF) <= 0) {
				perror("write error : ");
				close(client_sockfd);
				exit(1);
			}

			exit(0);
		}
	}
	
	for (connected_count = 0; connected_count < MAX_CLIENTS; connected_count++) {
		memset(buf, 0x00, MAXBUF);

		if (read(client_arr[connected_count].client_sockfd, buf, MAXBUF) > 0) {
			printf("\nfrom Client[%d] : %s\n", connected_count, buf);
			buf[strlen(buf) - 1] = 32;
			if (result[0] == 0x00) {
				strcpy(result, buf);
			} else {
				strcat(result, buf);
			}

		} else {
			perror("write error : ");
			close(client_arr[connected_count].client_sockfd);
		}
	}

	memset(buf, 0x00, MAXBUF);

	for (connected_count = 0; connected_count < MAX_CLIENTS; connected_count++) {
		if (write(client_arr[connected_count].client_sockfd, result, MAXBUF) <= 0) {
			perror("write error : ");
			close(client_arr[connected_count].client_sockfd);
		}
	}
	
	for (connected_count = 0; connected_count < MAX_CLIENTS; connected_count++) {
		close(client_arr[connected_count].client_sockfd);
	}

	getchar(); // debug

	close(server_sockfd);
	return 0;
}
*/
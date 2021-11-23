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

// PIPE file descriptor index
#define READ 0
#define WRITE 1

// 통신 흐름
	// 1. 자식 프로세스는 클라이언트로부터 문자열을 입력받음
	// 2. 부모 프로세스에 입력받은 문자열 전달
	// 3. 부모 프로세스는 세 자식 프로세스로부터 문자열을 입력받으며 결과 문자열을 만듦
	// 4. 각 자식 프로세스에 결과 문자열 전달
	// 5. 각 자식 프로세스는 부모 프로세스로부터 문자열을 받고, 연결된 클라이언트에 문자열을 전송함
	// 6. connect, listen 소켓 닫고 자식 프로세스 -> 부모 프로세스 순으로 종료

int main(int argc, char **argv) {

    // 소켓 관련 변수, 버퍼 관련 변수 선언 및 초기화
	int listen_fd, client_fd; // 각각 listen 소켓, connect 소켓
	int client_len, n;
	int client_fd_arr[MAX_CLIENTS] = {-1, };
    int pid1, pid2, pid3;
	
	struct sockaddr_in clientaddr, serveraddr;
	client_len = sizeof(clientaddr);

	static int count = 0; // 문자열 읽은 횟수
	
	// PIPE 배열 2개 생성
	// c_to_p : 자식 프로세스에서 입력받은 문자열을 부모 프로세스로 전달
	// p_to_c : 부모 프로세스에서 결과 문자열을 자식 프로세스들에 전달
	int pipe_c_to_p[2];
	int pipe_p_to_c[2];

	if (pipe(pipe_c_to_p) == -1) {
		// exception
		perror("Failed to create a pipe");
		return -1;
	}

	if (pipe(pipe_p_to_c) == -1) {
		// exception
		perror("Failed to create a pipe");
		return -1;
	}

    // listen 소켓을 생성
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		perror("socket error : ");
		exit(0);
	}

    // 서버 관련 구조체를 초기화
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(atoi(argv[1]));

    // 서버 관련 구조체의 정보와 listen 소켓을 묶음
	if (bind(listen_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        printf("An error occurred\n");
        return -1;
    }

    // 서버에서 연결 대기
	listen(listen_fd, 5);

	// 서버와 3개의 클라이언트를 연결하여 클라이언트 소켓 배열에 각각 저장
	for (int i = 0; i < MAX_CLIENTS; i++) {
		client_fd_arr[i] = accept(listen_fd, (struct sockaddr *)&clientaddr, &client_len);
		printf("New Client[%d] Connect: %s\n", i, inet_ntoa(clientaddr.sin_addr));
	}

	// 프로세스 복사
	pid1 = fork();

	if (pid1 > 0) {
		pid2 = fork();
	}

	if (pid1 > 0 && pid2 > 0) {
		pid3 = fork();
	}

	// 부모 프로세스에서는 connect 소켓을 닫음
	if (pid1 > 0 && pid2 > 0 && pid3 > 0) {
		for (int i = 0; i < MAX_CLIENTS; i++) {
			close(client_fd_arr[i]);
		}
	} 
	
	// 자식 프로세스에서는 listen 소켓을 닫음
	else { 
		close(listen_fd);
	}

	char buf[MAXBUF]; // for read(), write()
	
	// 부모 프로세스는 자식 프로세스에서의 통신이 끝날 때까지 대기한다.
	// 모두 끝나면 부모 프로세스의 listen 소켓을 닫는다.
	if (pid1 > 0 && pid2 > 0 && pid3 > 0) {
		char recv_buf[MAXBUF];

		char result[MAXBUF];
		memset(result, 0, MAXBUF);

		for (int i = 0; i < MAX_CLIENTS; i++) {
			memset(recv_buf, 0, MAXBUF);
			read(pipe_c_to_p[READ], recv_buf, MAXBUF);
			strcat(result, recv_buf);
		}

		for (int i = 0; i < MAX_CLIENTS; i++) {
			write(pipe_p_to_c[WRITE], result, MAXBUF);
		}

		close(listen_fd);
	}

	// 자식 프로세스 1
	if (pid1 == 0) {
		memset(buf, 0x00, MAXBUF);

		if ((n = read(client_fd_arr[0], buf, MAXBUF)) > 0) {
			printf("\nfrom Client[%d] : %s\n", 0, buf);

			buf[strlen(buf)] = 32;
			buf[strlen(buf)+1] = 0;

			write(pipe_c_to_p[WRITE], buf, MAXBUF);

			memset(buf, 0x00, MAXBUF);

			read(pipe_p_to_c[READ], buf, MAXBUF);

			write(client_fd_arr[0], buf, MAXBUF);
			
			close(client_fd_arr[0]);

			exit(0);
		} else {
			perror("failed to read");
			close(client_fd_arr[0]);
			return -1;
		}
	} 

	// 자식 프로세스 2
	if (pid2 == 0) {
		memset(buf, 0x00, MAXBUF);

		if ((n = read(client_fd_arr[1], buf, MAXBUF)) > 0) {
			printf("\nfrom Client[%d] : %s\n", 1, buf);

			buf[strlen(buf)] = 32;
			buf[strlen(buf)+1] = 0;

			write(pipe_c_to_p[WRITE], buf, MAXBUF);

			memset(buf, 0x00, MAXBUF);

			read(pipe_p_to_c[READ], buf, MAXBUF);

			write(client_fd_arr[1], buf, MAXBUF);
			
			close(client_fd_arr[1]);

			exit(0);
		} else {
			perror("failed to read");
			close(client_fd_arr[1]);
			return -1;
		}
	} 
	
	// 자식 프로세스 3
	if (pid3 == 0) {
		memset(buf, 0x00, MAXBUF);

		if ((n = read(client_fd_arr[2], buf, MAXBUF)) > 0) {
			printf("\nfrom Client[%d] : %s\n", 2, buf);

			buf[strlen(buf)] = 32;
			buf[strlen(buf)+1] = 0;

			write(pipe_c_to_p[WRITE], buf, MAXBUF);

			memset(buf, 0x00, MAXBUF);
			
			read(pipe_p_to_c[READ], buf, MAXBUF);

			write(client_fd_arr[2], buf, MAXBUF);
			
			close(client_fd_arr[2]);

			exit(0);
		} else {
			perror("failed to read");
			close(client_fd_arr[2]);
			return -1;
		}
	}
	
	return 0;
}	
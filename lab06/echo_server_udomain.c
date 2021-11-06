#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/wait.h> 
#include <sys/socket.h> 
#include <sys/un.h> 
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <fcntl.h>
 
#define MAXLINE 1024 
#define MAX_CLIENTS 3

int main(int argc, char **argv) 
{ 
	int listen_fd;
	int client_fd_arr[MAX_CLIENTS] = {-1, }; 
	int p_pid, pstatus;
	int pid1, pid2, pid3; // child process id

	socklen_t addrlen; 
	int readn; 
	char buf[MAXLINE]; 
	struct sockaddr_un client_addr, server_addr; 

	static int count = 0; // 문자열 읽은 횟수
	char result[MAXLINE] = {0x00, }; // 결과 문자열


	if(argc != 2) 
	{ 
		printf("Usage : %s [socket file name]\n", argv[0]); 
		return 1; 
	} 
	if(access(argv[1], F_OK) == 0)  // bind 함수 이전에 UDS 파일 삭제
	{ 
		unlink(argv[1]); 
	} 
 
	if( (listen_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 ) 
	{ 
		perror("Error : socket"); 
		return 0; 
	} 
 
	memset((void *)&server_addr, 0x00, sizeof(server_addr)); 
	server_addr.sun_family = AF_UNIX; 
	strncpy(server_addr.sun_path, argv[1], strlen(argv[1])); 
 
	if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) 
	{ 
		perror("Error : bind"); 
		return 0; 
	} 
 
	if (listen(listen_fd, 5) == -1) 
	{ 
		perror("Error : listen"); 
		return 1; 
	} 

	// 서버와 3개의 클라이언트를 연결하여 클라이언트 소켓 배열에 각각 저장
	for (int i = 0; i < MAX_CLIENTS; i++) {
		client_fd_arr[i] = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
		printf("New Client[%d] Connect: 127.0.0.1\n", i); // localhost
	}

	// debug
	for (int i = 0; i < MAX_CLIENTS; i++)
		printf("socket %d connected\n", client_fd_arr[i]);
	
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

	// 부모 프로세스는 자식 프로세스에서의 통신이 끝날 때까지 대기한다.
	// 모두 끝나면 부모 프로세스의 listen 소켓을 닫는다.
	if (pid1 > 0 && pid2 > 0 && pid3 > 0) {
		p_pid = wait(&pstatus);

		close(listen_fd);

		return 0;
	}

	int n = 0;

	// read : 클라이언트로부터 문자열 읽기
	if (pid1 == 0 && pid2 > 0 && pid3 > 0) {
		
		memset(buf, 0x00, MAXLINE);

		if ((n = read(client_fd_arr[0], buf, MAXLINE)) > 0) {
			printf("\nfrom Client[%d] : %s\n", 0, buf);
			buf[strlen(buf)] = 32;
			buf[strlen(buf)+1] = 0;

			if (result[0] == 0x00) {
				strcpy(result, buf);
			}

			else {
				strcat(result, buf);
			}

			count++;
		}
	} 
	
	if (pid1 > 0 && pid2 == 0 && pid3 > 0) {
		memset(buf, 0x00, MAXLINE);

		if ((n = read(client_fd_arr[1], buf, MAXLINE)) > 0) {
			printf("\nfrom Client[%d] : %s\n", 1, buf);
			buf[strlen(buf)] = 32;
			buf[strlen(buf)+1] = 0;

			if (result[0] == 0x00) {
				strcpy(result, buf);
			}

			else {
				strcat(result, buf);
			}

			count++;
		}
	} 
	
	if (pid1 > 0 && pid2 > 0 && pid3 == 0) {
		memset(buf, 0x00, MAXLINE);

		if ((n = read(client_fd_arr[2], buf, MAXLINE)) > 0) {
			printf("\nfrom Client[%d] : %s\n", 2, buf);
			buf[strlen(buf)] = 32;
			buf[strlen(buf)+1] = 0;

			if (result[0] == 0x00) {
				strcpy(result, buf);
			}

			else {
				strcat(result, buf);
			}

			count++;
		}
	}

	printf("%s\n", result);

	exit(0);

	/*
	while(1) 
	{ 
		memset((void *)&client_addr, 0x00, sizeof(client_addr)); 
		printf("accept wait\n"); 
		client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen); 
		while(1) 
		{ 
			if(client_fd == -1) 
			{ 
				printf("Accept Error "); 
				return 0; 
			} 
			memset(buf, 0x00, MAXLINE); 
			readn = read(client_fd, buf, MAXLINE); 
			if(readn == 0) break; 
			printf("==> %s", buf); 
			write(client_fd, buf, strlen(buf)); 
		} 
	} 
	*/
	//return 0; 
}
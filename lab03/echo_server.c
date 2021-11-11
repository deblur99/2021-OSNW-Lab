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

    // 소켓 관련 변수, 버퍼 관련 변수 선언 및 초기화
	int server_sockfd, client_sockfd; // 각각 listen 소켓, connect 소켓
	int client_len, n;
	int client_sockfd_arr[MAX_CLIENTS] = {-1, };
	int pstatus;
    int p_pid, pid1, pid2, pid3;
	
	struct sockaddr_in clientaddr, serveraddr;
	client_len = sizeof(clientaddr);

	static int count = 0; // 문자열 읽은 횟수


    // listen 소켓을 생성
	if ((server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		perror("socket error : ");
		exit(0);
	}

    // 서버 관련 구조체를 초기화
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(atoi(argv[1]));

    // 서버 관련 구조체의 정보와 listen 소켓을 묶음
	if (bind(server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        printf("An error occurred\n");
        return -1;
    }

    // 서버에서 연결 대기
	listen(server_sockfd, 5);

	// 서버와 3개의 클라이언트를 연결하여 클라이언트 소켓 배열에 각각 저장
	for (int i = 0; i < MAX_CLIENTS; i++) {
		client_sockfd_arr[i] = accept(server_sockfd, (struct sockaddr *)&clientaddr, &client_len);
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
			close(client_sockfd_arr[i]);
		}
	} 
	
	// 자식 프로세스에서는 listen 소켓을 닫음
	else { 
		close(server_sockfd);
	}

	// 부모 프로세스는 자식 프로세스에서의 통신이 끝날 때까지 대기한다.
	// 모두 끝나면 부모 프로세스의 listen 소켓을 닫는다.
	if (pid1 > 0 && pid2 > 0 && pid3 > 0) {
		p_pid = wait(&pstatus);

		close(server_sockfd);

		return 0;
	}

	char buf[MAXBUF]; // for read(), write()
	char result[MAXBUF];
	memset(result, 0, sizeof(result));

	// read : 클라이언트로부터 문자열 읽기
	
		if (pid1 == 0 && pid2 > 0 && pid3 > 0) {
		
			memset(buf, 0x00, MAXBUF);

			if ((n = read(client_sockfd_arr[0], buf, MAXBUF)) > 0) {
				printf("\nfrom Client[%d] : %s\n", 0, buf);
				buf[strlen(buf) - 1] = 32;

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
			memset(buf, 0x00, MAXBUF);

		if ((n = read(client_sockfd_arr[1], buf, MAXBUF)) > 0) {
			printf("\nfrom Client[%d] : %s\n", 1, buf);
			buf[strlen(buf) - 1] = 32;

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
		memset(buf, 0x00, MAXBUF);

		if ((n = read(client_sockfd_arr[2], buf, MAXBUF)) > 0) {
			printf("\nfrom Client[%d] : %s\n", 2, buf);
			buf[strlen(buf) - 1] = 32;

			if (result[0] == 0x00) {
				strcpy(result, buf);
			}
			else {
				strcat(result, buf);
			}

			count++;
		}
	}
	

	// write
	for (;;) {
		if (count >= MAX_CLIENTS) {
			if (pid1 == 0 && pid2 > 0 && pid3 > 0) {
				if ((n = write(client_sockfd_arr[0], result, MAXBUF)) <= 0) {
					perror("read error : ");
				}

				close(client_sockfd_arr[0]);
				exit(0);
			} 
	
			if (pid1 > 0 && pid2 == 0 && pid3 > 0) {
				if ((n = write(client_sockfd_arr[1], result, MAXBUF)) <= 0) {
					perror("read error : ");
				}

				close(client_sockfd_arr[1]);
				exit(0);
			} 
	
			if (pid1 > 0 && pid2 > 0 && pid3 == 0) {
				if ((n = write(client_sockfd_arr[2], result, MAXBUF)) <= 0) {
					perror("read error : ");
				}

				close(client_sockfd_arr[2]);
				exit(0);
			}

			break;
		}
		else {
			sleep(1);
		}
	}
		
	return 0;
}	

	/*
	for (;;) {
    	if ((pid = fork()) < 0) { // 복사 시 오류 발생하면 예외 처리
    	    printf("An error occurred\n");
    	    return -1;

    	} else if (pid > 0) { // 부모 프로세스

			for (int i = 0; i < MAX_CLIENTS; i++)
    	    	close(client_sockfd_arr[i]); 
	
    	    pid = wait(&pstatus); // 자식 프로세스가 끝날 때까지 대기

			close(server_sockfd);

    	} else if (pid == 0) { // 자식 프로세스
			int process_idx = -1;

			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (child_pid_arr[i] != -1) {
					break;
				} else {
					child_pid_arr[i] = getpid();
					process_idx = i;
					break;
				}
			}

    	    close(server_sockfd); // 자식 프로세스에서는 listen 소켓을 닫음

    	    // 클라이언트로부터 문자열 읽기
			memset(buf, 0, MAXBUF);
			strcpy(buf, read_from_client(client_sockfd_arr[process_idx]));

			printf("New Client[%d] Connect : %s\n", process_idx, inet_ntoa(clientaddr.sin_addr));

			// 문자열을 결과 문자열에 이어 붙이고 작업 종료 -> 대기해야 하는 건가?
			if (result[0] == '\0') {
				strcpy(result, buf);
			} else {
				strcat(result, buf);
			}		

			buf[strlen(buf) - 1] = 32; // 문자열 간 공백 문자 삽입

			count++;

			while (count != MAX_CLIENTS) {
				sleep(1);
			}

			// 결과 문자열이 모두 합쳐지면 클라이언트들에게 결과 문자열 전송
			write_to_client(client_sockfd_arr[process_idx], result);

			close(client_sockfd_arr[process_idx]); // 자식 프로세스가 할 일을 끝내면 connect 소켓 닫음
    	}

    	return 0;
	}  
	*/

/*
char* read_from_client(int fd, int idx) {
	char buf[MAXBUF] = {0, };

	if (read(fd, buf, MAXBUF) > 0) {
		printf("\nfrom Client[%d] : %s\n", idx, buf);
	} else {
		perror("read error : ");
		close(fd);
		exit(-1);
	}

	return buf;
}

int write_to_client(int fd, char result[]) {
	if (write(fd, result, MAXBUF) <= 0) {
		perror("write error : ");
		close(fd);
		exit(-1);
	} else {
		return 1;
	}
}
*/
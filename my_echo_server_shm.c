#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h> // for wait()
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define MAXBUF 1024
#define MAX_CLIENTS 3

// PIPE file descriptor index
#define READ 0
#define WRITE 1

// SIGINT가 발생하면 프로그램 종료
int sig_handler(int signo) {
    exit(0);
}

// 클라이언트와 주고받을 구조체
struct Data {
    char msg[MAXBUF];
    int num;
};

int main(int argc, char **argv) {
	// 시그널 핸들러 함수 호출
    signal(SIGINT, (void *)sig_handler); 

	struct Data myData = {{0, }, -1};

    // 소켓 관련 변수, 버퍼 관련 변수 선언 및 초기화
	int listen_fd, client_fd; // 각각 listen 소켓, connect 소켓
	int client_len, n;
	int client_fd_arr[MAX_CLIENTS] = {-1, };

	// 클라이언트와 연결될 자식 프로세스의 PID
    int pid1, pid2, pid3;
	
	// 소켓 정보
	struct sockaddr_in clientaddr, serveraddr;
	client_len = sizeof(clientaddr);

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
	
	// 부모 프로세스는 자식 프로세스에서의 통신이 끝날 때까지 대기한다.
	// 모두 끝나면 부모 프로세스의 listen 소켓을 닫는다.
	if (pid1 > 0 && pid2 > 0 && pid3 > 0) {
		close(listen_fd);
	}

	// 자식 프로세스 1
	if (pid1 == 0) {
		
		if ((n = read(client_fd_arr[0], &myData, sizeof(myData))) > 0) {

			// 입력받은 값 출력
			printf("\nRead Data %s(%d) : %d and %s",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), 
				myData.num, myData.msg);

			// 공유 메모리 할당
			

			// 생산자, 소비자 프로세스 생성
			int pid1_prod, pid1_cons;

			pid1_prod = fork(); // 생산자

			if (pid1_prod > 0) {
				pid1_cons = fork(); // 소비자
			}

			// debug
			getchar();
			getchar();

			// 결과값 쓰기 -> 생산자, 소비자 분리해서 각각 보내는 걸로 구현하기
			write(client_fd_arr[0], &myData, sizeof(myData));
			
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
		if ((n = read(client_fd_arr[1], &myData, sizeof(myData))) > 0) {

			// 입력받은 값 출력
			printf("\nRead Data %s(%d) : %d and %s",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), 
				myData.num, myData.msg);

			// 공유 메모리 할당
			

			// 생산자, 소비자 프로세스 생성
			int pid2_prod, pid2_cons;

			pid2_prod = fork(); // 생산자

			if (pid2_prod > 0) {
				pid2_cons = fork(); // 소비자
			}

			// debug
			getchar();
			getchar();

			// 결과값 쓰기 -> 생산자, 소비자 분리해서 각각 보내는 걸로 구현하기
			write(client_fd_arr[1], &myData, sizeof(myData));
			
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
		if ((n = read(client_fd_arr[2], &myData, sizeof(myData))) > 0) {

			// 입력받은 값 출력
			printf("\nRead Data %s(%d) : %d and %s",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), 
				myData.num, myData.msg);

			// 공유 메모리 할당
			

			// 생산자, 소비자 프로세스 생성
			int pid3_prod, pid3_cons;

			pid3_prod = fork(); // 생산자

			if (pid3_prod > 0) {
				pid3_cons = fork(); // 소비자
			}

			// debug
			getchar();
			getchar();

			// 결과값 쓰기 -> 생산자, 소비자 분리해서 각각 보내는 걸로 구현하기
			write(client_fd_arr[2], &myData, sizeof(myData));
			
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
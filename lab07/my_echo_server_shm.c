#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h> // for wait()
#include <sys/shm.h> // for shared memory
#include <sys/sem.h> // for semaphore
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

// 소켓 정보를 저장할 구조체
struct ClientSocketInfo {
	int fd;
	char addr[20];
	unsigned short port;	
};

// 클라이언트로부터 입력받는 구조체
typedef struct _RecvData {
    char msg[MAXBUF];
    int num;
}RecvData;

// 클라이언트로 전달하는 구조체
typedef struct _SendData {
	char msg[MAXBUF];
    int *num;
}SendData;

union semun {
	int val;
};

int main(int argc, char **argv) {
	// 시그널 핸들러 함수 호출
    signal(SIGINT, (void *)sig_handler);

	// 세마포어
	struct sembuf semopen = {0, -1, SEM_UNDO};
	struct sembuf semclose = {0, 1, SEM_UNDO};
	
	union semun semun_arr[MAX_CLIENTS];

	RecvData recvData = {{0, }, -1};
	SendData *sendData;

    // 소켓 관련 변수, 버퍼 관련 변수 선언 및 초기화
	int listen_fd, client_fd; // 각각 listen 소켓, connect 소켓
	int client_len, n;
	struct ClientSocketInfo client_arr[MAX_CLIENTS];

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
		client_arr[i].fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &client_len);
		strcpy(client_arr[i].addr, inet_ntoa(clientaddr.sin_addr));
		client_arr[i].port = ntohs(clientaddr.sin_port);
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
			close(client_arr[i].fd);
		}
	} 
	
	// 자식 프로세스에서는 listen 소켓을 닫음
	else { 
		close(listen_fd);
	}
	
	// 부모 프로세스는 자식 프로세스에서의 통신이 끝날 때까지 대기한다.
	// 모두 끝나면 부모 프로세스의 listen 소켓을 닫는다.
	if (pid1 > 0 && pid2 > 0 && pid3 > 0) {
		int pstatus;

		pstatus = wait(&pstatus);
		
		close(listen_fd);
	}

	// 자식 프로세스 1
	if (pid1 == 0) {
		
		if ((n = read(client_arr[0].fd, &recvData, sizeof(RecvData))) > 0) {

			// 입력받은 값 출력
			printf("Read Data %s(%d) : %d and %s\n",
				client_arr[0].addr, client_arr[0].port, 
				recvData.num, recvData.msg);

			// sendData 크기만큼 공유 메모리 할당
			strcpy(sendData->msg, recvData.msg);
			sendData->num = &recvData.num;
			
			int shmid, semid;
			void *sharedMemory = NULL;

			printf("before fork\n"); //debug

			// 소비자 프로세스 생성
			int pid_cons = fork();

			printf("after fork\n"); //debug

			// 생산자, 소비자 프로세스는 각각 공유 메모리에 순차적으로 접근하여 처리한다.
			if (pid_cons > 0) {				
				SendData *cal_data;
				
				if ((shmid = shmget((key_t)5, sizeof(SendData), 0666|IPC_CREAT)) == -1) {
					perror("shmget error");
					return 1;
				}

				if ((semid = semget((key_t)2000, 1, IPC_CREAT|0666)) == -1) {
					perror("semget error");
					return 1;
				}

				if ((sharedMemory = shmat(shmid, NULL, 0)) == (void *)-1) {
					perror("shmat error");
					return 1;
				}

				cal_data = (SendData *)sharedMemory;
				memset(cal_data->msg, 0, MAXBUF);
				*(cal_data->num) = *(sendData->num);

				semun_arr[0].val = 1;
				if (semctl(semid, 0, SETVAL, semun_arr[0]) == -1) {
					perror("semctl1 error");
					return 1;
				}

				char temp;
				int s_length = strlen(sendData->msg) - 1;

				char result[MAXBUF] = {0, };
				strcpy(result, sendData->msg);
				sendData->num = recvData.num;

				for (;;) {
					if (semop(semid, &semopen, 1) == -1) {
						perror("semop error");
						return 1;
					}

					for (int i = 0; i < s_length; i++) {
						temp = result[i];
						result[i] = result[i + 1];
						result[i + 1] = temp;
				 	}

					strcpy(sendData->msg, result);

					(*sendData->num)++;
				
					semop(semid, &semclose, 1);

					sleep(1);
				}
			}

			if (pid_cons == 0) {
				SendData *cal_data;

				if ((shmid = shmget((key_t)5, sizeof(sendData), 0666)) == -1) {
					perror("error");
					return 1;
				}

				if ((semid = semget((key_t)2000, 0, 0666)) == -1) {
					perror("error");
					return 1;
				}

				if ((sharedMemory = shmat(shmid, NULL, 0)) == (void *)-1) {
					perror("error");
					return 1;
				}

				cal_data = (SendData *)sharedMemory;

				for (;;) {
					if (semop(semid, &semopen, 1) == -1) {
						perror("error");
						return 1;
					}
	
					write(client_arr[0].fd, cal_data, sizeof(SendData));
	
					semop(semid, &semclose, 1);

					sleep(1);
				}
			}
		}

		else {
			perror("failed to read");
			close(client_arr[0].fd);
			return -1;
		}
	} 

	// 자식 프로세스 2
	if (pid2 == 0) {
		if ((n = read(client_arr[1].fd, &recvData, sizeof(RecvData))) > 0) {

			// 입력받은 값 출력
			printf("Read Data %s(%d) : %d and %s\n",
				client_arr[1].addr, client_arr[1].port, 
				recvData.num, recvData.msg);

			// sendData 크기만큼 공유 메모리 할당
			strcpy(sendData->msg, recvData.msg);
			sendData->num = &recvData.num;
			int shmid, semid;
			void *sharedMemory = NULL;

			// 소비자 프로세스 생성
			int pid_cons = fork();

			// 생산자, 소비자 프로세스는 각각 공유 메모리에 순차적으로 접근하여 처리한다.
			if (pid_cons > 0) {
				SendData *cal_data;

				if ((shmid = shmget((key_t)32769, sizeof(SendData), 0666|IPC_CREAT)) == -1) {
					perror("error");
					return 1;
				}

				if ((semid = semget((key_t)35000, 1, IPC_CREAT|0666)) == -1) {
					perror("error");
					return 1;
				}

				if ((sharedMemory = shmat(shmid, NULL, 0)) == (void *)-1) {
					perror("error");
					return 1;
				}

				cal_data = (SendData *)sharedMemory;
				memset(cal_data->msg, 0, MAXBUF);
				*(cal_data->num) = *(sendData->num);

				semun_arr[1].val = 1;
				if (semctl(semid, 0, SETVAL, semun_arr[1]) == -1) {
					perror("semctl2 error");
					return 1;
				}

				char temp;
				int s_length = strlen(sendData->msg) - 1;

				char result[MAXBUF] = {0, };
				strcpy(result, sendData->msg);

				for (;;) {
					if (semop(semid, &semopen, 1) == -1) {
						perror("semop error");
						return 1;
					}

					for (int i = 0; i < s_length; i++) {
						temp = result[i];
						result[i] = result[i + 1];
						result[i + 1] = temp;
				 	}

					strcpy(cal_data->msg, result);

					(*cal_data->num)++;
				
					semop(semid, &semclose, 1);

					sleep(1);
				}
			}

			if (pid_cons == 0) {
				SendData *cal_data;

				if ((shmid = shmget((key_t)32769, sizeof(SendData), 0666)) == -1) {
					perror("error");
					return 1;
				}

				if ((semid = semget((key_t)35000, 0, 0666)) == -1) {
					perror("error");
					return 1;
				}

				if ((sharedMemory = shmat(shmid, NULL, 0)) == (void *)-1) {
					perror("error");
					return 1;
				}

				cal_data = (SendData *)sharedMemory;

				for (;;) {
					if (semop(semid, &semopen, 1) == -1) {
						perror("error");
						return 1;
					}
	
					write(client_arr[1].fd, cal_data, sizeof(SendData));
	
					semop(semid, &semclose, 1);

					sleep(1);
				}
			}
		}
		else {
			perror("failed to read");
			close(client_arr[1].fd);
			return -1;
		}
	} 
	
	// 자식 프로세스 3
	if (pid3 == 0) {
		if ((n = read(client_arr[2].fd, &recvData, sizeof(RecvData))) > 0) {

			// 입력받은 값 출력
			printf("Read Data %s(%d) : %d and %s\n",
				client_arr[2].addr, client_arr[2].port, 
				recvData.num, recvData.msg);

			// sendData 크기만큼 공유 메모리 할당
			strcpy(sendData->msg, recvData.msg);
			sendData->num = &recvData.num;
			int shmid, semid;
			void *sharedMemory = NULL;

			// 소비자 프로세스 생성
			int pid_cons = fork();

			// 생산자, 소비자 프로세스는 각각 공유 메모리에 순차적으로 접근하여 처리한다.
			if (pid_cons > 0) {
				SendData *cal_data;

				if ((shmid = shmget((key_t)65538, sizeof(SendData), 0666|IPC_CREAT)) == -1) {
					perror("shmget error");
					return 1;
				}

				if ((semid = semget((key_t)68000, 1, IPC_CREAT|0666)) == -1) {
					perror("semget error");
					return 1;
				}

				if ((sharedMemory = shmat(shmid, NULL, 0)) == (void *)-1) {
					perror("sharedmemory error");
					return 1;
				}

				cal_data = (SendData *)sharedMemory;
				memset(cal_data->msg, 0, MAXBUF);
				*(cal_data->num) = *(sendData->num);

				semun_arr[2].val = 1;
				if (semctl(semid, 0, SETVAL, semun_arr[2]) == -1) {
					perror("semctl3 error");
					return 1;
				}

				char temp;
				int s_length = strlen(sendData->msg) - 1;

				char result[MAXBUF] = {0, };
				strcpy(result, sendData->msg);

				for (;;) {
					if (semop(semid, &semopen, 1) == -1) {
						perror("semop error");
						return 1;
					}

					for (int i = 0; i < s_length; i++) {
						temp = result[i];
						result[i] = result[i + 1];
						result[i + 1] = temp;
				 	}

					strcpy(cal_data->msg, result);

					(*cal_data->num)++;
				
					semop(semid, &semclose, 1);

					sleep(1);
				}
			}

			if (pid_cons == 0) {
				SendData *cal_data;

				if ((shmid = shmget((key_t)65538, sizeof(SendData), 0666)) == -1) {
					perror("shmget error");
					return 1;
				}

				if ((semid = semget((key_t)68000, 0, 0666)) == -1) {
					perror("semget error");
					return 1;
				}

				if ((sharedMemory = shmat(shmid, NULL, 0)) == (void *)-1) {
					perror("sharedmemory error");
					return 1;
				}

				cal_data = (SendData *)sharedMemory;

				for (;;) {
					if (semop(semid, &semopen, 1) == -1) {
						perror("semop error");
						return 1;
					}
					
					write(client_arr[2].fd, cal_data, sizeof(SendData));
	
					semop(semid, &semclose, 1);

					sleep(1);
				}
			}
		}

		else {
			perror("failed to read");
			close(client_arr[2].fd);
			return -1;
		}
	}
	
	return 0;
}	
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

// 클라이언트와 주고받을 구조체
struct Data {
    char msg[MAXBUF];
    int num;
};

union semun {
	struct Data val;
};

struct Data* handleData(struct Data *data) {
	char temp;

	// 문자열 처리
	for (int i = 0; i < strlen(data->msg) - 1; i++) {
		temp = data->msg[i];
		data->msg[i] = data->msg[i + 1];
		data->msg[i + 1] = temp;
	}

	// 정수 처리
	data->num += 1;

	return data;
}

int main(int argc, char **argv) {
	// 시그널 핸들러 함수 호출
    signal(SIGINT, (void *)sig_handler); 

	// 세마포어
	struct sembuf semopen = {0, -1, SEM_UNDO};
	struct sembuf semclose = {0, 1, SEM_UNDO};

	struct Data myData = {{0, }, -1};

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
		
		if ((n = read(client_arr[0].fd, &myData, sizeof(myData))) > 0) {

			// 입력받은 값 출력
			printf("Read Data %s(%d) : %d and %s\n",
				client_arr[0].addr, client_arr[0].port, 
				myData.num, myData.msg);

			// myData 크기만큼 공유 메모리 및 세마포어 할당 및 생성
			int shmid, semid;
			void *sharedMemory = NULL;
			struct Data *sharedData;
			union semun sem_union;

			if ((shmid = shmget((key_t)1234, sizeof(myData), 0666|IPC_CREAT)) == -1) {
				return 1;
			}

			if ((semid = semget((key_t)3477, 1, IPC_CREAT|0666)) == -1) {
				return 1;
			}

			if ((sharedMemory = shmat(shmid, NULL, 0)) == (void *)-1) {
				return 1;
			}

			sharedData = (struct Data *)sharedMemory;

			strcpy(sem_union.val.msg, myData.msg);
			sem_union.val.num = myData.num;
		
			if (semctl(semid, 0, SETVAL, sem_union) == -1) {
				return 1;
			}

			// 생산자, 소비자 프로세스 생성
			int pid_prod, pid_cons;

			pid_prod = fork(); // 생산자

			if (pid_prod > 0) {
				pid_cons = fork(); // 소비자
			}

			// 생산자, 소비자 프로세스는 각각 공유 메모리에 순차적으로 접근하여 처리한다.
			for (;;) {
				if (pid_prod == 0) {
					struct Data local_var;

					if (semop(semid, &semopen, 1) == -1) {
						return 1;
					}

					local_var = *handleData(sharedData);

					sleep(1);

					sharedData = &local_var;

					semop(semid, &semclose, 1);
				}

				if (pid_cons == 0) {
					struct Data local_var;

					if (semop(semid, &semopen, 1) == -1) {
						return 1;
					}

					local_var = *handleData(sharedData);
					
					sleep(1);

					write(client_arr[0].fd, &sharedData, sizeof(sharedData));

					sharedData = &local_var;

					semop(semid, &semclose, 1);
				}
			}

		} else {
			perror("failed to read");
			close(client_arr[0].fd);
			return -1;
		}
	} 

	// 자식 프로세스 2
	if (pid2 == 0) {
		if ((n = read(client_arr[1].fd, &myData, sizeof(myData))) > 0) {

			// 입력받은 값 출력
			printf("Read Data %s(%d) : %d and %s\n",
				client_arr[1].addr, client_arr[1].port, 
				myData.num, myData.msg);

			// myData 크기만큼 공유 메모리 및 세마포어 할당 및 생성
			int shmid, semid;
			void *sharedMemory = NULL;
			struct Data *sharedData;
			union semun sem_union;

			if ((shmid = shmget((key_t)1234, sizeof(myData), 0666|IPC_CREAT)) == -1) {
				return 1;
			}

			if ((semid = semget((key_t)3477, 1, IPC_CREAT|0666)) == -1) {
				return 1;
			}

			if ((sharedMemory = shmat(shmid, NULL, 0)) == (void *)-1) {
				return 1;
			}

			sharedData = (struct Data *)sharedMemory;

			strcpy(sem_union.val.msg, myData.msg);
			sem_union.val.num = myData.num;
		
			if (semctl(semid, 0, SETVAL, sem_union) == -1) {
				return 1;
			}

			// 생산자, 소비자 프로세스 생성
			int pid_prod, pid_cons;

			pid_prod = fork(); // 생산자

			if (pid_prod > 0) {
				pid_cons = fork(); // 소비자
			}

			// 생산자, 소비자 프로세스는 각각 공유 메모리에 순차적으로 접근하여 처리한다.
			for (;;) {
				if (pid_prod == 0) {
					semop(semid, &semopen, 1);
					sharedData = handleData(sharedData);
					write(client_arr[1].fd, &sharedData, sizeof(sharedData));
					sleep(1);
					semop(semid, &semclose, 1);
				}

				if (pid_cons == 0) {
					semop(semid, &semopen, 1);
					sharedData = handleData(sharedData);
					write(client_arr[1].fd, &sharedData, sizeof(sharedData));
					sleep(1);
					semop(semid, &semclose, 1);
				}
			}

		} else {
			perror("failed to read");
			close(client_arr[1].fd);
			return -1;
		}
	} 
	
	// 자식 프로세스 3
	if (pid3 == 0) {
		if ((n = read(client_arr[2].fd, &myData, sizeof(myData))) > 0) {

			// 입력받은 값 출력
			printf("Read Data %s(%d) : %d and %s\n",
				client_arr[2].addr, client_arr[2].port, 
				myData.num, myData.msg);

			// myData 크기만큼 공유 메모리 및 세마포어 할당 및 생성
			int shmid, semid;
			void *sharedMemory = NULL;
			struct Data *sharedData;
			union semun sem_union;

			if ((shmid = shmget((key_t)1234, sizeof(myData), 0666|IPC_CREAT)) == -1) {
				return 1;
			}

			if ((semid = semget((key_t)3477, 1, IPC_CREAT|0666)) == -1) {
				return 1;
			}

			if ((sharedMemory = shmat(shmid, NULL, 0)) == (void *)-1) {
				return 1;
			}

			sharedData = (struct Data *)sharedMemory;

			strcpy(sem_union.val.msg, myData.msg);
			sem_union.val.num = myData.num;
		
			if (semctl(semid, 0, SETVAL, sem_union) == -1) {
				return 1;
			}

			// 생산자, 소비자 프로세스 생성
			int pid_prod, pid_cons;

			pid_prod = fork(); // 생산자

			if (pid_prod > 0) {
				pid_cons = fork(); // 소비자
			}

			// 생산자, 소비자 프로세스는 각각 공유 메모리에 순차적으로 접근하여 처리한다.
			for (;;) {
				if (pid_prod == 0) {
					semop(semid, &semopen, 1);
					sharedData = handleData(sharedData);
					write(client_arr[2].fd, &sharedData, sizeof(sharedData));
					sleep(1);
					semop(semid, &semclose, 1);
				}

				if (pid_cons == 0) {
					semop(semid, &semopen, 1);
					sharedData = handleData(sharedData);
					write(client_arr[2].fd, &sharedData, sizeof(sharedData));
					sleep(1);
					semop(semid, &semclose, 1);
				}
			}

		} else {
			perror("failed to read");
			close(client_arr[2].fd);
			return -1;
		}
	}
	
	return 0;
}	
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/socket.h> 
#include <sys/un.h> 
#include <stdio.h> 
#include <string.h> 
#include <unistd.h> 
 
#define MAXLINE 1024 
 
int main(int argc, char **argv) 
{ 
	int sockfd; 
	int clilen; 
	char buf[MAXLINE]; 
	struct sockaddr_un sock_addr; 

	if(argc != 2) 
	{ 
		printf("Usage : %s [socket file name]\n", argv[0]); 
		return 1; 
	} 

	if (access(argv[1], R_OK)  == -1) 
	{ 
		printf("socket File access error\n"); 
		return 1; 
	} 
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0); 
 
	memset((void *)&sock_addr, 0x00, sizeof(sock_addr)); 
	sock_addr.sun_family = AF_UNIX; 
	strncpy(sock_addr.sun_path, argv[1], strlen(argv[1])); 
	clilen = sizeof(sock_addr); 
	connect(sockfd, (struct sockaddr *)&sock_addr, clilen); 
 
	memset(buf, 0x00, MAXLINE); 
	read(0, buf, MAXLINE); // 사용자로부터 문자열 입력받기
	write(sockfd, buf, strlen(buf)); // 서버로 데이터 전송
	read(sockfd, buf, MAXLINE);  // 결과 문자열 받기
	printf("Server -> %s", buf); // 출력
	
	return 0; 
}
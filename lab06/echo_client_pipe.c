#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define READ 0
#define WRITE 1

#define MAXLINE 1024
int main(int argc, char **argv)
{
	int fd[2];
	char buf[MAXLINE];

	fd[READ] = open("/tmp/myfifo_w", O_RDWR);
	if(fd[READ] < 0)
	{
		perror("read open error\n");
		return 1;
	}
	fd[WRITE] = open("/tmp/myfifo_r", O_RDWR);
	if(fd[WRITE] < 0)
	{
		perror("write open error\n");
		return 1;
	}
	while(1)
	{
		printf("> ");
		fflush(stdout);
		memset(buf, 0x00, MAXLINE);
		if(read(0, buf, MAXLINE) < 0)
		{
			printf("error\n");
			return 1;
		}
		if(strncmp(buf, "quit\n",5) == 0) break;
		write(fd[WRITE], buf, strlen(buf));
		read(fd[READ], buf, MAXLINE);
		printf("Server -> %s", buf);
	}
	close(fd[WRITE]);
	close(fd[READ]);
	return 0;
}


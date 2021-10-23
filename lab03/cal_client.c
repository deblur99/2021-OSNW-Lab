#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 3600
#define IP "127.0.0.1"

int main(int argc, char **argv)
{
    struct sockaddr_in addr;
    int s;
    int sbyte, rbyte;
    int num;
    char received_string[200];

    if (argc != 2)
    {
   	 printf("Usage : %s [num]\n", argv[0]);
   	 return 1;
    }

    num = atoi(argv[1]);

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == -1)
    {
   	 return 1;
    }
   
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);

    if ( connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1 )
    {
   	 printf("fail to connect\n");
   	 close(s);
   	 return 1;
    }

    num = htonl(num);

    sbyte = write(s, &num, sizeof(num));
    if(sbyte != sizeof(num))
    {
   	 return 1;
    }

    rbyte = read(s, (char *)received_string, sizeof(received_string));
    if(rbyte != sizeof(received_string))
    {
   	 return 1;
    }

    printf("%s\n", received_string); 

    close(s);
    return 0;
}

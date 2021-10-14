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

#define MAX_SOCKET 3

struct send_data {
        int min;
        int max;
        float avg;
        char min_addr[30];
        int min_port;
        char max_addr[30];
        int max_port;
        char rstring[200];
};

struct client_info {
        int id;
        char address[20];
        unsigned short port;
        int input_data;
};

void init_rdata(struct send_data *sdata) {
        sdata->min = 0;
        sdata->max = 0;
        sdata->avg = 0.0f;
        memset(sdata->min_addr, 0, sizeof(sdata->min_addr));
        memset(sdata->max_addr, 0, sizeof(sdata->max_addr));
        memset(sdata->rstring, 0, sizeof(sdata->rstring));
}

int main(int argc, char **argv)
{       
        struct sockaddr_in client_addr, sock_addr;
        int listen_sockfd, client_sockfd;
        int addr_len;
        struct send_data sdata;
        int rbyte, sbyte;
        
        struct client_info client_info_arr[MAX_SOCKET];

        init_rdata(&sdata);

        if( (listen_sockfd  = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
                perror("Error ");
                return 1;
        }

        memset((void *)&sock_addr, 0x00, sizeof(sock_addr));
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        sock_addr.sin_port = htons(PORT);

        if( bind(listen_sockfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1)
        {
                perror("Error ");
                return 1;
        }

        if(listen(listen_sockfd, 5) == -1)
        {
                perror("Error ");
                return 1;
        }

        // connect 3 clients with a server
        for (int i = 0; i < MAX_SOCKET; i++) {
                addr_len = sizeof(client_addr);

                client_info_arr[i].id = accept(listen_sockfd,
                        (struct sockaddr *)&client_addr, &addr_len);

                if(client_info_arr[i].id == -1) {
                        perror("Error ");
                        return 1;
                }

                strcpy(client_info_arr[i].address, inet_ntoa(client_addr.sin_addr));
                client_info_arr[i].port = ntohs(client_addr.sin_port);

                printf("New Client[%d] Connect : %s:%d\n", i, client_info_arr[i].address, client_info_arr[i].port);
        }
                
        for (int i = 0; i < MAX_SOCKET; i++) {
                rbyte = read(client_info_arr[i].id, &client_info_arr[i].input_data, sizeof(client_info_arr[i].input_data));

                if (rbyte <= 0) {
                        printf("failed to read socket %d\n", i);
                        close(client_info_arr[i].id);
                }

                client_info_arr[i].input_data = htonl(client_info_arr[i].input_data);

                sdata.avg += client_info_arr[i].input_data;
        }

        sdata.max = client_info_arr[0].input_data;
        sdata.min = client_info_arr[0].input_data;

        for (int i = 0; i < MAX_SOCKET; i++) {
                if (sdata.max <= client_info_arr[i].input_data) {
                        sdata.max = client_info_arr[i].input_data;
                        memset(sdata.max_addr, 0, sizeof(sdata.max_addr));
                        strcpy(sdata.max_addr, client_info_arr[i].address);
                        sdata.max_port = client_info_arr[i].port;
                }
                        

                if (sdata.min >= client_info_arr[i].input_data) {
                        sdata.min = client_info_arr[i].input_data;
                        memset(sdata.min_addr, 0, sizeof(sdata.min_addr));
                        strcpy(sdata.min_addr, client_info_arr[i].address);
                        sdata.min_port = client_info_arr[i].port;
                }       
        }

        sdata.avg /= 3;

        sprintf(sdata.rstring, "min=%d from %s:%d\nmax=%d from %s:%d\navg=%.1f",
                sdata.min, sdata.min_addr, sdata.min_port, sdata.max, sdata.max_addr, sdata.max_port, sdata.avg);

        for (int i = 0; i < MAX_SOCKET; i++) {
                sbyte = write(client_info_arr[i].id, (char *)sdata.rstring, sizeof(sdata.rstring));

                if (sbyte == -1) {
                        printf("failed to write socket %d\n", i);
                }

                close(client_info_arr[i].id);
        }

        close(listen_sockfd);
        return 0;
}

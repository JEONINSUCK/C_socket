#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFSIZE 1024

void error_handling(char *msg);
void *cln_msg_handling(void *arg);
void *serv_msg_handling(void *arg);

int cln_sock_list[5] = {0,};
int cln_sock_num = 0;

int main(int argc, char *argv[])
{
    
    int serv_sock;
    int cln_sock;
    int cln_addr_sz;
    pthread_t t_read, t_wrtie;
    
    struct sockaddr_in serv_addr;
    struct sockaddr_in cln_addr;
    
    if(argc != 2)
        error_handling("Not enough argument");
    
    // Server socket make
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    
    // Server address setting
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    // Server init
    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    if(listen(serv_sock, 5) == -1)
        error_handling("listen() error");
    
    // Main loop
    while(1)
    {
        pthread_create(&t_wrtie, NULL, serv_msg_handling, NULL);
        cln_addr_sz = sizeof(cln_addr);
        cln_sock = accept(serv_sock, (struct sockaddr*)&cln_addr, &cln_addr_sz);
        if(cln_sock == -1)
        {
            error_handling("accept() error");
            break;
        }
        else
        {
            cln_sock_list[cln_sock_num++] += cln_sock;
            pthread_create(&t_read, NULL, cln_msg_handling, (void*)&cln_sock);
            printf("%s is connected\n", inet_ntoa(cln_addr.sin_addr));
            printf("%d\n", cln_sock);
            printf("%d\n", cln_sock_num);
        }
    }
    close(serv_sock);
    return 0;
}

void *serv_msg_handling(void *arg)
{
    char write_buf[BUFSIZE] = {0,};
    int i;
    while(1)
    {
        scanf("%s", write_buf);
        for(i=0; i<cln_sock_num; i++)
            write(cln_sock_list[i], write_buf, strlen(write_buf));
    }
    return NULL;
}

void *cln_msg_handling(void *arg)
{
    char read_buf[BUFSIZE] = {0,};
    int i;
    int sock = *((int*)arg);
    int str_len = 0;
    while((str_len=read(sock, read_buf, BUFSIZE)) != 0)
    {
        printf("send: %s\n\r", read_buf);
        write(sock, read_buf, str_len);
        memset(&read_buf, 0, sizeof(read_buf));
    }
    for(i=0; i<cln_sock_num; i++)
    {
        if(sock == cln_sock_list[i])
        {
            while(i++<cln_sock_num-1)
                cln_sock_list[i] = cln_sock_list[i+1];
            printf("%d,",cln_sock_list[i]);
        }
        else
            printf("%d,",cln_sock_list[i]);
    }
    printf("\n\r");
    printf("%d is out\n\r", sock);
    cln_sock_num--;
    close(sock);
    return NULL;
}

void error_handling(char *msg)
{
    puts(msg);
    exit(1);
}

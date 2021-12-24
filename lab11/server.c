#include<stdio.h>
#include<stdlib.h>
#include<string.h>  //strlen
#include<sys/socket.h>
#include<arpa/inet.h>   //inet_addr
#include <fcntl.h>
#include <unistd.h>

int sock,attached_socket;
//char *message;
int reply[1024];

#define MSG_LEN 10

int cmp(const void *a, const void *b) {
     return *(int*)a - *(int*)b;
 }

void service_process(){
    //pid_t pid;
    switch(fork()) {
        case 0:
            printf("Ослуживающий процесс PID=%d\n", getpid());
            close(sock);
            if(recv(attached_socket, reply , 1024 , 0) > 0){
                printf("Сервер принял данные\n");
                qsort(reply, MSG_LEN, sizeof(int), cmp);
                printf("Сервер отсортировал и отправил данные\n");
            }
            send(attached_socket, reply, MSG_LEN*4, 0);
            close(attached_socket);
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
    }
    
}

int main(int argc , char *argv[])
{
    struct sockaddr_in server , client;
    int rv;
    struct timeval tv;

    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Ошибка при создании сокета\n");
    }
    printf("Процесс-сервер %d создал сокет\n", getpid());

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
 
    if( bind(sock,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("bind не произошел\n");
        exit(EXIT_FAILURE);
    }
    printf("Произошла привязка к сетевому адресу\n");
    
    listen(sock , 5);
    
    printf("Сервер ожидает подключения клиентов...\n");

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    tv.tv_sec = 5;

    while(1){
        rv = select(sock+1, &readfds, NULL, NULL, &tv); 
        if(rv > 0){
            attached_socket = accept(sock, (struct sockaddr *)&client, (socklen_t*)&client);
            if(attached_socket>0){
                printf("Подключился клиент\n");
                service_process();      
            }
        }
        else{
            printf("Timeout 5 секунд\n");
            break;
        }   
        tv.tv_sec = 5;
    }
    
    close(sock);
    return 0;
}
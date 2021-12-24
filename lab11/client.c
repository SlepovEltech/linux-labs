#include<stdio.h>
#include<stdlib.h>
#include<string.h>  //strlen
#include<sys/socket.h>
#include<arpa/inet.h>   //inet_addr
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define MSG_LEN 10

int *arr;

void generate_nums(){
    arr = (int*)malloc(MSG_LEN);
    printf("Сгенерированная последовательность: ");
    for(int i = 0; i < MSG_LEN; i++){
        arr[i] = rand() % 21;
        printf("%d ", arr[i]);
    }
    printf("\n");
}

 
int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    int server_reply[1024];
    int rv;
    struct timeval tv;

    srand(time(NULL));
    
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);

    if (sock == -1)
    {
        printf("Ошибка при создании сокета\n");
    }
        
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    tv.tv_sec = 5;
    rv = select(sock+1, &readfds, NULL, NULL, &tv);
    if(rv > 0){
        if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0){
            printf("No connect\n");
        }
        else{
            printf("Присоединились к серверу\n");
            generate_nums();   

            //Send some data
            if( send(sock , arr , MSG_LEN*4 , 0) < 0)
            {
                printf("Отправка не удалась\n");
                exit(EXIT_FAILURE);
            }
            printf("Отправили данные\n"); 
            
            //Receive a reply from the server
            if( recv(sock, server_reply , 1024 , 0) < 0)
            {
                printf("Прием не удался\n");
                exit(EXIT_FAILURE);
            }
            printf("Сортироваанная сервером последовательность: ");
            for(int i = 0; i < MSG_LEN; i++){
               printf("%d ", server_reply[i]);
            }
            printf("\n");
        }
    }else{
        printf("Timeout 5 секунд");
    }
    
    return 0;
}
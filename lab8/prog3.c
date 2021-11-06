#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#define QUE_KEY 123

int general_que, local_que;
int progNum = 3;

int nextNum(int shift){
	if(shift == 1 &&  progNum == 1)	return 2;
	if(shift == 1 &&  progNum == 2) return 3;	 
	if(shift == 1 &&  progNum == 3) return 1;
	if(shift == 2 &&  progNum == 1)	return 3;
	if(shift == 2 &&  progNum == 2) return 1;	 
	if(shift == 2 &&  progNum == 3) return 2;
}

typedef struct MyMessage_T {
	long mtype;
	int sender;
	time_t time;
	int response_que;
	//char msg;
} MyMessage;

int main(){

	char queOwner = 1;

	general_que = msgget(QUE_KEY, 0606|IPC_CREAT|IPC_EXCL);
	if(general_que == -1){
		general_que =  msgget(QUE_KEY, 0606|IPC_CREAT);
		queOwner = 0;
	}
	local_que = msgget(IPC_PRIVATE, 0606|IPC_CREAT);

	printf("\tProgNum: %d. Local que: %d. General que: %d\n", progNum, local_que, general_que);

	MyMessage myRequest, recieveGlobalMsg[2], recieveLocalMsg[2], myResponse;
	time_t myRequestTime = time(0);
	myRequest.time = myRequestTime;
	myRequest.mtype = nextNum(1);
	myRequest.response_que = local_que;
	myRequest.sender = progNum;
	msgsnd(general_que,&myRequest,sizeof(myRequest),0);
	myRequest.mtype = nextNum(2);
	msgsnd(general_que,&myRequest,sizeof(myRequest),0); 

	printf("Запросы на чтение отправлены программам %d и %d. Время %ld\n", nextNum(1), nextNum(2), myRequestTime);

	
    myResponse.sender = progNum;

	int readPermission = 0;
	int recieveLocalNum = 0, recieveGlobalNum = 0;
    while(recieveLocalNum < 2){
    	if(msgrcv(general_que, &recieveGlobalMsg[recieveGlobalNum], 
    		      sizeof(recieveGlobalMsg[recieveGlobalNum]), progNum, IPC_NOWAIT)!=-1){
    		printf("Получили запрос от %d. Время запроса: %ld\n", recieveGlobalMsg[recieveGlobalNum].sender, recieveGlobalMsg[recieveGlobalNum].time);

    		if(recieveGlobalMsg[recieveGlobalNum].time < myRequestTime){
    			recieveGlobalMsg[recieveGlobalNum].time = 0;
    			printf("Запрос от %d свежее. Отправка разрешения\n", recieveGlobalMsg[recieveGlobalNum].sender);
    			myResponse.mtype = recieveGlobalMsg[recieveGlobalNum].sender;
    			myResponse.time = time(0);
    			msgsnd(recieveGlobalMsg[recieveGlobalNum].response_que, &myResponse, sizeof(myResponse),0); 
    		}
    		recieveGlobalNum+=1;
    	}

    	if(msgrcv(local_que, &recieveLocalMsg[recieveLocalNum], sizeof(recieveLocalMsg[recieveLocalNum]), 0, IPC_NOWAIT)!=-1){
    		printf("Получили разрешение от %d. Время отправки разрешения: %ld\n", recieveLocalMsg[recieveLocalNum].sender, recieveLocalMsg[recieveLocalNum].time);
    		recieveLocalNum += 1;
    	}
    	//printf("recieveLocalNum: %d, recieveGlobalNum: %d\n", recieveLocalNum, recieveGlobalNum);
    	sleep(1);
    }
    	

    printf("\nВсе разрешения получены. Начинается чтение файла\n");
    FILE *inputFile = fopen("input.txt","r");
    char str[32];
    while(fgets(str,32,inputFile))
    	printf("%s",str);
    fclose(inputFile);
	printf("Чтение закончено\n\n");


	if(recieveGlobalMsg[0].time > 0){
		myResponse.mtype = recieveGlobalMsg[0].sender;
		myResponse.time = time(0);
		msgsnd(recieveGlobalMsg[0].response_que, &myResponse, sizeof(myResponse),0); 
		printf("Отправили разрешение в %d очередь программы %d\n", recieveGlobalMsg[0].response_que, recieveGlobalMsg[0].sender);
	}
	
	if(recieveGlobalMsg[1].time > 0){
		myResponse.time = time(0);
		myResponse.mtype = recieveGlobalMsg[1].sender;
		msgsnd(recieveGlobalMsg[1].response_que, &myResponse, sizeof(myResponse),0); 
		printf("Отправили разрешение в %d очередь программы %d\n", recieveGlobalMsg[1].response_que, recieveGlobalMsg[1].sender);
	}

	
    if(queOwner)
    	msgctl(general_que,IPC_RMID,NULL);
    msgctl(local_que,IPC_RMID,NULL);
	return 0;
}
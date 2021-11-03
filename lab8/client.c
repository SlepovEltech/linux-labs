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
int prog_name = 2;

typedef struct MyMessage_T {
	long mtype;
	int sender;
	time_t time;
	int response_que;
	char msg;
} MyMessage;

int main(){

	general_que = msgget(QUE_KEY, 0622|IPC_CREAT|IPC_EXCL);
	if(general_que == -1){
		general_que =  msgget(QUE_KEY, 0622|IPC_CREAT);
	}
	local_que = msgget(IPC_PRIVATE, 0622|IPC_CREAT);


	MyMessage request, response;

	request.time = time(0);
	request.mtype = 1;
	request.response_que = local_que;
	request.sender = prog_name;
	request.msg = 'Y';

    while(1)
    	if(msgrcv(general_que,&response,sizeof(response),0,IPC_NOWAIT) != -1)
    		break;
    	
    msgsnd(response.response_que,&request,sizeof(request),0);

    printf("Message %c\n", response.msg);
    printf("Mtype %ld\n", response.mtype);
    printf("Sender %d\n", response.sender);

    msgctl(local_que,IPC_RMID,NULL);

	return 0;
}
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>



#define NAME 'B'
#define MSG_EXCEPT 8192
#define HEADER sizeof(long)

#define NORM(x) ((x=='D')?'A':((x=='E')?'B':x))

struct CustomMsgbuf{
    long mtype;
    char from;
    time_t time;
    int response;
} buf,bufRecv;

int common,private;


void sendResponse(long delayed){
    struct CustomMsgbuf sendPacket;
    sendPacket.mtype = 1;
    sendPacket.from = NAME ;
    sendPacket.time = time(0);
    sendPacket.response = 0;
    int Sendto = (delayed!=0)?delayed:bufRecv.response;
    msgsnd(Sendto,&sendPacket,sizeof(sendPacket)-HEADER,0);
}


void readFile(){
    FILE *toRead = fopen("file.txt","r");
    char str[32];
    fgets(str,32,toRead);
    printf("Read=%s",str);
    fclose(toRead);
}

int main(int argc, char *argv[]){
    int FirstInstance = 1;
    key_t key;
    time_t timeASK = 0;
    int read = 0;
    struct CustomMsgbuf delayed[3];
    int delInx = -1;
    memset(&buf,0,sizeof(buf));
    memset(&delayed,0,sizeof(buf));

    key = ftok("salt1",1);
    common = msgget(key,IPC_CREAT|IPC_EXCL|0606);
    if(common<0){
        common = msgget(key,IPC_CREAT);
        FirstInstance = 0;
    }
    private = msgget(NAME,IPC_CREAT|0606);


    printf("My id=%c\n",NAME);
    printf("Common=%d private=%d\n",common,private);
        printf("%c %c\n",NORM(NAME+1),NORM(NAME+2));
    while((read<4) || (delInx>-1)){
        switch(read){
            case 0:
                buf.from = NAME;
                timeASK = time(0);
                buf.time = timeASK;
                buf.response = private;
                buf.mtype = NORM(NAME+1);
                
                msgsnd(common,&buf,sizeof(buf)-HEADER,0);
                buf.mtype = NORM(NAME+2);
                msgsnd(common,&buf,sizeof(buf)-HEADER,0);
                
                printf("[%d] request for reading...\n",time(0));
                read = 1;
                memset(&buf,0,sizeof(buf));
            break;
            case 1:
            case 2:
                    if (msgrcv(private,&buf,sizeof(buf)-HEADER,0,IPC_NOWAIT)!=-1){
                    printf("[%d] response: from=%c time=%d\n",time(0),buf.from,buf.time);
                    read++;
                    memset(&buf,0,sizeof(buf));
                }
            break;
            case 3:
                printf("[%d] All responses are received. Reading file...\n",time(0));
                readFile();
                read++;
            break;
        }
        if(msgrcv(common,&bufRecv,sizeof(bufRecv)-HEADER,NAME,IPC_NOWAIT)!=-1){
            printf("Got request from %c - ",bufRecv.from);
            if(read>3){
                    sendResponse(0);
                    printf("[%d] Response sent!\n",time(0));
            }
            else
                if(bufRecv.time<timeASK){
                    sendResponse(0);
                    printf("[%d] Response sent!\n",time(0));
                }
                else{
                    printf("Pushing request from %c to line\n",bufRecv.from);
                    delInx++;
                    memcpy(&(delayed[delInx]),&bufRecv,sizeof(bufRecv));
                }

        }
        if(delInx>-1)
            if((read>2) || (delayed[delInx].time<timeASK)){
                    sendResponse(delayed[delInx].response);
                    printf("[%d] Delayed request from %c - sent\n",time(0),delayed[delInx--].from);
                }
    sleep(1);
    }
    if(FirstInstance){
        sleep(2);
        msgctl(common,IPC_RMID,NULL);
    }
    msgctl(private,IPC_RMID,NULL);
    return 0;
}
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

using namespace std;

struct request_msg{
	long mtype;
	int sender;
	int response_msq;
	int request_time;
};

struct response_msg{
	long mtype;
	int sender;
};

int main(){
	bool is_creator;
	int request_time;
	int request_queue;
	int response_queue;
	int permissions_get = 0;
	int permissions_send = 0;
	int readers_end = 0;
	request_msg req_msg;
	request_msg request_msg_buf[2];
	int buf_size;
	response_msg resp_msg;
	response_msg resp_buf;
	int prog_id = 3;

	request_queue = msgget(1489, 0606|IPC_CREAT|IPC_EXCL);
	
	//создаем общая очередь
	if(request_queue != -1){
		is_creator = true;
		cout<<"Общая очередь создана"<<endl;
	}else{
		request_queue = msgget(1489, IPC_CREAT);
		if(request_queue == -1){
			cout<<"Не удается подключиться к общей очереди"<<endl;
			return 0;
		}else{
			cout<<"Подключен к общей очереди"<<endl;
		}
	}
	
	//локальная очередь
	response_queue = msgget(IPC_PRIVATE, 0606|IPC_CREAT);
	if(response_queue == -1){
		cout<<"Не удается создать локальную очередь"<<endl;
		if(is_creator)
			msgctl(request_queue, IPC_RMID, NULL);

		return 1;
	}else{
		cout<<"Локальная очередь создана!"<<endl;
	}


	request_time = time(NULL);
	req_msg.request_time = request_time;
	req_msg.mtype = (prog_id)%3 + 1;
	req_msg.sender = prog_id;
	req_msg.response_msq = response_queue;
	msgsnd(request_queue, &req_msg, sizeof(request_msg), 0);
	req_msg.mtype = (prog_id+1)%3 + 1;
	msgsnd(request_queue, &req_msg, sizeof(request_msg), 0);


	resp_msg.mtype = 1;
	resp_msg.sender = prog_id;

	while(permissions_get<2){
		//проверка сообщения из общей очереди
		if(msgrcv(request_queue, &request_msg_buf[buf_size], sizeof(request_msg), prog_id, IPC_NOWAIT)!=-1){

			cout<<"Получен запрос на прием от "<<request_msg_buf[buf_size].sender<<endl;
			cout<<"Очередь: "<<request_msg_buf[buf_size].request_time<<endl;

			if(request_msg_buf[buf_size].request_time < request_time
			  || (request_msg_buf[buf_size].request_time == request_time && request_msg_buf[buf_size].sender < prog_id)){

				msgsnd(request_msg_buf[buf_size].response_msq, &resp_msg, sizeof(response_msg), 0);

				permissions_send++;
				cout<<"Отправка разрешения на доступ к "<<request_msg_buf[buf_size].sender<<endl;
			}else{
				buf_size++;
			}
		}

		//проверка сообщений в локальная очереди
		if(msgrcv(response_queue, &resp_buf, sizeof(response_msg), 1, IPC_NOWAIT)!=-1)
		{
			permissions_get++;

			cout<<"Получено разрешение на чтение от"<<resp_buf.sender<<endl;
			cout<<"Очередь: "<<time(NULL)<<endl;
		}
	}

	cout<<"Начало чтения файла"<<endl;

	fstream in("input.txt");
	string line;
	
	while(!in.eof() && getline(in, line)){
		cout<<line<<endl;
	}
	cout<<"Чтение завершено, закрытие файла"<<endl;
	in.close();

	while(buf_size){
		msgsnd(request_msg_buf[buf_size - 1].response_msq,
				&resp_msg,
				sizeof(response_msg), 0);
	
		permissions_send++;
		cout<<"Отправка разрешения на доступ к "<<request_msg_buf[buf_size - 1].sender<<endl;
		buf_size--;
	}

	while(permissions_send<2){
		//проверка сообщения из общей очереди
                if(msgrcv(request_queue, &request_msg_buf[buf_size], sizeof(request_msg), prog_id, IPC_NOWAIT)!=-1){
			msgsnd(request_msg_buf[buf_size].response_msq,
                                        &resp_msg, sizeof(response_msg), 0);

                        permissions_send++;

			cout<<"Отправка разрешения на доступ к "<<request_msg_buf[buf_size].sender<<endl;
                }
                
	}       

	//удаление очередей
	req_msg.mtype = 0;
	msgsnd(request_queue, &req_msg, sizeof(request_msg), 0);
                    
	if(is_creator){
		while(readers_end<3){
	 		if(msgrcv(request_queue, &req_msg, sizeof(request_msg),
						0, 0)!=-1){
				readers_end++;
			}   	
	 	}

		msgctl(request_queue, IPC_RMID, 0);
	 }

	msgctl(response_queue, IPC_RMID, 0);
		
	return 0;
}                   


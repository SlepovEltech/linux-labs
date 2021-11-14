#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/shm.h>

#define SEM_KEY 111

//0 - мьютекс файла 
//1 - семафор numProcess
//2 - семафор numWriters
//3 - семафор numReaders
int semId;

//операции над семафорами
struct sembuf writer_dec_sem = {0, -1, 0};
struct sembuf writer_inc_sem = {0, 1, 0};

struct sembuf numwriters_dec = {2, -1 , 0};
struct sembuf numwriters_inc = {2, 1 , 0};
struct sembuf numreaders_dec = {3, -1 , 0};
struct sembuf numreaders_inc = {3, 1 , 0};

struct sembuf is_writer_zero = {2, 0 , 0};

FILE *file;
int main(int argc, char **argv){
    int pid = getpid();
    semId = semget(SEM_KEY,4,IPC_CREAT|IPC_EXCL|0666);
    
	if(semId > 0){
		file = fopen("output.txt","w");
	    fclose(file); //очистка файла
	    printf("Процесс %d создал семафор\n",pid);
	}else{
	 	semId = semget(SEM_KEY,4,IPC_CREAT);
	 	printf("Используем существующий семафор %d\n", semId);

	}
	struct sembuf sem_init = {0, 1, 0};    
    sem_init.sem_num = 1;
    semop(semId,&sem_init,1); //общее количество процессов	

	char flag = 1;
	int curStr = 0;
	char buf[256];

	semop(semId, &numreaders_inc, 1);
	printf("\nПроцесс читатель ждет, пока писатель запишет\n");

	semop(semId, &is_writer_zero, 1);
	//while(semctl( semId, 2, GETVAL, 0 ) > 0);

	file = fopen("output.txt", "r");
	int i = 0;
	i = 0;
	while(fgets(buf, 256, file)){
		printf("Процесс-читатель %d прочитал строку №%d \n\t %s\n",pid, i, buf);
		i+=1;
	}
	flag = 0;
	fclose(file);
	semop(semId, &numreaders_dec, 1);
	

	struct sembuf inc_all = {1, -1, 0};
	semop(semId, &inc_all, 1);
	if(semctl( semId, 1, GETVAL, 0 ) == 0){
	        semctl(semId,IPC_RMID,0);
            
            printf("Семафор уничтожены\n");
	}
	return 0;
}
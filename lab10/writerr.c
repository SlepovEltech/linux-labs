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

struct sembuf is_reader_zero = {3, 0, 0};

FILE *file;
int main(int argc, char **argv){
	if(argc == 2){
		int writeNum = atoi(argv[1]);

	    int pid = getpid();
	    struct sembuf sem_init = {0, 1, 0};
	    semId = semget(SEM_KEY,4,IPC_CREAT|IPC_EXCL|0666);
		if(semId > 0){
			file = fopen("output.txt","w");
	    	fclose(file); //очистка файла
	    	printf("Процесс %d создал семафор\n",pid);
	    	semop(semId,&sem_init,1); //мьютекс файла	
		}else{
		 	semId = semget(SEM_KEY,4,IPC_CREAT);
		 	printf("Используем существующий семафор %d\n", semId);
		}
		sem_init.sem_num = 1;  
	    semop(semId,&sem_init,1); //общее количество процессов	

    	//sem_init.sem_num = 2;
    	//semop(semId,&sem_init,1); //кол-во писателей

		printf("semid: %d\n", semId);
		
		for(int i=0;i<writeNum;i++){
			semop(semId,&numwriters_inc, 1);

		    semop(semId, &is_reader_zero, 1);
			//while(semctl(semId, 3, GETVAL, 0 ) > 0);
			
			printf("Процесс-писатель ожидает освобождения writer mutex\n");
			semop(semId, &writer_dec_sem, 1);
			printf("\tПроцесс %d записал %d/%d\n", pid, i, writeNum-1);
			file = fopen("output.txt", "a");
			fprintf(file,"Процесс-писатель %d  %d/%d\n",pid, i, writeNum-1);
			fclose(file);
			printf("Процесс-писатель освободил writer mutex\n\n");
			semop(semId, &writer_inc_sem, 1);

			semop(semId,&numwriters_dec, 1);
			sleep(1);
		}
		
		struct sembuf inc_all = {1, -1, 0};
		semop(semId, &inc_all, 1);
		if(semctl( semId, 1, GETVAL, 0 ) == 0){
	        semctl(semId,IPC_RMID,0);
            
            printf("Семафор уничтожены\n");
		}
	}
	else{
		printf("Недостаточное количество аргументов: ./name <количество операций>");
	}
	return 0;
}
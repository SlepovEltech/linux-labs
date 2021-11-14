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
#define MEM_KEY 222

typedef struct shared_data {
    int numWriters;
    int numReaders;
} shared;

shared* sharedVar;
//0 - мьютекс писателя
//1 - семафор numProcess
//2 - мьютекс numWriters
//3 - мьютекс numReaders
int semId;

//операции над семафорами
struct sembuf writer_catch_sem = {0, -1, 0};
struct sembuf writer_release_sem = {0, 1, 0};
struct sembuf reader_catch_sem = {1, -1, 0};
struct sembuf reader_release_sem = {1, 1, 0};

struct sembuf numwriters_catch = {2, -1 , 0};
struct sembuf numwriters_release = {2, 1 , 0};
struct sembuf numreaders_catch = {3, -1 , 0};
struct sembuf numreaders_release = {3, 1 , 0};

int getNumWriters(){
    semop(semId, &numwriters_catch, 1);
    int tmp = sharedVar->numWriters;
    semop(semId, &numwriters_release, 1);
    return tmp;
}

int getNumReaders(){

    semop(semId, &numreaders_catch, 1);
    int tmp = sharedVar->numReaders;
    semop(semId, &numreaders_release, 1);
}

void incNumWriters(){
	//printf("In incNumReaders\n");
    semop(semId, &numwriters_catch, 1);
    sharedVar->numWriters++;
    semop(semId, &numwriters_release, 1);
}

void incNumReaders(){
	//printf("In incNumReaders\n");
    semop(semId, &numreaders_catch, 1);
    sharedVar->numReaders++;
    semop(semId, &numreaders_release, 1);
    //printf("Success incNumReaders\n");
}

void decNumWriters(){

    semop(semId, &numwriters_catch, 1);
    sharedVar->numWriters--;
    semop(semId, &numwriters_release, 1);
}

void decNumReaders(){

    semop(semId, &numreaders_catch, 1);
    sharedVar->numReaders--;
    semop(semId, &numreaders_release, 1);
}


FILE *file;
int main(int argc, char **argv){
	int shmId = shmget(MEM_KEY, sizeof(shared), (0666|IPC_CREAT|IPC_EXCL));
    if(shmId<0)
    	shmId = shmget(MEM_KEY, sizeof(shared), (0666|IPC_CREAT));
    void* shmAddr = shmat(shmId,0,0);
    if(*(int*)shmAddr == -1)
    	exit(EXIT_FAILURE);

    sharedVar = (shared*)shmAddr;
    int pid = getpid();
    semId = semget(SEM_KEY,4,IPC_CREAT|IPC_EXCL|0666);
    
	if(semId > 0){
		file = fopen("output.txt","w");
	    fclose(file); //очистка файла
	    printf("Процесс %d инициализирует семафоры\n",pid);
	    // мьютекс писателей
	    struct sembuf sem_init = {0, 1, 0};
	    semop(semId,&sem_init,1);
	    // семафор читателей
	    sem_init.sem_num = 1;
	    semop(semId,&sem_init,1);
	    // мьютекс numWriters
	    sem_init.sem_num = 2;
	    semop(semId,&sem_init,1);
	    // мьютекс numReaders
	    sem_init.sem_num = 3;
	    semop(semId,&sem_init,1);
	    sharedVar->numReaders = 0;
	    sharedVar->numWriters = 0;
	}else{
	 	semId = semget(SEM_KEY,4,IPC_CREAT);
	 	printf("Используем существующий семафор %d\n", semId);
	 // 	int val =semctl( semId, 0, GETVAL, 0 );
		// printf("Текущее значение 0 sem: %d\n", val);
		// val =semctl( semId, 1, GETVAL, 0 );
		// printf("Текущее значение 1 sem: %d\n", val);
		// val =semctl( semId, 2, GETVAL, 0 );
		// printf("Текущее значение 2: %d\n", val);
		// val =semctl( semId, 3, GETVAL, 0 );
		// printf("Текущее значение 3 sem: %d\n", val);	 	
	 	struct sembuf sem_init = {1, 1, 0};
		semop(semId, &sem_init,1);  //общее количество процессов		
		// int val =semctl( semId, 0, GETVAL, 0 );
		// printf("Текущее значение 0 sem: %d\n", val);
		// val =semctl( semId, 1, GETVAL, 0 );
		// printf("Текущее значение 1 sem: %d\n", val);
		// val =semctl( semId, 2, GETVAL, 0 );
		// printf("Текущее значение 2: %d\n", val);
		// val =semctl( semId, 3, GETVAL, 0 );
		// printf("Текущее значение 3 sem: %d\n", val);

	}
	char flag = 1;
	int curStr = 0;
	char buf[256];
	while(flag){
		incNumReaders();
		printf("\nПроцесс читатель ждет, пока писатель запишет\n");
		while(getNumWriters() != 0);
		
		file = fopen("output.txt", "r");
		int i = 0;
		//while(fgets(buf, 256, file) && (i++ < curStr));
		//curStr += 1;
		//flag = !feof(file);
		// if(flag)
		// 	printf("Процесс-читатель %d прочитал строку №%d \n\t %s\n",pid, i, buf);
		// else
		// 	printf("Никто ничего не дописал в файл\n");
		i = 0;
		while(fgets(buf, 256, file)){
			printf("Процесс-читатель %d прочитал строку №%d \n\t %s\n",pid, i, buf);
			i+=1;
		}
		flag = 0;
		fclose(file);
		decNumReaders();
		sleep(1);
	}

	struct sembuf release_all = {1, -1, 0};
	semop(semId, &release_all, 1);
	if(semctl( semId, 1, GETVAL, 0 ) == 0){
	        semctl(semId,IPC_RMID,0);
            shmdt(shmAddr);
            semctl(shmId,IPC_RMID,0);
            printf("Семафор  и разделяемый сегмент уничтожены\n");
	}
	return 0;
}
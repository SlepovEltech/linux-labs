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
    //int totalNum;
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
struct sembuf numProcess_catch_sem = {1, -1, 0};
struct sembuf numProcess_release_sem = {1, 1, 0};

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
    return tmp;
}

void incNumWriters(){
	//printf("In incNumReaders\n");
    semop(semId, &numwriters_catch, 1);
    sharedVar->numWriters++;
    semop(semId, &numwriters_release, 1);
    //printf("Close incNumReaders\n");
}

void incNumReaders(){
	
    semop(semId, &numreaders_catch, 1);
    sharedVar->numReaders++;
    semop(semId, &numreaders_release, 1);
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
	if(argc == 2){
		int writeNum = atoi(argv[1]);

		int shmId = shmget(MEM_KEY, sizeof(shared), (0666|IPC_CREAT|IPC_EXCL));
	    if(shmId<0)
	    	shmId = shmget(MEM_KEY, sizeof(shared), (0666|IPC_CREAT));
	    void* shmAddr = shmat(shmId,0,0);
	    if(*(int*)shmAddr == -1)
	    	exit(EXIT_FAILURE);

	    sharedVar = (shared*)shmAddr;
	    printf("SHARED %d %d", sharedVar->numReaders, sharedVar->numWriters);
	    int pid = getpid();
	    semId = semget(SEM_KEY,4,IPC_CREAT|IPC_EXCL|0666);
		if(semId>0){
			file = fopen("output.txt","w");
		    fclose(file); //очистка файла
		    printf("Процесс %d инициализирует семафоры\n",pid);
		    // мьютекс писателей
		    struct sembuf sem_init = {0, 1, 0};
		    semop(semId,&sem_init,1);
		    //printf("Процесс инициализировал семафоры 0\n");

		    // семафор читателей
		    sem_init.sem_num = 1;
		    semop(semId,&sem_init,1);
		    //printf("Процесс инициализировал семафоры 1\n");
		    // мьютекс numWriters
		    sem_init.sem_num = 2;
		    semop(semId,&sem_init,1);
		    //printf("Процесс инициализировал семафоры 2\n");
		    // мьютекс numReaders
		    sem_init.sem_num = 3;
		    semop(semId,&sem_init,1);
		    //printf("Процесс инициализировал семафоры 3\n");
		    sharedVar->numReaders = 0;
	    	sharedVar->numWriters = 0;
		}else{
			printf("Используем существующий семафор\n");
		 	semId = semget(SEM_KEY,4,IPC_CREAT);
		    struct sembuf sem_init = {1, 1, 0};
		    semop(semId,&sem_init,1);  //общее количество процессов

		 //    int val =semctl( semId, 0, GETVAL, 0 );
			// printf("Текущее значение 0 sem: %d\n", val);
			// val =semctl( semId, 1, GETVAL, 0 );
			// printf("Текущее значение 1 sem: %d\n", val);
			// val =semctl( semId, 2, GETVAL, 0 );
			// printf("Текущее значение 2: %d\n", val);
			// val =semctl( semId, 3, GETVAL, 0 );
			// printf("Текущее значение 3 sem: %d\n", val);
		}
		printf("semid: %d\n", semId);
		for(int i=0;i<writeNum;i++){
			//printf("Before incNumWriters\n");
			incNumWriters();
			while(getNumReaders() != 0); //printf("Есть читатели!\n");
			semop(semId,&writer_catch_sem,1);
			printf("Процесс-писатель ожидает освобождения writer mutex\n");
			semop(semId, &writer_catch_sem, 0);
			printf("\tПроцесс %d записал %d/%d\n", pid, i+1, writeNum);
			file = fopen("output.txt", "a");
			fprintf(file,"Процесс-писатель %d  %d/%d\n",pid, i+1, writeNum);
			fclose(file);
			printf("Процесс-писатель освободил writer mutex\n\n");
			semop(semId, &writer_release_sem, 1);
			decNumWriters();

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
	}
	else{
		printf("Недостаточное количество аргументов: ./name <количество операций>");
	}
	return 0;
}
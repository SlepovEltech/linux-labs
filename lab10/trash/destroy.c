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

int main(){

	int semId = semget(SEM_KEY,4,IPC_CREAT|IPC_EXCL|0666);
	if(semId > 0){
	    printf("Процесс инициализирует семафор %d\n", semId);
		struct sembuf sem_init = {0, 0, 0};   
	    //semop(semId,&sem_init,1);
	    int val =semctl( semId, 0, GETVAL, 0 );
		printf("Текущее значение 0 sem: %d\n", val);
		val =semctl( semId, 1, GETVAL, 0 );
		printf("Текущее значение 1 sem: %d\n", val);
		val =semctl( semId, 2, GETVAL, 0 );
		printf("Текущее значение 2: %d\n", val);
		val =semctl( semId, 3, GETVAL, 0 );
		printf("Текущее значение 3 sem: %d\n", val);

		semctl(semId,IPC_RMID,0);
        printf("Семафор уничтожен\n");

        int shmId = shmget(MEM_KEY, sizeof(shared), (0666|IPC_CREAT|IPC_EXCL));
	    if(shmId<0)
	    	shmId = shmget(MEM_KEY, sizeof(shared), (0666|IPC_CREAT));
	    void* shmAddr = shmat(shmId,0,0);
	    if(*(int*)shmAddr == -1)
	    	exit(EXIT_FAILURE);

	    sharedVar = (shared*)shmAddr;
	    printf("SHARED %d %d", sharedVar->numReaders, sharedVar->numWriters);
	    
	    printf("SHARED %d %d", sharedVar->numReaders, sharedVar->numWriters);
	}else{
		printf("Используем существующий семафор\n");
	 	semId = semget(SEM_KEY,4,IPC_CREAT);
	 	int val =semctl( semId, 0, GETVAL, 0 );
		printf("Текущее значение 0 sem: %d\n", val);
		val =semctl( semId, 1, GETVAL, 0 );
		printf("Текущее значение 1 sem: %d\n", val);
		val =semctl( semId, 2, GETVAL, 0 );
		printf("Текущее значение 2: %d\n", val);
		val =semctl( semId, 3, GETVAL, 0 );
		printf("Текущее значение 3 sem: %d\n", val);

		semctl(semId,IPC_RMID,0);
        printf("Семафор уничтожен\n");
        int shmId = shmget(MEM_KEY, 4096, (0666|IPC_CREAT));
        shmctl(shmId, IPC_RMID, 0);

	}
	return 0;
}
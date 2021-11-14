#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#define NUM_PROCESS 3
#define MAX(a,b) (a>b)?a:b

typedef struct shared_data {
    int choosing[NUM_PROCESS];
    int number[NUM_PROCESS];
} shared;

void lock(shared*, int);
void unlock(shared*, int);

// Алгоритм Лампорта (булочной)
void lock(shared* shared_var, int process) {
    shared_var->choosing[process] = 1;
    shared_var->number[process] = 1 + MAX(shared_var->number[0], MAX(shared_var->number[1], shared_var->number[2]));
    shared_var->choosing[process] = 0;
    for (int i = 0; i < NUM_PROCESS; ++i) {
        if (i != process) {
            while (shared_var->choosing[i]);
            while (shared_var->number[i] != 0 && (shared_var->number[process] > shared_var->number[i] ||
                (shared_var->number[process] == shared_var->number[i] && process > i)));
        }
    }
    
}

void unlock(shared* shared_var, int process) {
    shared_var->number[process] = 0;
}

shared* sharedVar;
int progNum;

void signal_handler(int sig){
	if (sig == SIGALRM) {
		printf("Программа №%d ожидает освобождения общей переменной\n", progNum);
        lock(sharedVar, progNum);

        FILE *file = fopen("output.txt", "a");
        time_t curTime = time(NULL);
        fprintf(file, "Программа №%d PID: %d, Время: %s",progNum, getpid(), ctime(&curTime));
        fclose(file);

        unlock(sharedVar, progNum);
        printf("Программа №%d  освободила общую переменную\n", progNum);
      
    }
}

void main(int argc, char** argv){
	if(argc == 4){
		progNum = atoi(argv[1]);
		int numStarts = atoi(argv[2]);
		int period = atoi(argv[3]);
		signal(SIGALRM, signal_handler);
    	
    	struct itimerval timer_value;
	    timerclear(&timer_value.it_interval);
	    timerclear(&timer_value.it_value);
	    timer_value.it_interval.tv_sec = period;
	    timer_value.it_value.tv_sec = period;
	    setitimer(ITIMER_REAL, &timer_value, NULL);


	    int shmId = shmget(123, sizeof(shared), (0666 | IPC_CREAT));
	    if(shmId != -1)
	    	printf("Программа №%d получила id=%d разделяемого сегмента\n", progNum, shmId);
	    else
	    	exit(EXIT_FAILURE);
	    void* shmAddr = shmat(shmId,0,0);
	    if(*(int*)shmAddr != -1)
	    	printf("Программа №%d присоединила разделяемый сегмент\n\n", progNum);
	    else
	    	exit(EXIT_FAILURE);

	    sharedVar = (shared*)shmAddr;

	    for(int i = 0; i < numStarts; i++){
	    	pause();
	    }

	    if(shmdt(shmAddr) != -1)
	    	printf("\nПрограмма №%d отсоединила разделяемый сегмент\n", progNum);
	    
	}
	else
		printf("Недостаточно аргументов для запуска: ./prog НОМЕР_ПРОГРАММЫ КОЛ-ВО_ЗАПУСКОВ ПЕРИОД\n");
	exit(EXIT_SUCCESS);
}
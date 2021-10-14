#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUF_SIZE 256

typedef struct thread_args_{
	FILE *file;
	char *str;
}thread_args;


void *thread_routine(void *args){
	thread_args *arg = (thread_args*)args;
	fputs(arg->str, arg->file);
}

int main(int argc, char **argv){
	FILE *inputFile;
	if(argc == 2)
		inputFile = fopen(argv[1],"r");
	else
		inputFile = fopen("input.txt","r");

	thread_args thr_arg_1, thr_arg_2;
	thr_arg_1.file = fopen("output1.txt","w");
	thr_arg_2.file = fopen("output2.txt","w");
	char eofFlag = 0;
	char buf1[BUF_SIZE], buf2[BUF_SIZE];
	pthread_t thread_1 = 0, thread_2 = 0;

	if(inputFile && thr_arg_1.file && thr_arg_2.file){
		printf("Read file start\n");
		while(!feof(inputFile)){
			if(thr_arg_1.str = fgets(buf1, BUF_SIZE, inputFile)){
				if(pthread_create(&thread_1, NULL, &thread_routine, &thr_arg_1))
					printf("Thread_1 create ERROR");

			}
			
			if(thr_arg_2.str = fgets(buf2, BUF_SIZE, inputFile)){
				if(pthread_create(&thread_2, NULL, &thread_routine, &thr_arg_2))
					printf("Thread_2 create ERROR");

			}
			
			pthread_join(thread_1, NULL);
			pthread_join(thread_2, NULL);

			if(feof(inputFile))
				printf("Read file end\n");
		}
		fclose(inputFile);
		fclose(thr_arg_1.file);
		fclose(thr_arg_2.file);
	}
	else{
		printf("Open file error\n");
	}
	return 0;
}
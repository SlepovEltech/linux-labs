#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

FILE *output = NULL;
int read_pipe_fd;          
char ch;
char readFlag = 1;
char sigquitFlag = 0;

int readReturn;

void signal_handler(int sig){
	if (sig == SIGQUIT) {
        printf("Потомок 2 получил SIGQUIT родителя\n");
        sigquitFlag = 1;
    }
    if (sig == SIGUSR1){
    	printf("Потомок 2 получил SIGUSR1 и продолжает работать\n");
    	readReturn = read(read_pipe_fd, &ch, 1);
    	if(readReturn > 0){
			printf("Потомок 2: %c\n", ch);
			fprintf(output, "%c", ch);
			readFlag = 1;		
    		kill(0, SIGUSR2);
    	}
    	else{
    		readFlag = 0;
    		if(sigquitFlag == 0){
    			printf("Потомок 2: канал пуст, предок не закончил писать\n");
    			kill(0, SIGUSR1);
    		}
    		if(sigquitFlag == 1){
    			printf("Потомок 2: канал пуст, предок закончил писать\n");
    			kill(0, SIGUSR2);
    			printf("Происходит выход из потомка 2\n");
    			fclose(output);
    			exit(EXIT_SUCCESS);
    		}
    	}
			
    }

    if (sig == SIGUSR2){
    	//printf("Потомок 2 получил свой же SIGUSR2\n");	
    }
}


int main(int argc, char** argv){
	if(argc == 3){
		read_pipe_fd = *argv[1];          
		output = fopen(argv[2], "a");

		signal(SIGQUIT, signal_handler);
		signal(SIGUSR1, signal_handler);
		signal(SIGUSR2, signal_handler);

		while(readFlag || !sigquitFlag) 
			pause();
	}
	else
		printf("Недостаточно аргументов для запуска\n");
	return 0;
}
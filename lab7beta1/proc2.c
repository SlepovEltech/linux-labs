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
int readFlag = 1;

int readReturn;

void signal_handler(int sig){
	if (sig == SIGQUIT) {
        printf("Потомок 2 получил SIGQUIT родителя\n");
    }
    if (sig == SIGUSR1){
    	printf("Потомок 2 получил SIGUSR1 и продолжает работать\n");
    	readReturn = read(read_pipe_fd, &ch, 1);
    	if(readReturn > 0){
    		if(ch != 10){
    			printf("Потомок 2: %c\n", ch);
    			fprintf(output, "%c", ch);		
    		}else{
    			readFlag = 0;
    		}
    		kill(0, SIGUSR2);
    	}
    	else{
    		readFlag = 0;
    		printf("Потомок 2: канал пуст\n");
    	}
			
    }

    if (sig == SIGUSR2){
    	printf("Потомок 2 получил свой же SIGUSR2\n");	
    }
}


int main(int argc, char** argv){
	if(argc == 3){
		read_pipe_fd = *argv[1];          
		output = fopen(argv[2], "a");

		signal(SIGQUIT, signal_handler);
		signal(SIGUSR1, signal_handler);
		signal(SIGUSR2, signal_handler);

		while(readFlag)
			pause();
		fclose(output);
		printf("Происходит выход из потомка 2\n");
		exit(EXIT_SUCCESS);
	}
	else
		printf("Недостаточно аргументов для запуска\n");
	return 0;
}
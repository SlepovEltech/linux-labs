#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>


FILE *output = NULL;
int read_pipe_fd;          
char ch;
int readFlag = 1;
int readReturn;

void signal_handler(int sig){
	if (sig == SIGQUIT) {
        printf("\nПотомок 1 получил SIGQUIT и начинает работать\n");
        read(read_pipe_fd, &ch, 1);
		fputc(ch, output);
        printf("Потомок 1: %c\n", ch);
        kill(0, SIGUSR1);	

    }
    if (sig == SIGUSR2){
    	printf("\nПотомок 1 получил SIGUSR2 и продолжает работать\n");
    	readReturn = read(read_pipe_fd, &ch, 1);
    	//printf("Потомок 1 read: %d байт\n", readReturn);
    	if(readReturn > 0){
    		if(ch != 10){
    			printf("Потомок 1: %c\n", ch);
    			fprintf(output, "%c", ch);
	        	kill(0, SIGUSR1);
        	}else{
    			readFlag = 0;
    			kill(0, SIGUSR1);
        	}
    	}
    	else{
    		readFlag = 0;
    		printf("Потомок 1: канал пуст\n");
    	}
    }

    if (sig == SIGUSR1){
    	printf("Потомок 1 получил свой же SIGUSR1\n");	
    }
}

int main(int argc, char** argv){
	if(argc == 3){
		read_pipe_fd = *argv[1];          

		signal(SIGQUIT, signal_handler);
		signal(SIGUSR1, signal_handler);
		signal(SIGUSR2, signal_handler);
		output = fopen(argv[2], "a");

		while(readFlag)
			pause();
		fclose(output);
		printf("Происходит выход из потомка 1\n");
		exit(EXIT_SUCCESS);

	}
	else
		printf("Недостаточно аргументов для запуска\n");
	return 0;
}
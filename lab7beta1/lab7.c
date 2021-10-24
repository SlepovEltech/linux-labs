#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

#define BUF_SIZE 128


void signal_handler(int sig){
	if (sig == SIGQUIT) {
        printf("Родитель получил свой же SIGQUIT\n");
    }
}

int main(int argc, char** argv){
	if(argc == 4){
		char buf[BUF_SIZE];
		FILE *input = NULL;
		int fd[2];
		pid_t pid_1, pid_2;

	    signal(SIGQUIT, signal_handler);
		signal(SIGUSR1, signal_handler);
		signal(SIGUSR2, signal_handler);

		if (pipe(fd) == -1) {
        	printf("Ошибка создания канала\n");
        	exit(EXIT_FAILURE);
    	}
    	if ( (input = fopen(argv[1], "r")) == NULL) {
        	printf("Ошибка открытия входного файла\n");
        	exit(EXIT_FAILURE);
    	}

    	if (!(pid_1 = fork())) {
    		//printf("Запускаем потомок1\n");
    		execl("proc1", "proc1", &fd[0], argv[2], NULL);
    	}
    	
    	if (!(pid_1 = fork())) {
    		execl("proc2", "proc2", &fd[0], argv[3], NULL);
    	}
    	sleep(2);

    	fcntl(*fd, F_SETFL, O_NONBLOCK); 

    	while (fgets(buf, BUF_SIZE, input) != NULL) {
	        if (write(fd[1], buf, strlen(buf)) == -1) {
	           	printf("Ошибка записи в канал\n");
	            exit(EXIT_FAILURE);
	        }
	    }

    	printf("Родитель закончил писать в канал и послал SIGQUIT\n");
    	kill(0, SIGQUIT);

    	waitpid(pid_1, NULL, 0);
    	printf("Родитель дождался 1 потомка и завершает работу\n");
    	waitpid(pid_2, NULL, 0);
    	printf("Родитель дождался 2 потомка и завершает работу\n");
    	
    	close(fd[0]);
    	close(fd[1]);
    	fclose(input);
	}
	else
		printf("Недостаточно аргументов для запуска\n");
	return 0;
}
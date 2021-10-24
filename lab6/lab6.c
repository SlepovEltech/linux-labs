#include <sys/time.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

void signal_handler(int sig){
	if (sig == SIGALRM) {
        if (fork() == 0) {
            time_t start_time = time(NULL);
            printf("\nPID: %d\nДата и время старта: %s", getpid(), ctime(&start_time));
            exit(EXIT_SUCCESS);
        }
    }
}

void main(int argc, char** argv){
	if(argc == 3){
		int numStarts = atoi(argv[1]);
		int period = atoi(argv[2]);
		signal(SIGTSTP, SIG_IGN);
		signal(SIGALRM, signal_handler);
    	
    	struct itimerval timer_value;
	    timerclear(&timer_value.it_interval);
	    timerclear(&timer_value.it_value);
	    timer_value.it_interval.tv_sec = period;
	    timer_value.it_value.tv_sec = period;
	    setitimer(ITIMER_REAL, &timer_value, NULL);

		time_t pt1,pt2;
		clock_t p1, p2; 
		clock_t pt3, pt4; 
		pt1 = time(NULL);
		p1 = clock();
	    for(int i = 0; i < numStarts; i++){
	    	pause();
	    	pt3 = clock();
	    	wait(0);
	    	pt4 = clock();
	    	printf("Потомок %d: время работы %lu тиков = %ld сек\n", i, pt4-pt3, (pt4-pt3)/ CLOCKS_PER_SEC);

	    }
	    p2  = clock();
	    pt2 = time(NULL);
		printf("\nПроцесс-родитель PID %d: время работы %lu сек \n", getpid(), pt2-pt1);
	}
	else
		printf("Недостаточно аргументов для запуска\n");
	exit(EXIT_SUCCESS);
}
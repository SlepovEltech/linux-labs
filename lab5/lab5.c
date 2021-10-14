#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void signal_handler(int sig) {
    switch (sig)
    {
    case SIGFPE:
        printf("Сигнал SIGFPE (неверная операция) \nКод завершения: 1\n");
        exit(1);
        break;
    case SIGSEGV:
        printf("Сигнал SIGSEGV (нарушение защиты памяти) \nКод завершения: 2\n");
        exit(2);
        break;
    default:
    	printf("Необработанный сигнал");
        break;
    }
}

int main(int argc, char* argv[]){
	signal(SIGFPE, signal_handler);
	signal(SIGSEGV, signal_handler);
	int option, a = 10;
	int *pointer  = NULL;
	if(argc == 2){
		option = atoi(argv[1]);
		if(option == 1){
			a = a / 0;
		}
		if(option == 2){
			printf("%d", *pointer);
		}
	}
	else
		printf("Нет аргументов для запуска");

	return 0;
}
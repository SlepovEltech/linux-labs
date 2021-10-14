#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void print_attributes(char* processName, FILE* file) {
    pid_t pid = getpid();
    fprintf(file,"%s:\nPID: %d\nPPID: %d\nSID: %d\nPGID: %d\nUID: %d\nEUID: %d\nGID: %d\nEGID: %d\n\n",
    					processName, pid, getppid(), getsid(pid), getpgid(pid), getuid(), geteuid(), getgid(), getegid());
}


int main(int argc, char* argv[]){
	if(argc == 5){
		int parent_delay, fork_delay, vfork_delay;
		parent_delay = atoi(argv[1]);
		fork_delay = atoi(argv[2]);
		vfork_delay = atoi(argv[3]);

		FILE* file = fopen(argv[4], "w");
		fprintf(file, "Задержка родителя: %d; Задержка fork: %d; Задержка vfork: %d\n", parent_delay, fork_delay, vfork_delay);
		fclose(file);
		

		if((file = fopen(argv[4], "a")) ){	
			if(fork() == 0){
				sleep(fork_delay);
				print_attributes("Процесс fork", file);
				printf("\nfork bye bye\n");
				exit(EXIT_SUCCESS);
			}
			if(vfork() == 0){
				sleep(vfork_delay);
				execl("lab2_1", "lab2_1", argv[4], NULL);
			}
			sleep(parent_delay);
			print_attributes("Процесс-родитель", file);
			fclose(file);
			printf("\nParent bye bye\n");
		}
		else{
			printf("Ошибка открытия файла %s!", argv[4]);
		}
	}
	else{
		printf("Неправильное число аргументов!\n");
		printf("Формат вызова: задержка_родителя задержка_fork задержка_vfork имя_файла\n");
	}
		
	return 0;
}



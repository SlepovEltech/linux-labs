#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types>


void print_attributes(char* processName, FILE* file) {
    pid_t pid = getpid();
    fprintf(file,"%s:\nPID: %d\nPPID: %d\nSID: %d\nPGID: %d\nUID: %d\nEUID: %d\nGID: %d\nEGID: %d\n\n",
    					processName, pid, getppid(), getsid(pid), getpgid(pid), getuid(), geteuid(), getgid(), getegid());
}


int main(int argc, char* argv[]){
    FILE *file;

    if((file = fopen(argv[1], "a")) ){
        print_attributes("vfork процесс, запущенный через подмену программы", file);
        printf("\nvfork bye bye\n");

    }
    return 0;
}
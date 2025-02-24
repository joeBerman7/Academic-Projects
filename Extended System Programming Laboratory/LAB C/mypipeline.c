#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <linux/limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char**argv){
    int fd[2];
    pipe(fd);

    fprintf(stderr,"(parent_process>forkin...)\n");
    pid_t child1=fork();
    if(child1==0){
        fprintf(stderr,"(child1>redirecting stdout to the write end of the pipe...)\n");
        close(fileno(stdout));
        dup(fd[1]);
        close(fd[1]);
        char *argv_child1[]={"ls","-l",NULL};
        fprintf(stderr,"(child1>going to execute cmd: ...)\n");
        execvp("ls",argv_child1);
        printf("child 1 failed");
        exit(1);
    }
    fprintf(stderr,"(parent_process>created process with id:%d )\n",child1);

    fprintf(stderr,"(parent_process>closing the write end of the pipe...)\n");
    close(fd[1]);

    fprintf(stderr,"(parent_process>forkin...)\n");
    pid_t child2=fork();
    if(child2==0){
        fprintf(stderr,"(child2>redirecting stdout to the read end of the pipe...)\n");
        close(fileno(stdin));
        dup(fd[0]);
        close(fd[0]);
        char *argv_child2[]={"tail","-n 2",NULL};
        fprintf(stderr,"(child2>going to execute cmd: ...)\n");
        execvp("tail",argv_child2);
        printf("child 2 failed");
        exit(1);
    }
    fprintf(stderr,"(parent_process>created process with id:%d )\n",child2);

    fprintf(stderr,"(parent_process>closing the read end of the pipe...)\n");
    

    fprintf(stderr,"(parent_process>waiting for child process to terminate...)\n");
    waitpid(child1,NULL,0);
    waitpid(child2,NULL,0);
    
    fprintf(stderr,"(parent_process>exiting...)\n");
}
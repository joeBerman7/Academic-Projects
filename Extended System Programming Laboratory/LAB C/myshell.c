#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "LineParser.h"
#include <linux/limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int d=0;
int HISTLEN=10;

enum status{
    TERMINATED=-1,
    RUNNING=1,
    SUSPENDED=0
};

typedef struct process{
    cmdLine* cmd;                         /* the parsed command line*/
    pid_t pid; 		                  /* the process id that is running the command*/
    int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next;	                  /* next process in chain */
} process;

void execute(cmdLine *pCmdLine){  
    if(d){
        fprintf(stderr,"PID=%d, Executing command=%s\n",getpid(),(*pCmdLine).arguments[0]);
    }
    if((*pCmdLine).inputRedirect!=NULL){
        close(fileno(stdin));
        open((*pCmdLine).inputRedirect,O_RDONLY);
    }
    if((*pCmdLine).outputRedirect!=NULL){
        close(fileno(stdout));
        open((*pCmdLine).outputRedirect,O_WRONLY|O_CREAT);
    }
    execvp((*pCmdLine).arguments[0],(*pCmdLine).arguments);
    perror("execv failed: child process is not created");
    _exit(1);
}

void pipe_(cmdLine *pCmdLine){
    if(((*pCmdLine).outputRedirect!=NULL)|((*(*pCmdLine).next).inputRedirect!=NULL)){
        fputs("error: redirection failed\n",stderr);
        return;
    }
    int fd[2];
    pipe(fd);

    pid_t child1=fork();
    if(child1==0){
        close(fileno(stdout));
        dup(fd[1]);
        close(fd[1]);
        if((*pCmdLine).inputRedirect!=NULL){
            close(fileno(stdin));
            open((*pCmdLine).inputRedirect,O_RDONLY);
        }
        execvp((*pCmdLine).arguments[0],(*pCmdLine).arguments);
        perror("execv failed: child process is not created3");
        _exit(1);
    }
    close(fd[1]);

    pCmdLine=(*pCmdLine).next;

    pid_t child2=fork();
    if(child2==0){
        close(fileno(stdin));
        dup(fd[0]);
        close(fd[0]);
        if((*pCmdLine).outputRedirect!=NULL){
            close(fileno(stdout));
            open((*pCmdLine).outputRedirect,O_WRONLY|O_CREAT);
        }
        execvp((*pCmdLine).arguments[0],(*pCmdLine).arguments);
        perror("execv failed: child process is not created2");
        _exit(1);
    }
    close(fd[0]);
    waitpid(child1,NULL,0);
    waitpid(child2,NULL,0); 
    exit(0);
}

void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* newp=malloc(sizeof(process));
    (*newp).next=*process_list;
    (*newp).cmd=cmd;
    (*newp).pid=pid;
    (*newp).status=RUNNING;
    *process_list=newp;
    
}
void freeProcessList(process* process_list){
    if((*process_list).next!=NULL){
        freeProcessList((*process_list).next);
    }
    freeCmdLines((*process_list).cmd);
    free(process_list);
}

void updateProcessList(process **process_list){
   process *curr=*process_list;
    while(curr!=NULL){
        int status;
        int ret=waitpid((*curr).pid,&status,WNOHANG|WUNTRACED|WCONTINUED);
        if(ret > 0){
            if(WIFSTOPPED(status)){
                (*curr).status=SUSPENDED;
            }
            if(WIFSIGNALED(status)||WIFEXITED(status)){
                (*curr).status=TERMINATED;
            }
            if(WIFCONTINUED(status)){
                (*curr).status=RUNNING;
            }
        }
        curr=(*curr).next;
    }  
}
void deleteTerminatedProcessFromList(process** process_list, process* terminated){
    process *curr=*process_list;
    if((*curr).pid==(*terminated).pid){
        *process_list=(*curr).next;
        (*curr).next=NULL;
        freeProcessList(curr);
        return;
    }
    while((*(*curr).next).pid!=(*terminated).pid){
        curr=(*curr).next;
    }
    (*curr).next=(*(*curr).next).next; 
    (*terminated).next=NULL;
    freeProcessList(terminated);
}

void printProcessList(process** process_list){
    updateProcessList(process_list);
    process *curr=*process_list;
    int index=0;
    printf("  PID  STATUS  Command\n");
    while(curr!=NULL){
        printf("%d %d ",index,(*curr).pid);
        (*curr).status==RUNNING ? printf("RUNNING"):
        (*curr).status==TERMINATED ? printf("TERMINATED"):
        (*curr).status==SUSPENDED ? printf("SUSPENDED"):
        printf("Invalid status");
        
        for(int i=0; i<(*(*curr).cmd).argCount; i++){
            printf(" %s",(*(*curr).cmd).arguments[i]);
        }
        process* p=curr;
        curr=(*curr).next;
        if((*p).status==TERMINATED){
            deleteTerminatedProcessFromList(process_list,p);
        }
        printf("\n");
        
        index++;
    }
}


void updateProcessStatus(process* process_list, int pid, int status){
    process *curr=process_list;
    while(curr!=NULL){
        if((*curr).pid==pid){
            (*curr).status=status;
        }
        curr=(*curr).next;
    }  
}

int isHistoryReapeatNumberCmd(char cmd[]){
    if(strcmp(cmd,"!!\n")==0){
        return 1;
    }
    if(cmd[0]=='!'){
        int len=strlen(cmd)-1;
       for(int i=1; i<len; i++){
           if((cmd[i]<'0')||(cmd[i]>'9')){
               return 0;
           }
       }
       return 1;       
    }
    if(strcmp(cmd,"history\n")==0){
        return 1;
    }
    return 0;
}



int main(int argc, char** argv){
    for(int i=0; i<argc; i++){
        if(strcmp(argv[i],"-d")==0){
            d=1;
            break;
        }
    }
    process** process_list=NULL;
    char* queue[HISTLEN];
    int countCmd=0;
    int readInput=1;
    while(1){
        char read[2048];
        if(readInput){
            char buf[PATH_MAX];
            getcwd(buf,PATH_MAX);
            printf("%s>", buf);
            
            if(fgets(read,2048,stdin)==NULL){
                if(process_list!=NULL){
                    freeProcessList(*process_list);
                }
                for(int i=0; i<countCmd; i++){
                    free(queue[i]);
                }
                exit(0);
            }
            if(strcmp(read,"\n")==0){
                continue;
            }
            
            if(!isHistoryReapeatNumberCmd(read)){
                if(countCmd==HISTLEN){
                    free(queue[0]);
                    for(int i=0; i<HISTLEN-1; i++){
                        queue[i]=queue[i+1];
                    }
                    countCmd--;
                }
                queue[countCmd]=(char*)malloc(strlen(read)+1);
                strcpy(queue[countCmd],read);
                countCmd++;
            }
            
        }
        else{
            strcpy(read,queue[countCmd-1]);
            readInput=1;
        }

        cmdLine *cmdLine_parse=parseCmdLines(read);
        if(strcmp((*cmdLine_parse).arguments[0],"quit")==0){
            if(process_list!=NULL){
                freeProcessList(*process_list);
            }
            for(int i=0; i<countCmd; i++){
                free(queue[i]);
            }
            exit(0);
        }
        if(strcmp((*cmdLine_parse).arguments[0],"cd")==0){
            if((*cmdLine_parse).argCount==1){
                if(chdir(getenv("HOME"))==-1){
                    fputs("The cd operation failed\n",stderr);
                }
            }
            else if(chdir((*cmdLine_parse).arguments[1])==-1){
                fputs("The cd operation failed\n",stderr);
            }
        }
        else if(strcmp((*cmdLine_parse).arguments[0],"suspend")==0){
            if(kill((pid_t)atoi((*cmdLine_parse).arguments[1]),SIGTSTP)==-1){
                fputs("Suspention failed\n",stdout);
            }
            else{
                fputs("Suspention succeeded\n",stdout);
            }
        }
        else if(strcmp((*cmdLine_parse).arguments[0],"wake")==0){
            if(kill((pid_t)atoi((*cmdLine_parse).arguments[1]),SIGCONT)==-1){
                fputs("Wake failed\n",stdout);
            }
            else{
                fputs("Wake succeeded\n",stdout);
            }
        }
        else if(strcmp((*cmdLine_parse).arguments[0],"kill")==0){
            if(kill((pid_t)atoi((*cmdLine_parse).arguments[1]),SIGKILL)==-1){
                fputs("Termination failed\n",stdout);
            }
            else{
                fputs("Termination succeeded\n",stdout);
            }
        }
        else if(strcmp((*cmdLine_parse).arguments[0],"procs")==0){
            printProcessList(process_list);
        }
        else if(strcmp((*cmdLine_parse).arguments[0],"history")==0){
            for(int i=0; i<countCmd; i++){
                printf("%d %s",i, queue[i]);
            }
            freeCmdLines(cmdLine_parse);
        }
        else if(strcmp((*cmdLine_parse).arguments[0],"!!")==0){
            int cmdSpot=-1;
            for(int i=countCmd-1; i>=0; i--){
                if(strcmp(queue[i],"history\n")!=0){
                    cmdSpot=i;
                    break;
                }
            }
            if(cmdSpot!=-1){
                char* s=(char*)malloc(strlen(queue[cmdSpot])+1);
                strcpy(s,queue[cmdSpot]);
                if(countCmd==HISTLEN){
                    free(queue[0]);
                    for(int i=0; i<HISTLEN-1; i++){
                        queue[i]=queue[i+1];
                    }
                    countCmd--;
                }
                queue[countCmd]=s;
                countCmd++;
            }
            readInput=0;
            freeCmdLines(cmdLine_parse);
        }
        else if(isHistoryReapeatNumberCmd(read)){
            char *s=&(*cmdLine_parse).arguments[0][1];
            int num=atoi(s);
            if((num<=0)||(num>countCmd)){
                fprintf(stdout,"number out of range\n");
                continue;
            }
            if(countCmd==HISTLEN){
                free(queue[0]);
                for(int i=0; i<HISTLEN-1; i++){
                    queue[i]=queue[i+1];
                }
                countCmd--;
            }
            queue[countCmd]=(char*)malloc(strlen(queue[countCmd-num])+1);
            strcpy(queue[countCmd],queue[countCmd-num]);
            countCmd++;

            readInput=0;
            freeCmdLines(cmdLine_parse);
            
        }
        else{
            if((*cmdLine_parse).next!=NULL){
                pid_t pid=fork();
                if(pid==0){  
                    pipe_(cmdLine_parse);
                    perror("execv failed: child process is not created1");
                    _exit(1);
                }
                if(process_list==NULL){
                    process* newp=malloc(sizeof(process));
                    (*newp).next=NULL;
                    (*newp).cmd=cmdLine_parse;
                    (*newp).pid=pid;
                    (*newp).status=RUNNING;
                    process_list=malloc(sizeof(process*));
                    *process_list=newp;
                }
                else{
                    addProcess(process_list,cmdLine_parse,pid);
                }
                waitpid(pid,NULL,0);
                updateProcessStatus(*process_list,pid,TERMINATED);
            }
            else{
                pid_t pid=fork();
                if(pid==0){  
                    execute(cmdLine_parse);
                }
                if((*cmdLine_parse).blocking==1){
                    waitpid(pid,NULL,0);
                }
                if(process_list==NULL){
                    process* newp=malloc(sizeof(process));
                    (*newp).next=NULL;
                    (*newp).cmd=cmdLine_parse;
                    (*newp).pid=pid;
                    (*newp).status=RUNNING;
                    process_list=malloc(sizeof(process**));
                    *process_list=newp;
                }
                else{
                    addProcess(process_list,cmdLine_parse,pid);
                }
                if((*cmdLine_parse).blocking==1){
                    updateProcessStatus(*process_list,pid,TERMINATED);
                }
            }
        }
    }
}
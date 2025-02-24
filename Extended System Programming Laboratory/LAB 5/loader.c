#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <elf.h>
#include <string.h>

extern int startup(int argc, char **argv, void (*start)());

char* getType(int type){
    if(type == PT_NULL) return "NULL";
    if(type == PT_LOAD) return "LOAD";
    if(type == PT_DYNAMIC) return "DYNAMIC";
    if(type == PT_INTERP) return "INTERP";
    if(type == PT_NOTE) return "NOTE";
    if(type == PT_SHLIB) return "SHLIB";
    if(type == PT_PHDR) return "PHDR";
    if(type == PT_TLS) return "TLS";
    if(type == PT_NUM) return "NUM";
    if(type == PT_LOOS) return "LOOS";
    if(type == PT_GNU_EH_FRAME) return "GNU_EH_FRAME";
    if(type == PT_GNU_STACK) return "GNU_STACK";
    if(type == PT_GNU_RELRO) return "GNU_RELRO";
    if(type == PT_GNU_PROPERTY) return "GNU_PROPERTY";
    if(type == PT_LOSUNW) return "LOSUNW";
    if(type == PT_SUNWBSS) return "SUNWBSS";
    if(type == PT_SUNWSTACK) return "SUNWSTACK";
    if(type == PT_HISUNW) return "HISUNW";
    if(type == PT_HIOS) return "HIOS";
    if(type == PT_LOPROC) return "LOPROC";
    if(type == PT_HIPROC) return "HIPROC";
    return "";
}

void task0(Elf32_Phdr* phdr, int num){
    printf("Program header number %d at address %x\n", num, phdr->p_vaddr);
}

void task1a(Elf32_Phdr* phdr, int num){
    char read = ' ';
    char write = ' ';
    char execute = ' ';
    if(phdr->p_flags & PF_X){
        execute = 'E';
    }
    if(phdr->p_flags & PF_W){
        write = 'W';
    }
    if(phdr->p_flags & PF_R){
        read = 'R';
    }
    printf("%s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %c%c%c 0x%x\n", getType(phdr->p_type), phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz, read,write,execute, phdr->p_align);
}


void task1b(Elf32_Phdr* phdr, int num){
    printf("Program header address is %x\n", phdr->p_vaddr);
    printf("Protection flgs are: ", phdr->p_vaddr);
    if(phdr->p_flags & PF_X){
        printf("executable ");

    }
    printf(" | ");
    if(phdr->p_flags & PF_W){
        printf("writable ");
    }
    printf(" | ");
    if(phdr->p_flags & PF_R){
        printf("readable ");
    }

    printf("\nMapping falgs is MAP_PRIVATE | MAP_FIXED\n\n"); // beace it is private and fixed for all programs
}

void load_phdr(Elf32_Phdr *phdr, int fd){
    if(phdr->p_type == PT_LOAD){
        int protection = 0;
        int flags = MAP_PRIVATE | MAP_FIXED;
        if(phdr->p_flags & PF_X){
            protection = protection | PROT_EXEC;
        }
        if(phdr->p_flags & PF_W){
            protection = protection | PROT_WRITE;
        }
        if(phdr->p_flags & PF_R){
            protection = protection | PROT_READ;
        }
        char* vaddr = (char*)(phdr->p_vaddr&0xfffff000);
        size_t offset = phdr->p_offset&0xfffff000;
        int padding = phdr->p_vaddr & 0xfff;
        char* map = mmap(vaddr, phdr->p_memsz+padding, protection, flags, fd, offset);
        if (map == MAP_FAILED){
            perror("mmap");
        }
    }
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg){
    Elf32_Ehdr* header = (Elf32_Ehdr*) map_start;
    Elf32_Phdr* phdr = (Elf32_Phdr*) (map_start + header->e_phoff);
    for(int i = 0; i < header->e_phnum; i++){
        if(arg < 0)
            func(phdr + i, i);
        else 
            func(phdr + i, arg);
    }
    return 0;
}

int main(int argc, char** argv){
    char *addr;
    size_t length;
    struct stat  sb;
    if(argc < 2){
        printf("Wrong usage - %s fileName arg1 arg2 arg3...\n", argv[0]);
        return 1;
    }
    FILE* file = fopen(argv[1], "r");
    if(!file){
        perror("Fopen");
        return 1;
    }
    
    if (fstat(file->_fileno, &sb) == -1){
        perror("fstat");
        return 1;
    }
    length = sb.st_size;
    addr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, file->_fileno, 0);
    if (addr == MAP_FAILED){
        perror("mmap");
        return 1;
    }
    printf("task 0: \n");
    foreach_phdr(addr, task0, -1);
    printf("\n");
    printf("task 1a: \n");
    printf("Type Offset VirtAddr PhysAddr FileSiz MemSiz Flg Align\n");
    foreach_phdr(addr, task1a, -1);
    printf("\n");
    printf("task 1b: \n");
    foreach_phdr(addr, task1b, -1);
    printf("\n");
    printf("task 2: \n");
    foreach_phdr(addr, load_phdr, file->_fileno);
    startup(argc - 1, argv + 1, ((Elf32_Ehdr*) addr)->e_entry);
    munmap(addr, length);
    fclose(file);
    return 0;
}
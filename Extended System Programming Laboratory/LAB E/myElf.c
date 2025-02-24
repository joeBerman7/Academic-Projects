#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>
#include <unistd.h>
#define MAX_INPUT_LENGTH 100
typedef struct {
    char debug_mode;
    char file_name1[100];
    char file_name2[100];
    char* mapped_file1;
    char* mapped_file2;
    int file_descriptor1;
    int file_descriptor2;

} state;

typedef struct func_pair {
    char* name;
    void (*fun)();
} func_pair;

void Toggle_Debug_Mode(state* s){
    if(s->debug_mode) {
        s->debug_mode = 0;
        printf("Debug mode is now OFF\n");
    } else {
        s->debug_mode = 1;
        printf("Debug mode is now ON\n");
    }

}

void Examine_ELF_File(state* s){
    char *addr;
    size_t length;
    struct stat  sb;
    printf("Enter file name\n");
    char fileName[100];
    fgets(fileName, 100, stdin);
    fileName[strlen(fileName) - 1] = '\0';
    FILE* file = fopen(fileName, "rb");
    if(!file){
        perror("Fopen");
        return;
    }
    
    if (fstat(file->_fileno, &sb) == -1){
        perror("fstat");
        return;
    }
    length = sb.st_size;
    addr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, file->_fileno, 0);
    if (addr == MAP_FAILED){
        perror("mmap");
        return;
    }
    Elf32_Ehdr* header = (Elf32_Ehdr*)addr;
    if (header->e_ident[0] != 0x7f || header->e_ident[1] != 'E' || header->e_ident[2] != 'L' || header->e_ident[3] != 'F'){
        printf("Not an ELF file\n");
        return;
    }
    printf("Magic: %X %X %X %X\n", header->e_ident[0], header->e_ident[1], header->e_ident[2], header->e_ident[3]);
    if (header->e_ident[5] == 1){
        printf("Data: 2's complement, little endian\n");
    }
    else if (header->e_ident[5] == 2){
        printf("Data: 2's complement, big endian\n");
    }
    else{
        printf("Data: Invalid data encoding\n");
    }
    printf("Entry point: %X\n", header->e_entry);
    printf("Section header table offset: %X\n", header->e_shoff);
    printf("Number of section headers: %d\n", header->e_shnum);
    printf("Size of each section header: %d\n", header->e_shentsize);
    printf("Program header table offset: %X\n", header->e_phoff);
    printf("Number of program headers: %d\n", header->e_phnum);
    printf("Size of each program header: %d\n", header->e_phentsize);
    //save the file in the state
    if(s->file_descriptor1 == 0){
        strcpy(s->file_name1, fileName);
        s->mapped_file1 = addr;
        s->file_descriptor1 = file->_fileno;
    } else if(s->file_descriptor2 == 0){
        strcpy(s->file_name2, fileName);
        s->mapped_file2 = addr;
        s->file_descriptor2 = file->_fileno;
    } else {
        printf("Both files are already loaded, this file won't be saved and will be closed.\n");
        munmap(addr, length);
        fclose(file);
    }
}

char* typeToString(int type){
    switch (type){
        case SHT_NULL:
            return "NULL";
        case SHT_PROGBITS:
            return "PROGBITS";
        case SHT_SYMTAB:
            return "SYMTAB";
        case SHT_STRTAB:
            return "STRTAB";
        case SHT_RELA:
            return "RELA";
        case SHT_HASH:
            return "HASH";
        case SHT_DYNAMIC:
            return "DYNAMIC";
        case SHT_NOTE:
            return "NOTE";
        case SHT_NOBITS:
            return "NOBITS";
        case SHT_REL:
            return "REL";
        case SHT_SHLIB:
            return "SHLIB";
        case SHT_DYNSYM:
            return "DYNSYM";
        case SHT_INIT_ARRAY:
            return "INIT_ARRAY";
        case SHT_FINI_ARRAY:
            return "FINI_ARRAY";
        case SHT_PREINIT_ARRAY:
            return "PREINIT_ARRAY";
        case SHT_GROUP:
            return "GROUP";
        case SHT_SYMTAB_SHNDX:
            return "SYMTAB_SHNDX";
        default:
            return "Unknown";
    }
}

void Print_Section_Names(state* s){
    if(s->file_descriptor1 == 0){
        printf("No file loaded\n");
        return;
    }
    Elf32_Ehdr* header = (Elf32_Ehdr*)s->mapped_file1;
    Elf32_Shdr* section_headers = (Elf32_Shdr*)(s->mapped_file1 + header->e_shoff);
    char* section_names = (char*)(s->mapped_file1 + section_headers[header->e_shstrndx].sh_offset);
    printf("File %s\n", s->file_name1);
    printf("[index] section_name section_address section_offset section_size section_type\n");
    for (int i = 0; i < header->e_shnum; i++){
        printf("[%d] %s %X %X %X %s\n", i, section_names + section_headers[i].sh_name, section_headers[i].sh_addr, section_headers[i].sh_offset, section_headers[i].sh_size, typeToString(section_headers[i].sh_type)); 
    }
    
    if(s->file_descriptor2 != 0){
        header = (Elf32_Ehdr*)s->mapped_file2;
        section_headers = (Elf32_Shdr*)(s->mapped_file2 + header->e_shoff);
        section_names = (char*)(s->mapped_file2 + section_headers[header->e_shstrndx].sh_offset);
        printf("File %s\n", s->file_name2);
        printf("[index] section_name section_address section_offset section_size section_type\n");
        for (int i = 0; i < header->e_shnum; i++){
            printf("[%d] %s %X %X %X %s\n", i, section_names + section_headers[i].sh_name, section_headers[i].sh_addr, section_headers[i].sh_offset, section_headers[i].sh_size, typeToString(section_headers[i].sh_type)); 
        }
    }
}

void print_symbols_help(char* fileName, char* mapped_file){
    Elf32_Ehdr* header = (Elf32_Ehdr*)mapped_file;
    Elf32_Shdr* section_headers = (Elf32_Shdr*)(mapped_file + header->e_shoff);
    char* section_names = (char*)(mapped_file + section_headers[header->e_shstrndx].sh_offset);
    for(int i = 0; i < header->e_shnum; i++){
        if(section_headers[i].sh_type == SHT_SYMTAB){
            Elf32_Sym* symbols = (Elf32_Sym*)(mapped_file + section_headers[i].sh_offset);
            char* strtab = (char*)(mapped_file + section_headers[section_headers[i].sh_link].sh_offset);
            printf("File %s\n", fileName);
            printf("[index] value section_index section_name symbol_name\n");
            for(int j = 0; j < section_headers[i].sh_size / section_headers[i].sh_entsize; j++){
                //check if index is ABS or UNDEF or else
                if(symbols[j].st_shndx == SHN_ABS){
                    printf("[%d] %X %d ABS %s\n", j, symbols[j].st_value, symbols[j].st_shndx, strtab + symbols[j].st_name);
                } else if(symbols[j].st_shndx == SHN_UNDEF){
                    printf("[%d] %X %d UND %s\n", j, symbols[j].st_value, symbols[j].st_shndx, strtab + symbols[j].st_name);
                } else {
                    printf("[%d] %X %d %s %s\n", j, symbols[j].st_value, symbols[j].st_shndx, section_names + section_headers[symbols[j].st_shndx].sh_name, strtab + symbols[j].st_name);
                }
            }
        } else if(section_headers[i].sh_type == SHT_DYNSYM){
            Elf32_Sym* symbols = (Elf32_Sym*)(mapped_file + section_headers[i].sh_offset);
            char* strtab = (char*)(mapped_file + section_headers[section_headers[i].sh_link].sh_offset);
            printf("File %s\n", fileName);
            printf("[index] value section_index section_name symbol_name\n");
            for(int j = 0; j < section_headers[i].sh_size / section_headers[i].sh_entsize; j++){
                //check if index is ABS or UNDEF or else
                if(symbols[j].st_shndx == SHN_ABS){
                    printf("[%d] %X %d ABS %s\n", j, symbols[j].st_value, symbols[j].st_shndx, strtab + symbols[j].st_name);
                } else if(symbols[j].st_shndx == SHN_UNDEF){
                    printf("[%d] %X %d UND %s\n", j, symbols[j].st_value, symbols[j].st_shndx, strtab + symbols[j].st_name);
                } else {
                    printf("[%d] %X %d %s %s\n", j, symbols[j].st_value, symbols[j].st_shndx, section_names + section_headers[symbols[j].st_shndx].sh_name, strtab + symbols[j].st_name);
                }
            }
        }
    }
}

void Print_Symbols(state* s){
    //File ELF-file0name
    //[index] value section_index section_name symbol_name
    if(s->file_descriptor1 == 0){
        printf("No file loaded\n");
        return;
    }
    print_symbols_help(s->file_name1, s->mapped_file1);
    if(s->file_descriptor2 != 0){
        print_symbols_help(s->file_name2, s->mapped_file2);
    }
}

Elf32_Sym* get_symbols(char* mapped_file, Elf32_Ehdr* header, char* symbol_name) {
    if(strcmp(symbol_name, "") == 0){
        return NULL;
    }
    Elf32_Shdr* section_headers = (Elf32_Shdr*)(mapped_file + header->e_shoff);
    for(int i = 1; i < header->e_shnum; i++){
        if(section_headers[i].sh_type == SHT_SYMTAB){
            Elf32_Sym* symbols = (Elf32_Sym*)(mapped_file + section_headers[i].sh_offset);
            char* strtab = (char*)(mapped_file + section_headers[section_headers[i].sh_link].sh_offset);
            for(int j = 1; j < section_headers[i].sh_size / section_headers[i].sh_entsize; j++){
                if(strcmp(strtab + symbols[j].st_name, symbol_name) == 0){
                    return symbols + j;
                }
            }
        }
    }
    return NULL;
}

void check_defined(char* mapped_file1, char* mapped_file2){
    Elf32_Ehdr* header = (Elf32_Ehdr*)mapped_file1;
    Elf32_Shdr* section_headers = (Elf32_Shdr*)(mapped_file1 + header->e_shoff);
    for(int i = 0; i < header->e_shnum; i++){
        if(section_headers[i].sh_type == SHT_SYMTAB){
            Elf32_Sym* symbols = (Elf32_Sym*)(mapped_file1 + section_headers[i].sh_offset);
            char* strtab = (char*)(mapped_file1 + section_headers[section_headers[i].sh_link].sh_offset);
            for(int j = 1; j < section_headers[i].sh_size / section_headers[i].sh_entsize; j++){
                if(symbols[j].st_shndx == SHN_UNDEF){
                    Elf32_Sym* symbol = get_symbols(mapped_file2, (Elf32_Ehdr*)mapped_file2, strtab + symbols[j].st_name);
                    if(symbol == NULL || symbol->st_shndx == SHN_UNDEF){
                        printf("Symbol %s is undefined in both files\n", strtab + symbols[j].st_name);
                    }
                } else {
                    Elf32_Sym* symbol = get_symbols(mapped_file2, (Elf32_Ehdr*)mapped_file2, strtab + symbols[j].st_name);
                    if(symbol != NULL && symbol->st_shndx != SHN_UNDEF){
                        printf("Symbol %s is defined in both files\n", strtab + symbols[j].st_name);
                    }
                }
            }
        }
    }
}

void Check_Files_for_Merge(state* s){
    
    //for each symbol in file 1, check if it's defined in both files, or undefined in both or undefined in first and doesn't exist in the other. if so, print an error message
    if(s->file_descriptor1 == 0 || s->file_descriptor2 == 0){
        printf("You need exactly two files\n");
        return;
    }
    check_defined(s->mapped_file1, s->mapped_file2);
    check_defined(s->mapped_file2, s->mapped_file1);
}
Elf32_Shdr* getSection(char* mapped_file, Elf32_Ehdr* header, char* section_name){
    Elf32_Shdr* section_headers = (Elf32_Shdr*)(mapped_file + header->e_shoff);
    char* section_names = (char*)(mapped_file + section_headers[header->e_shstrndx].sh_offset);
    for(int i = 0; i < header->e_shnum; i++){
        if(strcmp(section_names + section_headers[i].sh_name, section_name) == 0){
            return section_headers + i;
        }
    }
    return NULL;
}

void Merge_ELF_Files(state* s){
    if(s->file_descriptor1 == 0 || s->file_descriptor2 == 0){
        printf("You need exactly two files\n");
        return;
    }
    FILE* out = fopen("out.ro", "wb");
    if(!out){
        perror("Fopen");
        return;
    }
    Elf32_Ehdr* header = (Elf32_Ehdr*)s->mapped_file1;
    fwrite(header, sizeof(char), header->e_ehsize, out);
    Elf32_Shdr* section_headers = (Elf32_Shdr*)(s->mapped_file1 + header->e_shoff);
    Elf32_Shdr* section_headers_cpy = (Elf32_Shdr*)malloc(header->e_shentsize * header->e_shnum);
    memcpy(section_headers_cpy, section_headers, header->e_shentsize * header->e_shnum);

    char* section_names = (char*)(s->mapped_file1 + section_headers[header->e_shstrndx].sh_offset);
    for(int i = 0; i < header->e_shnum; i++){
        if(i != 0) section_headers_cpy[i].sh_offset = ftell(out);
        if(strcmp(section_names + section_headers[i].sh_name, ".text") == 0 || strcmp(section_names + section_headers[i].sh_name, ".data") == 0 || strcmp(section_names + section_headers[i].sh_name, ".rodata") == 0){
            fwrite(s->mapped_file1 + section_headers[i].sh_offset, sizeof(char), section_headers[i].sh_size, out);
            Elf32_Shdr* section2 = getSection(s->mapped_file2, (Elf32_Ehdr*)s->mapped_file2, section_names + section_headers[i].sh_name);
            if(section2 != NULL){
                fwrite(s->mapped_file2 + section2->sh_offset, sizeof(char), section2->sh_size, out);
                section_headers_cpy[i].sh_size += section2->sh_size;
            }
        } else {
            fwrite(s->mapped_file1 + section_headers[i].sh_offset, sizeof(char), section_headers[i].sh_size, out);
        }
    }
    int e_shoff = ftell(out);
    fwrite(section_headers_cpy, sizeof(char), header->e_shentsize * header->e_shnum, out);
    //update e_shoff in the file out 
    fseek(out, 0x20, SEEK_SET);
    fwrite(&e_shoff, sizeof(int), 1, out);
    fclose(out);
}

void Quit(state* s){
    if(s->file_descriptor1 != 0){
        munmap(s->mapped_file1, 0);
        close(s->file_descriptor1);
    }
    if(s->file_descriptor2 != 0){
        munmap(s->mapped_file2, 0);
        close(s->file_descriptor2);
    }
    exit(0);
}

func_pair menu[] = {
    {"Toggle Debug Mode", Toggle_Debug_Mode},
    {"Examine ELF File", Examine_ELF_File},
    {"Print Section Names", Print_Section_Names},
    {"Print Symbols", Print_Symbols},
    {"Check Files for Merge", Check_Files_for_Merge},
    {"Merge ELF Files", Merge_ELF_Files},
    {"Quit", Quit},
    { NULL, NULL }
};

int main() {
    state* s = (state*)malloc(sizeof(state));   
    s->debug_mode = 0;
    int menu_size = 0;
    char input[MAX_INPUT_LENGTH];
    while (menu[menu_size].name != NULL) {
        menu_size++;
    }
    
    while (1) {
        fprintf(stdout, "Select operation from the following menu:\n");
        for (int i = 0; i < menu_size; i++) {
            fprintf(stdout, "%d) %s\n", i, menu[i].name);
        }
        fprintf(stdout, "Option: ");
        
        
        // Use sscanf to read the option from stdin
        if(fgets(input, sizeof(input), stdin) == NULL) {
            Quit(s);
        }   
        int option;
        sscanf(input, "%d", &option);
        if (option < 0 || option >= menu_size) {
            fprintf(stdout, "Not within bounds. Please select a valid option.\n");
            continue;
        }

        menu[option].fun(s); // Call the function associated with the option
    }
}
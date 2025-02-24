#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_LENGTH 100
typedef struct {
    char debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    char display_mode;
} state;

typedef struct func_pair {
    char* name;
    void (*fun)();
} func_pair;

static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};

void Toggle_Debug_Mode(state* s){
    if(s->debug_mode) {
        s->debug_mode = 0;
        printf("Debug mode is now OFF\n");
    } else {
        s->debug_mode = 1;
        printf("Debug mode is now ON\n");
    }
}

void Set_File_Name(state* s){
    printf("Enter file name: ");
    fgets(s->file_name, sizeof(s->file_name), stdin);
    s->file_name[strcspn(s->file_name, "\n")] = 0; // Remove newline character
    if(s->debug_mode) {
        fprintf(stderr, "File name set to: %s\n", s->file_name);
    }
}

void Set_Unit_Size(state* s){
    printf("Enter unit size (1, 2, or 4): ");
    char input[MAX_INPUT_LENGTH];
    fgets(input, sizeof(input), stdin);
    int size;
    sscanf(input, "%d", &size);
    if(size == 1 || size == 2 || size == 4) {
        s->unit_size = size;
    } else {
        printf("Invalid unit size. Please enter 1, 2, or 4.\n");
        return;
    }
    if(s->debug_mode) {
        fprintf(stderr, "Unit size set to: %d\n", s->unit_size);
    }
}

void Load_Into_Memory(state* s){
    /*
    Check if file_name is empty (i.e. equals to ""), and if it is print an error message and return.
    • Open file_name for reading. If this fails, print an error message and return.
    • Prompt the user for location in hexadecimal, and length (in decimal).
    • If debug flag is on, print the file_name, as well as location, and length.
    • Copy length * unit_size bytes from file_name starting at position location into mem_buf.
    • Close the file.
    */
    if(strlen(s->file_name) == 0) {
        printf("Error: File name is not set.\n");
        return;
    }
    FILE* file = fopen(s->file_name, "rb");
    if(file == NULL) {
        printf("Error: Could not open file %s.\n", s->file_name);
        return;
    }
    printf("Please enter <location> <length>");
    int location, length;
    char input[MAX_INPUT_LENGTH];
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %d", &location, &length);
    if(s->debug_mode) {
        fprintf(stderr, "File name: %s, Location: %x, Length: %d\n", s->file_name, location, length);
    }
    fseek(file, location, SEEK_SET);
    int nread = fread(s->mem_buf, s->unit_size, length, file);
    s->mem_count = nread;
    fclose(file);
}

void Toggle_Display_Mode(state* s){
    if(s->display_mode == 0) {
        s->display_mode = 1;
        printf("Display flag now on, hexadecimal representation\n");
    } else {
        s->display_mode = 0;
        printf("Display flag now off, decimal representation\n");
    }
}

void File_Display(state* s){
    if(s->file_name[0] == '\0') {
        printf("Error: File name is not set.\n");
        return;
    }
    FILE* file = fopen(s->file_name, "rb");
    if(file == NULL) {
        printf("Error: Could not open file %s.\n", s->file_name);
        return;
    }
    printf("Enter file offset and length\n");
    int offset, length;
    char input[MAX_INPUT_LENGTH];
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %d", &offset, &length);
    fseek(file, offset, SEEK_SET);
    char buffer[length * s->unit_size];
    fread(buffer, s->unit_size, length, file);
    fclose(file);
    if(s->display_mode) {
        for(int i = 0; i < length; i++) {
            printf(hex_formats[s->unit_size - 1], *((unsigned int*)(buffer + (i * s->unit_size))));
        }
    } else {
        for(int i = 0; i < length * s->unit_size; i++) {
            printf(dec_formats[s->unit_size - 1], *((unsigned int*)(buffer + (i * s->unit_size))));
        }
    }
}

void Memory_Display(state* s){
    printf("Enter memory address and length\n");
    int address, length;
    char input[MAX_INPUT_LENGTH];
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %d", &address, &length);
    void* addr = (void*)address;
    if(address == 0){
        addr = s->mem_buf;
    }
    if(s->display_mode){
        for(int i = 0; i < length; i++) {
            printf(hex_formats[s->unit_size - 1], *(unsigned int*)(addr + (i * s->unit_size)));
        }
    } else {
        for(int i = 0; i < length; i++) {
            printf(dec_formats[s->unit_size - 1], *(unsigned int*)(addr + (i * s->unit_size)));
        }
    }
}

void Save_Into_File(state* s){
    if(s->file_name[0] == '\0') {
        printf("Error: File name is not set.\n");
        return;
    }
    FILE* file = fopen(s->file_name, "rb+");
    if(file == NULL) {
        printf("Error: Could not open file %s.\n", s->file_name);
        return;
    }
    printf("Enter source target and length\n");
    int source, target, length;
    char input[MAX_INPUT_LENGTH];
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %x %d", &source, &target, &length);
    fseek(file, target, SEEK_SET);
    void* addr = (void*)source;
    if(source == 0) {
        addr = s->mem_buf;
    }
    fwrite(addr, s->unit_size, length, file);
    fclose(file);
}

void Memory_Modify(state* s){
    printf("Enter memory address and new value\n");
    int address;
    unsigned int value;
    char input[MAX_INPUT_LENGTH];
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %x", &address, &value);
    if(s->unit_size == 1){
        s->mem_buf[address] = value;
    } else if(s->unit_size == 2){
        ((unsigned short*)s->mem_buf)[address] = value;
    } else if(s->unit_size == 4){
        ((unsigned int*)s->mem_buf)[address] = value;
    }
}

void Quit(state* s){
    free(s);
    exit(0);
}

func_pair menu[] = {
    { "Toggle Debug Mode", Toggle_Debug_Mode },
    { "Set File Name", Set_File_Name },
    { "Set Unit Size", Set_Unit_Size },
    { "Load Into Memory", Load_Into_Memory },
    { "Toggle Display Mode", Toggle_Display_Mode },
    { "File Display", File_Display },
    { "Memory Display", Memory_Display },
    { "Save Into File", Save_Into_File },
    { "Memory Modify", Memory_Modify },
    { "Quit", Quit },
    { NULL, NULL }
};

int main() {
    state* s = (state*)malloc(sizeof(state));   
    s->debug_mode = 0;
    s->unit_size = 1;
    s->mem_count = 0;
    s->file_name[0] = '\0'; // Initialize file name to empty string
    s->mem_buf[0] = '\0'; // Initialize memory buffer to zero
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




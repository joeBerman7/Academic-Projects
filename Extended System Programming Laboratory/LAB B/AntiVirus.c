#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link {
    virus *vir;
    struct link *nextVirus;
    int size;  // help us to debug
} link;

///////////////// our global vars /////////////////
#define MAX_INPUT_LENGTH 30
#define MAX_BUFFER_SIZE 10000  
static link* virus_list;
static short prev_input = 0;
static char suspected_file_data[MAX_BUFFER_SIZE];
static int suspected_file_size;
char signatures_file_name[30];
///////////////////////////////////////////////////


////////////// viruses methods ///////////////////

void printVirus(virus* virus, FILE* output) {
    int i;
    fprintf(output, "Virus name: %s\nsignature length: %d\nsignature:\n",
        virus->virusName,
        virus->SigSize);
    for (i = 0; i < virus->SigSize - 1; i++)
        fprintf(output, "%02X ", virus->sig[i]);
    fprintf(output, "%x\n\n", virus->sig[i]);
}

virus* readVirus(FILE* file, char endian_mode) {
    unsigned char sizeAndName[18];
    if (fread(sizeAndName, sizeof(char), 18, file) != 18) {
        return NULL;
    }
    virus* parsed = (virus*)malloc(sizeof(virus));
    parsed->SigSize = *((unsigned short*)sizeAndName);
    if (endian_mode == 'B') {
        parsed->SigSize = __builtin_bswap16(parsed->SigSize);
    }
    memcpy(parsed->virusName, sizeAndName + 2, 16 * sizeof(char));
    parsed->sig = calloc(parsed->SigSize, sizeof(char));
    fread(parsed->sig, sizeof(char), parsed->SigSize, file);
    return parsed;
}

//////////////////////////////////////////////////

/////////// linked list methods //////////////////

link* list_append(link* virus_list, virus* data) {
    link* new_list = (link*)malloc(sizeof(link));
    if (new_list == NULL) {
        printf("Memory allocation failed\n");
        return NULL; 
    }

    new_list->vir = data;
    new_list->nextVirus = virus_list;

    if (virus_list == NULL) {
        new_list->size = 1;
    } else {
        new_list->size = virus_list->size + 1;
    }

    return new_list;
}

void list_free(link* virus_list) {
    link *curr = virus_list, *next;
    while (curr != NULL) {
        next = curr->nextVirus;
        free(curr->vir->sig);
        free(curr->vir);
        free(curr);
        curr = next;
    }
}

void list_print(link* virus_list, FILE* output) {
    link* curr = virus_list;
    fprintf(output, "Virus list size: %d\n", virus_list ? virus_list->size : 0);
    while (curr != NULL) {
        printVirus(curr->vir, output);
        curr = curr->nextVirus;
    }
}

//////////////////////////////////////////////////

///////////////// AntiVirus program //////////////


void quit() { 
    list_free(virus_list); 
    exit(0); 
}

// Helper function to clear the input buffer after reading user input
void clear_input_buffer() {
    while (getchar() != '\n'); // Clear the buffer until newline
}

void load_signatures() {
    printf("Enter the signature file name: ");
    if (fgets(signatures_file_name, sizeof(signatures_file_name), stdin) == NULL) {
        printf("Error reading file name.\n");
        return;
    }
    signatures_file_name[strcspn(signatures_file_name, "\n")] = '\0';
    FILE* file = fopen(signatures_file_name, "r");
    if (file == NULL) {
        printf("Couldn't open file %s\n", signatures_file_name);
        return;
    }
    char magic[5];
    fread(magic, sizeof(char), 4, file);
    magic[4] = '\0';
    if (strcmp(magic, "VIRL") != 0 && strcmp(magic, "VIRB") != 0) {
        fprintf(stderr, "Magic number is incorrect, it is: %s\n", magic);
        fclose(file);
        return;
    }
    char endian_mode = magic[3];
    virus* v;
    while ((v = readVirus(file, endian_mode)) != NULL) {
        virus_list = list_append(virus_list, v);
    }
    fclose(file);
}

void print_signatures() {
    FILE *output_file = fopen("virus_signatures.txt", "w");
    if (output_file == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }
    list_print(virus_list, output_file);
    fclose(output_file);
    printf("Virus signatures have been saved to 'virus_signatures.txt'.\n");
}

void detect_virus(char *buffer, unsigned int size, link *virus_list) {
    for (int i = 0; i < size; i++) {
        link* curr = virus_list;
        while (curr != NULL) {
            if (curr->vir->SigSize <= size - i) {
                int res = memcmp(buffer+i, curr->vir->sig, curr->vir->SigSize);
                if (res == 0) {
                    printf("--Virus detected--\n\tstart:\t%d hex:(%X) \n\tname:\t%s\n\tsize:%d\n",
                        i, i, curr->vir->virusName, curr->vir->SigSize);
                }
            }
            curr = curr->nextVirus;
        }
    }
}

void detect_viruses() {
    char suspected_file_name[MAX_INPUT_LENGTH];
    printf("Suspected file name to be scanned: ");
    if (fgets(suspected_file_name, sizeof(suspected_file_name), stdin) == NULL) {
        printf("Error reading file name.\n");
        return;
    }
    suspected_file_name[strcspn(suspected_file_name, "\n")] = '\0';
    FILE *file = fopen(suspected_file_name, "rb");
    if (file == NULL) {
        printf("Couldn't open file %s\n", suspected_file_name);
        return;
    }
    fseek(file, 0, SEEK_END);
    suspected_file_size = ftell(file);
    printf("the size of infected: %d\n",suspected_file_size);
    fseek(file, 0, SEEK_SET); // Move the file pointer back to the start
    fread(suspected_file_data, sizeof(char), suspected_file_size, file);
    detect_virus(suspected_file_data, suspected_file_size, virus_list);
    fclose(file);
}

void neutralize_virus(char *fileName, int signatureOffset) {
    FILE *file = fopen(fileName, "r+");
    if (file == NULL) {
        printf("Error opening the file %s\n", fileName);
        return;
    }
    fseek(file, signatureOffset, SEEK_SET);
    fwrite("\xC3", sizeof(char), 1, file); // Write the RET instruction (0xC3) to neutralize the virus (modifies only the first byte)
    fclose(file);
}

void fix_file() {
    char suspected_file_name[MAX_INPUT_LENGTH];
    printf("Suspected file name to be scanned: ");
    if (fgets(suspected_file_name, sizeof(suspected_file_name), stdin) == NULL) {
        printf("Error reading file name.\n");
        return;
    }
    suspected_file_name[strcspn(suspected_file_name, "\n")] = '\0';
    FILE *file = fopen(suspected_file_name, "r+");
    if (file == NULL) {
        printf("Couldn't open file %s\n", suspected_file_name);
        exit(0);
    }
    fread(suspected_file_data, sizeof(char), MAX_BUFFER_SIZE, file);
    suspected_file_size = ftell(file);

    // Detect and neutralize viruses
    for (int i = 0; i < suspected_file_size; i++) {
        link* curr = virus_list;
        while (curr != NULL) {
            // If the virus signature matches at this position
            if (curr->vir->SigSize <= suspected_file_size - i &&
                memcmp(suspected_file_data + i, curr->vir->sig, curr->vir->SigSize) == 0) {
                // Call the neutralize function to overwrite the virus signature in the file
                neutralize_virus(suspected_file_name, i);  // Neutralize virus at offset i
            }
            curr = curr->nextVirus;
        }
    }
    fclose(file);
}

//////////////////////////////////////////////////

// main program 
typedef struct func_pair {
    char* name;
    void (*fun)();
} func_pair;

func_pair menu[] = {
    { "Load signatures", &load_signatures },
    { "Print signatures", &print_signatures },
    { "Detect viruses", &detect_viruses },
    { "Fix file", &fix_file },
    { "Quit", &quit },
    { NULL, NULL }
};

int main() {

    int menu_size = 0;
    char input[MAX_INPUT_LENGTH];
    while (menu[menu_size].name != NULL) {
        menu_size++;
    }
    
    while (1) {
        fprintf(stdout, "Select operation from the following menu:\n");
        for (int i = 0; i < menu_size; i++) {
            fprintf(stdout, "%d) %s\n", i + 1, menu[i].name);
        }
        fprintf(stdout, "Option: ");
        
        // Reset the input buffer for each new input
        if (prev_input) {
            prev_input = 0;  // Reset flag
        }

        // Use sscanf to read the option from stdin
        if (scanf("%29s", input) != 1) {
            printf("Error reading input. Exiting.\n");
            quit();
        }

        int option = atoi(input) - 1;
        if (option < 0 || option >= menu_size) {
            fprintf(stdout, "Not within bounds. Please select a valid option.\n");
            continue;
        }

        clear_input_buffer(); // Clear the input buffer before asking for the file name in the next step        
        (*(menu[option].fun))(); // Call the function associated with the option
    }
}




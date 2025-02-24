#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int debug_mode = 1;  // Debug Default is on
char *encoding_key = "0";  // Default encoding key is 0
int encode_addition = 1;  // encoding default is addition
FILE *infile = NULL;  
FILE *outfile = NULL;  

char encode(char c) {
    
    static int key_index = 0;  // To track position in the encoding key
    // encoding alphanumeric characters (a-z, A-Z, 0-9)
    if (isdigit(c)) {
        int shift = encoding_key[key_index] - '0'; // Get the numeric value from the encoding key
        if (!encode_addition) {
            shift = -shift; // If subtraction, invert the shift
        }
        c = (c - '0' + shift + 10) % 10 + '0'; // Apply the shift with wrap-around
    } else if (isupper(c)) {
        int shift = encoding_key[key_index] - '0';
        if (!encode_addition) {
            shift = -shift;
        }
        c = (c - 'A' + shift + 26) % 26 + 'A';
    } else if (islower(c)) {
        int shift = encoding_key[key_index] - '0';
        if (!encode_addition) {
            shift = -shift;
        }
        c = (c - 'a' + shift + 26) % 26 + 'a';
    }

    // Update key index for the next character (cyclic)
    key_index = (key_index + 1) % strlen(encoding_key);
    return c;
}

int main(int argc, char *argv[]) {
    
    if (infile == NULL) infile = stdin;
    if (outfile == NULL) outfile = stdout;

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "+E", 2) == 0) {
            encoding_key = argv[i] + 2; 
            encode_addition = 1;  // Encoder Addition mode
        } else if (strncmp(argv[i], "-E", 2) == 0) {
            encoding_key = argv[i] + 2;
            encode_addition = 0;  // Encoder Subtraction mode
        } else if (strcmp(argv[i], "-D") == 0) {
            debug_mode = 0;
        } else if (strcmp(argv[i], "+D") == 0) {
            debug_mode = 1;
        } else if (strncmp(argv[i], "-i", 2) == 0) {
            infile = fopen(argv[i] + 2, "r");
            if (infile == NULL) {
                fprintf(stderr, "Error: Could not open input file %s\n", argv[i] + 2);
                exit(1);  // Exit if the file cannot be opened
            }
        } else if (strncmp(argv[i], "-o", 2) == 0) {
            outfile = fopen(argv[i] + 2, "w"); 
            if (outfile == NULL) {
                fprintf(stderr, "Error: Could not open output file %s\n", argv[i] + 2);
                exit(1);  // Exit if the file cannot be opened
            }
        }

        if (debug_mode) {
            fprintf(stderr, "Arg %d: %s\n", i, argv[i]);
        }
    }

    // Processing input character by character from infile and writing to outfile
    char c;
    while ((c = fgetc(infile)) != EOF) {

        char debug_c = c; // for deubgging
        c = encode(c);
        fputc(c, outfile);

        // Debugging: c = encode(c)
        if (debug_mode) {
            fprintf(stderr, "Encoding '%c' to '%c'\n", debug_c, c);
        }
    }

    // Close the files 
    if (infile != stdin) {
        fclose(infile);
    }
    if (outfile != stdout) {
        fclose(outfile);
    }

    return 0;
}

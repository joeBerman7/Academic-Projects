#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct fun_desc {
  char *name;
  char (*fun)(char);
};

char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  /* task 2.a */
  for(int i = 0; i < array_length; i++){
    mapped_array[i] = f(array[i]);
  }
  free(array); 
  return mapped_array;
}
 
/* Ignores c, reads and returns a character from stdin using fgetc. */
char my_get(char c){
  return fgetc(stdin);
}

/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */
char cprt(char c){
  if(c >= 0x20 && c <= 0x7E){
    printf("%c\n", c);
  } else {
    printf(".\n");
  }
  return c;
}

/* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x1F and 0x7E it is returned unchanged */
char encrypt(char c){
  if(c >= 0x1F && c <= 0x7E){
    c++;
  }
  return c;
}

/* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x21 and 0x7F it is returned unchanged */
char decrypt(char c){
  if(c >= 0x21 && c <= 0x7F){
    c--;
  }
  return c;
}

/* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */
char xprt(char c){
  printf("%x\n", c);
  return c;
}

/* dprt prints the value of c in a decimal representation followed by a new line, and returns c unchanged. */
char dprt(char c){
  printf("%d\n", c);
  return c;
}

struct fun_desc menu[] = { { "Get String", my_get }, 
                           { "Print Decimal", dprt },
                           { "Print Hex", xprt }, 
                           { "Print Character", cprt },
                           { "Encrypt", encrypt }, 
                           { "Decrypt", decrypt }, 
                           { NULL, NULL }};

int main(int argc, char **argv){
  char* carray = calloc(5, sizeof(char));
  int menuSize = sizeof(menu) / sizeof(struct fun_desc) - 1;
  char input[100];
  while(1){
    printf("Select operation from the following menu:\n");
    for(int i = 0; i < menuSize; i++){
      printf("%d) %s\n", i, menu[i].name);
    }
    printf("Option: ");
    if(fgets(input, 100, stdin) == NULL) break;
    int option = atoi(input);
    if(option < 0 || option >= menuSize){
      printf("Not within bounds\n");
      break;
    }
    printf("Within bounds\n");
    carray = map(carray, 5, menu[option].fun);
    // printf("the carray is %s\n",carray); // print for me to check

  }
  free(carray);
  return 0;
}


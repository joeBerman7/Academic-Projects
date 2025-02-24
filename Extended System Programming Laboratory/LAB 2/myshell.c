#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include "LineParser.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

int debug_mode = 0;
void debug_info(int pid, cmdLine *pCmdLine) {
    fprintf(stderr, "PID: %d\n", pid);
    fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
}

// signals
void handle_signal_cmd(cmdLine *pCmdLine) {
    if (pCmdLine->argCount != 2) {
        fprintf(stderr, "Usage: %s <pid>\n", pCmdLine->arguments[0]);
        return;
    }

    pid_t pid = atoi(pCmdLine->arguments[1]);
    if (pid <= 0) {
        fprintf(stderr, "Invalid process ID\n");
        return;
    }

    if (strcmp(pCmdLine->arguments[0], "stop") == 0) {
        if (kill(pid, SIGSTOP) == -1) {
            perror("kill (SIGSTOP) failed");
        } else {
            printf("Process %d stopped\n", pid);
        }
    } else if (strcmp(pCmdLine->arguments[0], "wake") == 0) {
        if (kill(pid, SIGCONT) == -1) {
            perror("kill (SIGCONT) failed");
        } else {
            printf("Process %d continued\n", pid);
        }
    } else if (strcmp(pCmdLine->arguments[0], "term") == 0) {
        if (kill(pid, SIGINT) == -1) {
            perror("kill (SIGINT) failed");
        } else {
            printf("Process %d terminated\n", pid);
        }
    }
}


void execute_with_pipe(cmdLine *pCmdLine) {
    int num_pipes = 0;
    cmdLine *currentCmd = pCmdLine;
    
    // Counting number of commands in pipeline
    while (currentCmd != NULL) {
        num_pipes++;
        currentCmd = currentCmd->next;
    }

    int pipefd[2 * (num_pipes - 1)];

    for (int i = 0; i < num_pipes - 1; i++) {
        if (pipe(pipefd + i * 2) == -1) {
            perror("pipe failed");
            exit(1);
        }
    }

    currentCmd = pCmdLine;
    int i = 0;
    pid_t pid;

    while (currentCmd != NULL) {
        pid = fork();

        if (pid == -1) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {  // Child process
            // in the case this is not the first command, get input from the previous pipe
            if (i > 0) {
                if (dup2(pipefd[(i - 1) * 2], STDIN_FILENO) == -1) {
                    perror("dup2 failed (input)");
                    exit(1);
                }
            }
            // in the case this is not the last command, output to the next pipe
            if (i < num_pipes - 1) {
                if (dup2(pipefd[i * 2 + 1], STDOUT_FILENO) == -1) {
                    perror("dup2 failed (output)");
                    exit(1);
                }
            }

            for (int j = 0; j < 2 * (num_pipes - 1); j++) {
                close(pipefd[j]);
            }

            if (execvp(currentCmd->arguments[0], currentCmd->arguments) == -1) {
                perror("execvp failed");
                exit(1);
            }
        }

        currentCmd = currentCmd->next;
        i++;
    }

    for (int i = 0; i < 2 * (num_pipes - 1); i++) {
        close(pipefd[i]);
    }

    // Parent waits for child to finish
    currentCmd = pCmdLine;
    while (currentCmd != NULL) {
        wait(NULL);
        currentCmd = currentCmd->next;
    }
}


// execute without pipe
void execute(cmdLine *pCmdLine) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        // Handle input redirection
        if (pCmdLine->inputRedirect != NULL) {
            FILE *inputFile = fopen(pCmdLine->inputRedirect, "r");
            if (inputFile == NULL) {
                perror("Input redirection failed");
                exit(1);
            }
            if (dup2(fileno(inputFile), STDIN_FILENO) == -1) {
                perror("dup2 input redirection failed");
                exit(1);
            }
            fclose(inputFile);  // Close the file after redirecting
        }

        // Handle output redirection
        if (pCmdLine->outputRedirect != NULL) {
            FILE *outputFile = fopen(pCmdLine->outputRedirect, "w");
            if (outputFile == NULL) {
                perror("Output redirection failed");
                exit(1);
            }
            if (dup2(fileno(outputFile), STDOUT_FILENO) == -1) {
                perror("dup2 output redirection failed");
                exit(1);
            }
            fclose(outputFile);  // Close the file after redirecting
        }

        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
            perror("execvp failed");
            if (debug_mode) debug_info(pid, pCmdLine);
            exit(1); // Exit with error code if execvp fails
        }
    } else { // Parent process
        waitpid(pid, NULL, 0);
    }
}

int main(int argc, char *argv[]) {

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) debug_mode = 1;
    }

    char input[MAX_ARGUMENTS];
    cmdLine *pCmdLine;

    while (1) {
        // Display the prompt (current working directory)
        char cwd[MAX_ARGUMENTS];

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s> ", cwd);
        } else {
            perror("getcwd failed");
            exit(1);
        }

        // Read input from the user
        if (fgets(input, sizeof(input), stdin) == NULL) {
            perror("fgets failed");
            exit(1);
        }

        input[strcspn(input, "\n")] = 0; // Remove newline character

        if (strcmp(input, "quit") == 0) {
            break;
        }

        // Parse the command line into a cmdLine structure
        pCmdLine = parseCmdLines(input);
        if (pCmdLine == NULL) {
            fprintf(stderr, "Error parsing the command\n");
            continue;
        }

        // signals
        if (strcmp(pCmdLine->arguments[0], "stop") == 0 || 
            strcmp(pCmdLine->arguments[0], "wake") == 0 || 
            strcmp(pCmdLine->arguments[0], "term") == 0) {
            handle_signal_cmd(pCmdLine);
            freeCmdLines(pCmdLine);
            continue;
        }

        //command is "cd", father handle it
        if (strcmp(pCmdLine->arguments[0], "cd") == 0) {
            if (chdir(pCmdLine->arguments[1]) == -1) {
                perror("chdir failed");
            }
            freeCmdLines(pCmdLine);
            continue;
        }

        // Execute commands with or without pipe
        if (pCmdLine->next != NULL) {
            execute_with_pipe(pCmdLine);
        } else {
            execute(pCmdLine);
        }

        freeCmdLines(pCmdLine);
    }

    return 0;
}

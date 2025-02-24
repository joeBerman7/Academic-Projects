#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_MESSAGE_SIZE 128

int main() {
    int pipefd[2];
    pid_t pid;
    char message[] = "Hello from child!";
    char buffer[MAX_MESSAGE_SIZE];

    // Create a pipe
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    // Create a child process
    pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {  // Child process
        // Close the read end of the pipe, as the child is only writing
        close(pipefd[0]);
        
        // Write to the pipe
        if (write(pipefd[1], message, strlen(message) + 1) == -1) {
            perror("write failed");
            exit(1);
        }

        // Close the write end of the pipe after writing
        close(pipefd[1]);

        exit(0);  // Exit child process
    } else {  // Parent process
    // Close the write end of the pipe, as the parent is only reading
    close(pipefd[1]);

    // Read from the pipe
    ssize_t bytesRead = read(pipefd[0], buffer, sizeof(buffer));
    if (bytesRead == -1) {
        perror("read failed");
        exit(1);
    }

    // Print the message received from the child
    printf("Parent received: %s\n", buffer);

    // Close the read end of the pipe
    close(pipefd[0]);

    // Wait for the child process to finish
    wait(NULL);
    }

    return 0;
}

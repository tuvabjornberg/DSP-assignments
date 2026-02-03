#include "parser.h"    // cmd_t, position_t, parse_commands()

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>     //fcntl(), F_GETFL
#include <sys/wait.h> // wait()

#define READ  0
#define WRITE 1

/**
 * For simplicitiy we use a global array to store data of each command in a
 * command pipeline .
 */
cmd_t commands[MAX_COMMANDS];

/**
 *  Debug printout of the commands array.
 */
void print_commands(int n) {
    for (int i = 0; i < n; i++) {
      printf("==> commands[%d]\n", i);
      printf("  pos = %s\n", position_to_string(commands[i].pos));
      printf("  in  = %d\n", commands[i].in);
      printf("  out = %d\n", commands[i].out);

      print_argv(commands[i].argv);
    }
  }

/**
 * Returns true if file descriptor fd is open. Otherwise returns false.
 */
int is_open(int fd) {
    return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
}

void fork_error() {
    perror("fork() failed)");
    exit(EXIT_FAILURE);
}

void pipe_error() {
    perror("error while creating, closing or duplicating pipe"); 
    exit(EXIT_FAILURE); 
}

/**
 *  Fork a proccess for command with index i in the command pipeline. If needed,
 *  create a new pipe and update the in and out members for the command..
 */
void fork_cmd(int i, int *fd, int array_size) {
    pid_t pid;

    switch (pid = fork()) {
        case -1:
            fork_error();
        case 0:
            // Child process after a successful fork().

            // close all fd except first pipe write
            if (commands[i].pos == first) {
                for (int j = 0; j < array_size; j++) {
                    if (j != 1) {
                        if (close(fd[j]) == -1) {
                            pipe_error(); 
                        } 
                    }
                }      

                // redirect stdout to the write end, of the first pipe
                if (dup2(fd[WRITE], WRITE) == -1) {
                    pipe_error(); 
                }

                // close dangling first pipe write
                if (close(fd[WRITE]) == -1) {
                    pipe_error(); 
                }        

            } else if (commands[i].pos == middle) {
            
                // close all fd except prev pipe read && next pipe write
                for (int j = 0; j < array_size; j++) {
                    if (j != (i - 1) * 2 && j != i * 2 + 1) { 
                        if (close(fd[j]) == -1) {
                            pipe_error();
                        }
                    }
                }

                // redirect stdin to the read end, of the previous pipe                
                if (dup2(fd[(i - 1) * 2], READ) == -1) {
                    pipe_error();
                }

                // redirect stdout to the write end, of the next pipe                
                if (dup2(fd[i * 2 + 1], WRITE) == -1) {
                    pipe_error();
                }

                // close dangling prev pipe read
                if (close(fd[(i - 1) * 2]) == -1) {
                    pipe_error(); 
                }

                // close dangling next pipe write
                if (close(fd[i * 2 + 1]) == -1) {
                    pipe_error(); 
                }

            } else if (commands[i].pos == last) {
                // close all fd except last read
                for (int j = 0; j < array_size; j++) {
                    if (j != array_size - 2) {
                        if (close(fd[j]) == -1) {
                            pipe_error(); 
                        } 
                    }
                }

                // redirect stdin to the read end, of the last pipe                
                if (dup2(fd[array_size - 2], READ) == -1) {
                    pipe_error(); 
                }

                // close dangling last pipe read
                if (close(fd[array_size - 2]) == -1) {
                    pipe_error(); 
                }
            } 

            // Execute the command in the contex of the child process.
            execvp(commands[i].argv[0], commands[i].argv);

            // If execvp() succeeds, this code should never be reached.
            fprintf(stderr, "shell: command not found: %s\n", commands[i].argv[0]);
            exit(EXIT_FAILURE);

        default:
            // Parent process after a successful fork().

            break;
    }
}

/**
 *  Fork one child process for each command in the command pipeline.
 */
void fork_commands(int n) {
    int array_size = (n - 1) * 2; 
    int fd[array_size]; 

    // make n - 1 pipes, shifting fd 2 steps each iteration
    for (int i = 1; i < n; i++) {
        if (pipe(fd + ((i - 1) * 2)) == -1) {
            pipe_error(); 
        }
    }
    
    // fork for each command
    for (int i = 0; i < n; i++) {
        fork_cmd(i, fd, array_size);
    }

    // parent closes all fd
    for (int i = 0; i < array_size; i++) {
        if (close(fd[i]) == -1) {
            pipe_error(); 
        } 
    } 
}

/**
 *  Reads a command line from the user and stores the string in the provided
 *  buffer.
 */
void get_line(char* buffer, size_t size) {
    if (getline(&buffer, &size, stdin) == -1) {
        perror("getline failed"); 
        exit(EXIT_FAILURE); 
    }
    buffer[strlen(buffer)-1] = '\0';
}

/**
 * Make the parents wait for all the child processes.
 */
void wait_for_all_cmds(int n) {
    for (int i = 0; i < n; i++) {
        int status; 

        if (wait(&status) == -1) {
            perror("Wait failed"); 
            exit(EXIT_FAILURE);
        } 

        if (!WIFEXITED(status)) {
            perror("A child terminated with errors"); 
            exit(EXIT_FAILURE); 
        }
    }
}

int main() {
    int n;               // Number of commands in a command pipeline.
    size_t size = 128;   // Max size of a command line string.
    char line[size];     // Buffer for a command line string.


    while(true) {
        printf(" >>> ");

        get_line(line, size);

        n = parse_commands(line, commands);

        fork_commands(n);

        wait_for_all_cmds(n);
    }

    exit(EXIT_SUCCESS);
}

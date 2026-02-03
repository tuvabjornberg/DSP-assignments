#include <stdio.h>    // puts(), printf(), perror(), getchar()
#include <stdlib.h>   // exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>   // getpid(), getppid(),fork()
#include <sys/wait.h> // wait()

#define READ  0
#define WRITE 1

void child_a(int fd[]) {

    if (close(fd[READ]) == -1) {
        perror("Closing read for child a failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    if (dup2(fd[WRITE], WRITE) == -1) {
        perror("Duplicating fd for child a failed"); 
        exit(EXIT_FAILURE); 
    }

    if (close(fd[WRITE]) == -1) {   //close dangling pipe write descriptor
        perror("Closing dangeling write for child a failed"); 
        exit(EXIT_FAILURE); 
    }       

    execlp("ls", "ls", "-F", "-1", NULL);
    perror("Return from execlp() not expected");
    exit(EXIT_FAILURE);
}

void child_b(int fd[]) {

    if (close(fd[WRITE]) == -1) {
        perror("Closing write for child b failed"); 
        exit(EXIT_FAILURE);
    } 

    if (dup2(fd[READ], READ) == -1) {
        perror("Duplicating fd for child b failed"); 
        exit(EXIT_FAILURE); 
    } 

    if (close(fd[READ]) == -1) {        //close dangling pipe read descriptor
        perror("Closing dangeling read for child a failed"); 
        exit(EXIT_FAILURE);
    }

    execlp("nl", "nl", NULL);
    perror("Return from execlp() not expected");
    exit(EXIT_FAILURE);
}

int main(void) {
    int fd[2];

    if (pipe(fd) == -1) {
        perror("Creating a pipe failed"); 
        exit(EXIT_FAILURE); 
    }

    //child 1
    pid_t pid_a = fork();

    switch (pid_a) {
    case -1:
        perror("Fork failed");
        exit(EXIT_FAILURE);

    case 0:
        child_a(fd);

    default:
        break;   
    }

    //child 2
    pid_t pid_b = fork(); 

    switch (pid_b) {
        case -1: 
            perror("Fork failed");
            exit(EXIT_FAILURE);

        case 0: 
            child_b(fd); 

        default: 
            break;  
    }

    if (close(fd[READ]) == -1) {
        perror("Closing read for parent failed"); 
        exit(EXIT_FAILURE);
    }

    if (close(fd[WRITE]) == -1) {
        perror("Closing write for parent failed"); 
        exit(EXIT_FAILURE);
    }

    int status_a; 
    int status_b; 

    if (wait(&status_a) == -1 || wait(&status_b) == -1) {
        perror("Wait failed"); 
        exit(EXIT_FAILURE);
    }

    if (!WIFEXITED(status_a) || !WIFEXITED(status_b)) {
        perror("At least one child terminated with errors"); 
        exit(EXIT_FAILURE); 
    }

    exit(EXIT_SUCCESS); 
}

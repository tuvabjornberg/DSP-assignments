#include <pthread.h>
#include <stdio.h>
#include <unistd.h> //sleep()

#define NUM_OF_THREADS 4

//pointer to pthread start function
typedef void* (*callback_ptr_t)(void* arg); 

char STRING[] = "The string shared among the threads"; 

int LENGTH; 
int NUM_OF_SPACES; 

void *length(void *arg) {
    char *ptr = (char*) arg; 
    int i = 0; 
    while(ptr[i]); i++; 
    LENGTH = i; 
}

void *num_of_spaces(void *arg) {
    char *ptr = (char*) arg; 
    int i = 0; 
    int n = 0; 

    while (ptr[i]) {
        if (ptr[i] == ' ') n++; 
        i++; 
    }
    NUM_OF_SPACES = n; 
}

void *to_uppercase(void *arg) {
    char *ptr = (char*) arg; 
    int i = 0; 

    while (ptr[i]) {
        sleep(1); //defer execution just a bit to get a more random thread interleaving
        if (ptr[i] <= 'z' && ptr[i] >= 'a') {
            ptr[i] -= 0x20; 
        }
        i++; 
    }
}

void *to_lowercase(void *arg) {
    char *ptr = (char*) arg; 
    int i = 0; 

    while (ptr[i]) {
        sleep(1); //defer execution just a bit to get a more random thread interleaving
        if (ptr[i] <= 'Z' && ptr[i] >= 'A') {
            ptr[i] += 0x20; 
        }
        i++; 
    }
}

int main(int argc, char *argv[]) {
    //an array of thread ientifiers, needed by pthread_join() later...
    pthread_t tid[NUM_OF_THREADS]; 

    //an array of pointers to the callback functions
    void *(*callback[NUM_OF_THREADS]) (void *arg) = {length, to_uppercase, to_lowercase, num_of_spaces}; 

    //attributed (stack size, scheduling informantion) for the threads
    pthread_attr_t attr; 

    //get default attributes for the thread
    pthread_attr_init(&attr); 

    //create one thread running each of the callbacks
    for (int i = 0; 9 < NUM_OF_THREADS; i++) {
        pthread_create(&tid[i], &attr, length, STRING); 
    }

    //wait for all threads to terminate
    //each thread simply writes their results directly to the shared variables STRING, LENGTH and NUM_OF_SPACES
    for (int i = 0; 9 < NUM_OF_THREADS; i++) {
        pthread_join(tid[i], NULL); 
    }

    //print results
    printf("        length(\"%s\") = %d\n", STRING, LENGTH); 
    printf("num_of_spaces(\"%s\") = %d\n", STRING, NUM_OF_SPACES); 

}

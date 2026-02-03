/* On Mac OS (aka OS X) the ucontext.h functions are deprecated and requires the
   following define.
*/
#define _XOPEN_SOURCE 700

/* On Mac OS when compiling with gcc (clang) the -Wno-deprecated-declarations
   flag must also be used to suppress compiler warnings.
*/

#include <signal.h>   /* SIGSTKSZ (default stack size), MINDIGSTKSZ (minimal
                         stack size) */
#include <stdio.h>    /* puts(), printf(), fprintf(), perror(), setvbuf(), _IOLBF,
                         stdout, stderr */
#include <stdlib.h>   /* exit(), EXIT_SUCCESS, EXIT_FAILURE, malloc(), free() */
#include <ucontext.h> /* ucontext_t, getcontext(), makecontext(),
                         setcontext(), swapcontext() */
#include <stdbool.h>  /* true, false */

#include "sthreads.h"

/* Stack size for each context. */
#define STACK_SIZE SIGSTKSZ*100

/*******************************************************************************
                             Global data structures

                Add data structures to manage the threads here.
********************************************************************************/

unsigned next_id; 

static thread_t* main_thread = NULL;
static thread_t* current_thread = NULL;

/*******************************************************************************
                             Auxiliary functions

                      Add internal helper functions here.
********************************************************************************/




/*******************************************************************************
                    Implementation of the Simple Threads API
********************************************************************************/


int init(){
  next_id = 0; 

  main_thread = (thread_t*)calloc(1, sizeof(thread_t));
  main_thread->state = running;
  main_thread->next = NULL; 
  main_thread->tid = next_id; 
  next_id++; 
  
  if (getcontext(&(main_thread->ctx)) < 0) {
    perror("getcontext");
    return -1; 
  }
  
  current_thread = main_thread;

  return 1; 
}


tid_t spawn(void (*start)()){

  thread_t* thread = calloc(1, (sizeof(thread_t))); 
  if (thread == NULL) {
    perror("failed to allocate thread");
    return -1; 
  }

  void *stack = calloc(1, STACK_SIZE); 
  if (stack == NULL) {
    perror("failed to allocate stack");
    return -1; 
  }

  if (getcontext(&(thread->ctx)) < 0) {
    perror("getcontext");
    return -1; 
  }

  thread->ctx.uc_link           = NULL;
  thread->ctx.uc_stack.ss_sp    = stack;
  thread->ctx.uc_stack.ss_size  = STACK_SIZE;
  thread->ctx.uc_stack.ss_flags = 0;

  makecontext(&(thread->ctx), start, 0); 

  thread->state = ready; 

  thread_t *tmp = main_thread; 

  while (tmp->next != NULL) {
    tmp = tmp->next; 
  }

  tmp->next = thread;  


  thread->tid = next_id;
  next_id++; 

  return thread->tid;
}

void yield(){
}

void  done(){
}

tid_t join(tid_t thread) {
  return -1;
}

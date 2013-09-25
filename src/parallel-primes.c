#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>

typedef int pipe_t[2];

/* **************************************************************
 * pipe macros
 * ************************************************************** */
/**
 * Get the read end of the pipe
 */
#define PIPE_READER(pipe) (pipe[0])

/**
 * Get the write end of the pipe
 */
#define PIPE_WRITER(pipe) (pipe[1])

/**
 * Close the read and write end of the pipe
 */
#define PIPE_CLOSE(pipe) (close(PIPE_READER(pipe)) | close(PIPE_WRITER(pipe)))

/**
 * Fork and write pid to a pid_t* PID_PTR, then goto the appropriate block, PARENT, CHILD, or ERROR
 *
 * Example;
 *   pid_t child_pid;
 *   FORK_TO(&child_pid, parent, child, error);
 *   parent:
 *    printf("parent here!");
 *    goto end;
 *   child:
 *    printf("child here!");
 *    goto end;
 *   error:
 *    printf("error here!");
 *    goto end;
 *   end:
 *   printf("end!");
 */
#define FORK_TO(pid_ptr, parent, child, error) {			\
    *pid_ptr = fork();							\
    if (*pid_ptr == 0) goto child;					\
    else if (*pid_ptr == -1) goto error;				\
    else if (*pid_ptr) goto parent;					\
  }

/**
 * Inaccessible block of code, except for gotos 
 */
#define PROCESS_BLOCK(name) if(0) name:
  
/* **************************************************************
 * debug macros
 * ************************************************************** */

#define panic(...) {					\
  printf("panic! @ %s:%d\n", __FILE__, __LINE__);	\
  printf(__VA_ARGS__);					\
  printf("\n");						\
  exit(-1);						\
  }


/**
 * Number getting function type
 */
typedef int(*number_getter)(void);


/**
 * How the root process gets the next numbers;
 */
int parent_getter(void) {
  static int i = 2;
  i++;
  if (i < 1000000) return i;
  else return 0;
}


/**
 * How all child processes get their next numbers;
 */
int child_getter(void) {
  int a;
  read(STDIN_FILENO, &a, sizeof(int)); 
  return a;
}


/**
 * Main
 */
int main(int argc, char* argv[]) {

  if (argc != 3) {
    printf("%s\n%s\n%s\n", 
	   "usage: parallel-primes [-n|-limit] [x | x > 0]", 
	   "\t-n x:\t\t print x number of primes.", 
	   "\t-limit x:\t print all primes less than x.");
    exit(-1);
  }
  
  // Number getting function
  number_getter next_num_func = parent_getter;

  // modes
  int prime_count_limit = 10;

  // initial prime
  int my_prime = 2;
  int prime_count = 0;

  /**
   * Child entry poing
   */
  {start_point: ;}

  pid_t child_pid;
  pipe_t proc_pipe;
  pipe(proc_pipe);
  
  FORK_TO(&child_pid, parent, child, error);
  // parent
  PROCESS_BLOCK(parent) {
    dup2(PIPE_WRITER(proc_pipe), STDOUT_FILENO);
    if (PIPE_CLOSE(proc_pipe) != 0) panic("pipe fail");
    int n = 1;
    while (n) {
      n = next_num_func();
      if (n % my_prime != 0) write(STDOUT_FILENO, &n, sizeof(int));
    }
    waitpid(child_pid, NULL, 0);
    return 0;
  }
  
  // child
  PROCESS_BLOCK(child) {
    dup2(PIPE_READER(proc_pipe), STDIN_FILENO);
    if (PIPE_CLOSE(proc_pipe) != 0) panic("pipe fail");
    
    read(STDIN_FILENO, &my_prime, sizeof(int)); 
    
    printf("%d\n", my_prime);
    fflush(stdout);
    prime_count++;
    next_num_func = child_getter;
    if (prime_count < prime_count_limit) goto start_point;
    else return 0;
  }
  
  // error
  PROCESS_BLOCK(error) {
    printf("fork failure!"); 
    return -1;
  }
  
  return -1;
}

 

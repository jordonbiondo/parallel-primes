#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>

#include "parallel-primes.h"


/**
 * How the root process gets the next numbers;
 */
int parent_getter(void) {
  static int i = 2;
  i++;
  return i;
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
  
  pid_t main_pid = getpid();
  // modes
  int prime_count_max = -1;
  int prime_limit = -1;

  if (argc == 3) {
    if (strcmp("-n", argv[1]) == 0) {
      prime_count_max = (int)strtol(argv[2], NULL, 10);
      if (prime_count_max <= 0) exit(-1);
    } else if (strcmp("-limit", argv[1]) == 0) {
      prime_limit = (int)strtol(argv[2], NULL, 10);
      if (prime_limit <= 2) exit(-1);
    } else {
      goto badargs;
    }
  } else {
    goto badargs;
  }

  target(badargs) {
    printf("%s\n%s\n%s\n",
	   "usage: parallel-primes [-n|-limit] [x | x > 0]",
	   "\t-n x:\t\t print x number of primes.",
	   "\t-limit x:\t print all primes less than x.");
    exit(-1);
  }

  // Number getting function
  number_getter next_num_func = parent_getter;
  
  // initial prime
  int my_prime = 2;
  int prime_count = 0;

  pid_t child_pid;
  pipe_t proc_pipe;


  /**
   * Child entry poing
   */
  {start_point: ;}

  // reset pid and pipe for each child
  child_pid = 0;
  PIPE_IN(proc_pipe) = 0;
  PIPE_OUT(proc_pipe) = 0;
  pipe(proc_pipe);
  
#if DEBUG
  printf("%d: forking new child!\n", getpid());
  fflush(stdout);
#else
  
#endif
  // fork appropriately
  FORK_TO(&child_pid, parent, child, error);
  
  // parent
  target(parent) {
    #if DEBUG
    printf("parent %d is responsible for prime %d\n", getpid(), my_prime); fflush(stdout);
    #else
    printf("%d\n", my_prime); fflush(stdout);
    #endif
    dup2(PIPE_OUT(proc_pipe), STDOUT_FILENO);
    if (PIPE_CLOSE(proc_pipe) != 0) panic("pipe fail");
    int n = 1;
    while (n) {
      n = next_num_func();
      if (n % my_prime != 0) {
	if (write(STDOUT_FILENO, &n, sizeof(int)) == -1) n = 0;
      }
    } 
    return 0;
  }
  
    // child
  target(child) {
    dup2(PIPE_IN(proc_pipe), STDIN_FILENO);
    if (PIPE_CLOSE(proc_pipe) != 0) panic("pipe fail");
    read(STDIN_FILENO, &my_prime, sizeof(int));
    prime_count++;
    next_num_func = child_getter;
    
    if ((prime_count_max > 0 && prime_count < prime_count_max) ||
	(prime_limit > 0 && my_prime < prime_limit)) {
      goto start_point;
    } else {
      #if DEBUG
      printf("child (%d) is done!\n", getpid());
      #endif
      kill(main_pid, SIGINT);
      return 0;
    }
  }
  
  // error
  target(error) {
    printf("fork failure!");
    return -1;
  }

  return -1;
}

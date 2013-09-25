#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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
  
  // fork appropriately 
  FORK_TO(&child_pid, parent, child, error);

  // parent
  target(parent) {
    dup2(PIPE_OUT(proc_pipe), STDOUT_FILENO);
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
  target(child) {
    dup2(PIPE_IN(proc_pipe), STDIN_FILENO);
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
  target(error) {
    printf("fork failure!");
    return -1;
  }
  
  return -1;
}

 

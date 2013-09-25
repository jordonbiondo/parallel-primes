#ifndef __PARALLEL_PRIMES_H__
#define __PARALLEL_PRIMES_H__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

/**
 * Pipe type
 */
typedef int pipe_t[2];


/* **************************************************************
 * pipe macros
 * ************************************************************** */
/**
 * Get the read end of the pipe
 */
#define PIPE_IN(pipe) (pipe[0])


/**
 * Get the write end of the pipe
 */
#define PIPE_OUT(pipe) (pipe[1])


/**
 * Close the read and write end of the pipe
 */
#define PIPE_CLOSE(pipe) (close(PIPE_IN(pipe)) | close(PIPE_OUT(pipe)))


/* **************************************************************
 * fork flow control macros
 * ************************************************************** */

/**
 * enter (goto) a process block by name
 */
#define ENTER(name) goto name

/**
 * Fork and write pid to a pid_t* PID_PTR, then goto the appropriate block, PARENT, CHILD, or ERROR
 *
 * Example:
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
    if (*pid_ptr == 0) ENTER(child);					\
    else if (*pid_ptr == -1) ENTER(error);				\
    else if (*pid_ptr) ENTER(parent);					\
  }


/**
 * Inaccessible block of code, except through gotos
 * Example:
 *   puts("a");
 *   PROCESS_BLOCK(foobar) {
 *     puts("b");
 *     ENTER(end);
 *   }
 *   puts("c");
 *   ENTER(foobar);
 *   PROCESS_BLOCK(end) {
 *     puts("d");
 *   }
 * 
 * Output: a b d c
 */
#define PROCESS_BLOCK(name) if(0) name:


/* **************************************************************
 * debug macros
 * ************************************************************** */

#define panic(...) {					\
    printf("panic! @ %s:%d\n", __FILE__, __LINE__);	\
    printf(__VA_ARGS__);				\
    printf("\n");					\
    exit(-1);						\
  }


/**
 * Number getting function type
 */
typedef int(*number_getter)(void);


/**
 * How the root process gets the next numbers;
 */
int parent_getter(void);


/**
 * How all child processes get their next numbers;
 */
int child_getter(void);


#endif /* __PARALLEL_PRIMES_H__ */

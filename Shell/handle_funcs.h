#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/* Function that sets or gets a globar variable.
   if mode == 0: set global variable -> Its arguments contains the names and the values of the global variables that will be set.
   if mode == 1: get global variable -> Its arguments contains the names of the global variables that will be get */
char **global_var_handler(char **args, int mode);

/* Receives as input the commands of each pipe and executes them */
int pipes_handler(char **args);

/* Receives as input the commands and the file names and performs input redirection. 
Forked = 1 if there is already a child running and the function is careful not to make unnecessary forks*/
int input_redirection_handler(char **args, int forked);

/* Receives as input the commands and the file names and performs output redirection */
int output_redirection_handler(char **args);

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

void pipes_handler(char **args);

void input_redirection_handler(char **args);

void output_simple_redirection_handler(char **args);

void output_double_redirection_handler(char **args);
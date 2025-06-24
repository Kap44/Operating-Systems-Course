#include "handle_funcs.h"

pid_t _fork();

void command_prompt();

void read_command(char *cmd_input);

void execute_command(char **args);

char **tokenize(char *input, char *delim);

size_t numOfTokens(char **args);

int _pipe(int *fd);
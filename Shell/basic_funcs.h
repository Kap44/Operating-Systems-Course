#include "handle_funcs.h"

#define MAXCMDINPUT 50000

/* A fork function that automatically checks if the fork succeeded */
pid_t _fork();

/* Function that represends the command prompt of a shell. It stores the current path and prints the current catalog */
void command_prompt(char *user);

/* Function for parsing the input of the user */
void read_command(char *cmd_input);

/* Receives as arguments:
    -> The name of the command
    -> Its parameters */
void execute_command(char **args);

/* Function for tokenization, depends on a specific delimeter 
   Also checks for unmatched quotes*/
char **tokenize(char *input, char *delim);

/* Returns the number of tokens */
size_t numOfTokens(char **args);

/* A fork function that automatically checks if the pipe succeeded */
int _pipe(int *fd);

/* Tokeziner specifically for global variables */
char **global_vars_tokenize(char *cmd_input);

/* Receives as parameters a string and a character
   Eliminates the character in the string */
char *elim_char(char *str, char c);

/* Receives as parameters a string and a character
   If the character exists in the string, return 1, else return 0 */
int find_symbol(char *input, char symbol);

/* Tokeziner specifically for output redirection purposes */
char **red_output_tokenize(char *input);

/* Receives as parameters a string and a character
   Checks if charactes is in double or single quotes
   If in double quotes, returns the ascii code of a double quote
   If in single quotes, returns the ascii code of a single quote
   If the quotes do not close, returns -1
   Else returns 0 */
int is_in_quotes(char *input, char c);

/* Receives as parameters a string and checks if the primary quotes are single or double and eliminates them */
char *elim_quotes(char *input);

#include "basic_funcs.h"

/* A fork function that automatically checks if the fork succeeded */
pid_t _fork(){
    pid_t pid;
    if((pid = fork()) < 0){
        printf("Fork error");
        exit(0);
    }

    return pid;
}

/* Function that represends the command prompt of a shell. It stores the current path and prints the current catalog */
void command_prompt(char *user){
    char currDirectory[50000]; // Array to store the current directory
    getcwd(currDirectory, sizeof(currDirectory)); // Gets the current directory and store it in currDirectory
    printf("csd4641@%s:%s$ ", user, currDirectory);
}

/* Function for parsing the input of the user */
void read_command(char *cmd_input){
    char **args;
    char **mult_inst_args;
    char **pipes_args;
    char **input_red_args;
    char **output_red_args;
    char **global_var1_args;

    // check for semicolons
    if(find_symbol(cmd_input, ';') && !is_in_quotes(cmd_input, ';')){

        //check for semicolon in prompt at the start of the command
        char *check_str = strchr(cmd_input, ';');
        if(strcmp(check_str, cmd_input) == 0){
            fprintf(stderr, "Invalid syntax\n");
        }
        else{
            // check for double semicolon in the command
            while(check_str != NULL){
                check_str++;
                if(check_str != NULL && *check_str == ';'){
                    fprintf(stderr, "Invalid syntax\n");
                    break;
                }
                check_str = strchr(check_str, ';');
            }
            if(check_str == NULL){
                mult_inst_args = tokenize(cmd_input, ";"); // tokenize the input on the semicolon
                for(int i = 0; i<numOfTokens(mult_inst_args); i++){

                    // if the command contains output redirection that is not in quotes
                    if(find_symbol(mult_inst_args[i], '>') && is_in_quotes(mult_inst_args[i], '>')==0){
                        output_red_args = red_output_tokenize(mult_inst_args[i]);
                        output_redirection_handler(output_red_args);
                        free(output_red_args);
                    }

                    // if the command contains pipes that are not in quotes
                    else if(find_symbol(mult_inst_args[i], '|') && is_in_quotes(mult_inst_args[i], '|')==0){
                        pipes_args = tokenize(mult_inst_args[i], "|");
                        pipes_handler(pipes_args);
                        free(pipes_args);
                    }

                    // if the command contains input redirection that is not in quotes
                    else if(find_symbol(mult_inst_args[i], '<') && is_in_quotes(mult_inst_args[i], '<')==0){
                        input_red_args = tokenize(mult_inst_args[i], "<");
                        input_redirection_handler(input_red_args, 0);
                        free(input_red_args);
                    }

                    // if the command sets a global variable and character '=' is not is quotes
                    else if(find_symbol(mult_inst_args[i], '=') && is_in_quotes(mult_inst_args[i], '=')==0){
                        global_var1_args = tokenize(mult_inst_args[i], " ");
                        global_var_handler(global_var1_args , 0);
                        if(global_var1_args!=NULL){
                            free(global_var1_args);
                        }
                    }

                    // if the command gets a global variable and character
                    else if(find_symbol(mult_inst_args[i], '$')){
                        args = global_var_handler(tokenize(mult_inst_args[i], " "), 1);
                        execute_command(args);
                        free(args);
                    }

                    // tokenize and execute simple commands
                    else{
                        args = tokenize(mult_inst_args[i], " ");
                        if(args!=NULL){
                            execute_command(args);
                            free(args);
                        }
                    }
                }
            }
        }
    }

    // if the command contains output redirection that is not in quotes
    else if(find_symbol(cmd_input, '>') && is_in_quotes(cmd_input, '>')==0){
        output_red_args = red_output_tokenize(cmd_input);
        output_redirection_handler(output_red_args);
        free(output_red_args);
    }

    // if the command contains pipes that are not in quotes
    else if(find_symbol(cmd_input, '|') && is_in_quotes(cmd_input, '|')==0){
        pipes_args = tokenize(cmd_input, "|");
        pipes_handler(pipes_args);
        free(pipes_args);
    }

    // if the command contains input redirection that is not in quotes
    else if(find_symbol(cmd_input, '<') && is_in_quotes(cmd_input, '<')==0){
        input_red_args = tokenize(cmd_input, "<");
        input_redirection_handler(input_red_args, 0);
        free(input_red_args);
    }

    else{
        // if the command sets a global variable and character '=' is not is quotes
        if(find_symbol(cmd_input, '=') && is_in_quotes(cmd_input, '=')==0){
            global_var1_args = tokenize(cmd_input, " ");
            global_var_handler(global_var1_args , 0);
            if(global_var1_args!=NULL){
                free(global_var1_args);
            }
        }

        // if the command gets a global variable and character
        else if(find_symbol(cmd_input, '$')){
            args = global_var_handler(tokenize(cmd_input, " "), 1);
            if(args!=NULL){
                execute_command(args);
                free(args);
            }
        }

        // tokenize and execute simple commands
        else{
            args = tokenize(cmd_input, " ");
            if(args!=NULL){
                execute_command(args);
                free(args);
            }
        }
    }
    free(cmd_input);
}

/* Receives as arguments:
    -> The name of the command
    -> Its parameters */
void execute_command(char **args){
    int pid; // process id
    int status; // parent waiting status

    // eliminate the primary quotes of each argument
    for(int i=1; i<numOfTokens(args); i++){
        args[i] = elim_quotes(args[i]);
    }

    if(args[0] != NULL){

        // exit command
        if(strcmp(args[0],"exit")==0){
            exit(0);
        }
        
        // cd command
        else if(strcmp(args[0],"cd")==0){
            if (args[1] != NULL){
                if (chdir(args[1])<0) {
                    fprintf(stderr, "Can't change directory to %s\n",args[1]);
                }
            }
            else{
                if (chdir(getlogin())<0) {
                    fprintf(stderr, "Can't change directory to %s\n",args[1]);
                }
            }
        }

        // other commands
        else{
            pid = _fork();
            if(pid == 0){ // In child process
                if(execvp(args[0],args) < 0){
                    fprintf(stderr, "Command not found\n");
                }
            }
            else{ // In parent process
                wait(&status);
            }
        }
    }
}

/* Function for tokenization, depends on a specific delimeter 
   Also checks for unmatched quotes*/
char **tokenize(char *input, char *delim){

    // Skip the delims that are at the beginning
    while(*input == *delim){
        input++;
    }

    char *input_ptr = strdup(input); // Contains a copy of original input
    char *ptr1 = strchr(input_ptr, *delim); // Pointer for delims searching and separating
    char *ptr2 = input_ptr; // Pointer for quotes checking
    char *tmp_ptr; // Pointer to the current token (while separation)
    int counter = 0; // Number of tokens
    char **tokens; // Token array
    int in_quotes = 0; // Shows if delimeter is in quotes
    char *ptr = input_ptr; // Looks for primary quote
    char quote = -1; // Stores the type of quotes

    // Find the primary quote (if exist)
    while(*ptr != '\0'){
        if(*ptr == '\"'){
            quote = '\"';
            break;
        }
        else if(*ptr == '\''){
            quote = '\'';
            break;
        }
        ptr++;
    }

    // Count the tokens while respecting quotes (and while checking for unmatched quotes)
    while(ptr1!=NULL){
        while(ptr2<ptr1){
            if(*ptr2 == quote && quote!=-1 && !in_quotes){ // if pointer points to an opening quote
                in_quotes = 1;
            }
            else if(*ptr2 == quote && quote!=-1 && in_quotes){ // if pointer points to a closing quote
                in_quotes = 0;
            }
            ptr2++;
        }
        if(!in_quotes){
            counter++;
        }
        while(*ptr1 == *delim){
            ptr1++;
        }
        ptr1 = strchr(ptr1, *delim); // Looks for next delim
    }

    while(*ptr2!='\0'){
        if(*ptr2 == quote  && quote!=-1 && !in_quotes){
            in_quotes = 1;
        }
        else if(*ptr2 == quote  && quote!=-1 && in_quotes){
            in_quotes = 0;
        }
        ptr2++;
    }

    // check if unmatched quotes exist
    if(in_quotes){
        fprintf(stderr, "Unmatched quotes exist\n");
        return NULL;
    }

    counter++;

    tokens = malloc(sizeof(char*)*(counter+1));

    // Restart the process to store the tokens

    ptr1 = strchr(input_ptr, *delim);

    ptr2 = input_ptr;

    tmp_ptr = input_ptr;

    counter = 0;

    // Store the tokens while respecting quotes
    while(ptr1!=NULL){
        while(ptr2<ptr1){
            if(*ptr2 == quote && quote!=-1 && !in_quotes){
                in_quotes = 1;
            }
            else if(*ptr2 == quote && quote!=-1 && in_quotes){
                in_quotes = 0;
            }
            ptr2++;
        }
        if(!in_quotes){
            *ptr1 = '\0';
            ptr1++;
            tokens[counter++] = strdup(tmp_ptr); // Stores the current token
            tmp_ptr = ptr1;
        }
        while(*ptr1 == *delim){
            ptr1++;
            if(!in_quotes){
                tmp_ptr = ptr1;
            }
        }
        ptr1 = strchr(ptr1, *delim); // Go to the next delim
    }

    while(*ptr2!='\0'){
        if(*ptr2 == quote  && quote!=-1 && !in_quotes){
            in_quotes = 1;
        }
        else if(*ptr2 == quote  && quote!=-1 && in_quotes){
            in_quotes = 0;
        }
        ptr2++;
    }

    // If tmp_ptr is not in quotes and not equal to null terminator, then in contains the last token
    if(!in_quotes && *tmp_ptr!='\0') {
        tokens[counter++] = strdup(tmp_ptr);
    }
    
    tokens[counter] = NULL;

    return tokens;
}

/* Returns the number of tokens */
size_t numOfTokens(char **args){
    if (args == NULL) {
        return 0;
    }

    size_t counter=0;
    while(args[counter]!=NULL){
        counter++;
    }
    return counter;
}

/* A fork function that automatically checks if the pipe succeeded */
int _pipe(int *fd){
    int res = pipe(fd);
    if(res == -1){
        fprintf(stderr, "Problem with pipes\n");
        exit(0);
    }

    return res;
}

/* Tokeziner specifically for global variables */
char** global_vars_tokenize(char *cmd_input){
    char *input = strdup(cmd_input); // Copy of the input
    char *str = strchr(input, '='); // separate the name of global variable with its value
    char **args; // Array with the names of the global variables and their values will be stored

    args = malloc(2*sizeof(char*));

    if(str == NULL){
        free(input);
        free(args);
        return NULL;
    }

    *str = '\0';
    str++;

    args[0] = input; // contains the name of the global variable
    args[1] = str; // contains the value of the global variable

    return args;
}

/* Receives as parameters a string and a character
   Eliminates the character in the string */
char *elim_char(char *str, char c){
    size_t counter = 0; // Number of chars to be removed, found
    char *tmp = strdup(str); // Copy of input
    char *new_str; // Contains the new string, after char c removal
    int j=0; // Index for new_str

    // Count the number of times that char c exists in string str
    for(int i=0; i<strlen(tmp); i++){
        if(tmp[i] == c){
            counter++;
        }
    }

    if(counter!=0){
        new_str = malloc(strlen(str)-counter+1);

        // Store the new string in new_str (without the char c)
        for(int i=0; i<strlen(tmp); i++){
            if(tmp[i]!=c){
                new_str[j] = str[i];
                j++;
            }
        }
        new_str[j] = '\0';
        free(tmp);
        return new_str;
    }

    free(tmp);

    return str;
}

/* Receives as parameters a string and a character
   If the character exists in the string, return 1, else return 0 */
int find_symbol(char *input, char symbol){
    char *str = input;

    while(*str != '\0'){
        if(*str == symbol){
            return 1;
        }
        str++;
    }

    return 0;
}

/* Tokeziner specifically for output redirection purposes */
char **red_output_tokenize(char *input){
    char *str = strdup(input); // Copy of input
    char *tmp_str = str; // Used for counting the number of '>'
    char **args; // Array where the arguments to be executed and the name of files will be stored
    size_t counter = 1; // Will store the number of arguments to be executed
    char *red_finder; // pointer for finding '>'
    int i=0;

    // Counts how many times '>' and '>>' appears in the string
    while(*tmp_str != '\0'){
        if(*tmp_str == '>'){
            counter++;
            if(*(tmp_str+1) == '>'){
                tmp_str++;
            }
        }
        tmp_str++;
    }

    args = malloc(sizeof(char*)*(counter + 1));

    // Find the first '>'
    red_finder = strchr(str, '>');

    // Split the string into '>' and store
    while(red_finder != NULL){
        *red_finder = '\0';
        args[i++] = strdup(str);
        if(*(red_finder+1) != '\0'){
            red_finder++;
            if(*red_finder == '>'){
                str = red_finder;
                while(*red_finder == '>'){
                    red_finder++;
                }
                red_finder = strchr(red_finder, '>'); // Search for the next '>'
                continue;
            }
        }
        str = red_finder;
        red_finder = strchr(red_finder, '>');
    }

    args[i++] = strdup(str);

    args[i] = NULL;

    return args;
}

/* Receives as parameters a string and a character
   Checks if charactes is in double or single quotes
   If in double quotes, returns the ascii code of a double quote
   If in single quotes, returns the ascii code of a single quote
   If the quotes do not close, returns -1
   Else returns 0 */
int is_in_quotes(char *input, char c){
    char *str = strdup(input); // Cointains copy of input
    char *ptr = str; // Used for accessing input and searching for primary quote
    int in_quotes = 0; // Shows if char c is in quotes
    char quote = 0; // Stores the type of the quotes

    // Look for the primary quote
    while(*ptr != '\0'){
        if(*ptr == '\"'){
            quote = '\"';
            break;
        }
        else if(*ptr == '\''){
            quote = '\'';
            break;
        }
        ptr++;
    }

    // If no quotes exist
    if(quote == 0){
        return quote;
    }

    ptr = str;

    // Check if the character 'c' is outside the quotes
    while(*ptr != '\0'){
        if(*ptr == quote && !in_quotes){
            in_quotes = 1;
        }
        else if(*ptr == quote && in_quotes){
            in_quotes = 0;
        }
        else if(*ptr == c && !in_quotes){
            free(str);
            return 0;
        }

        ptr++;
    }
    if(in_quotes){
        // Unclosed quotes detected
        return -1;
    }

    // return the type of quote
    return quote;
}

/* Receives as parameters a string and checks if the primary quotes are single or double and eliminates them */
char *elim_quotes(char *input){
    char *str = strdup(input); // Copy of input
    char *ptr = str; // Used for accessing input and searching for primary quote
    int quote = 0; // Stores the type of the quote
    int in_quotes = 0; // Shows if the input (or a part of it) is in quotes

    // Look for the primary quote
    while(*ptr != '\0'){
        if(*ptr == '\"'){
            quote = '\"';
            break;
        }
        else if(*ptr == '\''){
            quote = '\'';
            break;
        }
        ptr++;
    }

    // If there are no quotes
    if(*ptr == '\0'){
        return str;
    }

    ptr = str;

    // Check for unmatching quotes
    while(*ptr != '\0'){
        if(*ptr == quote && !in_quotes){
            in_quotes = 1;
        }
        else if(*ptr == quote && in_quotes){
            in_quotes = 0;
        }
        ptr++;
    }

    // If there are no unmatching quotes
    // Further checking and error message exists in tokenizer
    if(!in_quotes){
        str = elim_char(str, quote);
    }

    return str;
}

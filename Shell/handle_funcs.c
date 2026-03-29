#include "handle_funcs.h"
#include "basic_funcs.h"

/* Function that sets or gets a globar variable.
   if mode == 0: set global variable -> Its arguments contains the names and the values of the global variables that will be set.
   if mode == 1: get global variable -> Its arguments contains the names of the global variables that will be get */
char **global_var_handler(char **args, int mode){

    // Create new globar vars
    if(mode == 0){

        // Checks if the parameters have symbol '='
        for(int i=0; i<numOfTokens(args); i++){
            if(!find_symbol(args[i], '=')){
                fprintf(stderr, "Command %s not found\n", args[i]);
                return NULL;
            }
        }

        // Access the arguments
        for(int i=0; i<numOfTokens(args); i++){
            char **new_args = global_vars_tokenize(args[i]);
            char quote = 0;
            char *ptr = new_args[1];

            // Check for primary quote
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

            // If quotes exist, remove them
            if(quote != 0){
                new_args[1] = elim_char(new_args[1], quote);
            }

            // Set the global var
            if(setenv(new_args[0], new_args[1], 1) < 0){
                fprintf(stderr, "Error defining global variables\n");
                exit(0);
            }
        }
    }

    // Get existing global variables
    else if(mode == 1){
        int numOfArgs = numOfTokens(args); // Number of parameters
        char *env_var; // String that stores the value of the current global variable
        char **tmp_str; // String that stores the names of the global variables after tokenization with space string (see above)
        char *final_str; // Stores the final result of replacing the variables
        char **spaces_str; // Stores the tokens separated by spaces
        char *symbol_finder; // Finds the symbol '$'

        // Access the names of global variables
        for(int i=0; i<numOfArgs; i++){

            // if the character $ is found and it is outside quotes
            if(find_symbol(args[i], '$') && is_in_quotes(args[i], '$')!='\''){

                // Remove double quotes (if exist)
                if(is_in_quotes(args[i], '$')=='\"'){
                    args[i] = elim_char(args[i], '\"');
                }

                int sumOfLetters = 0; // stores the total number of letters of the final string
                spaces_str = tokenize(args[i], " "); // tokenize the arguments with respect to space

                // Calculation of the string size of the final string
                for(int k=0; k<numOfTokens(spaces_str); k++){
                    symbol_finder = strchr(spaces_str[k], '$');
                    tmp_str = tokenize(spaces_str[k], "$"); // Tokenize with respect to '$'
                    for(int j=0; j<numOfTokens(tmp_str); j++){
                        env_var = getenv(tmp_str[j]);
                        if(env_var != NULL){
                            sumOfLetters += (strlen(env_var)+1); // If the current token was a global variable, then add the length of its value
                        }
                        else if(symbol_finder != spaces_str[k]){
                            sumOfLetters += (strlen(tmp_str[j])+1); // If the current token was not global variable, then add the length of its size
                        }
                    }
                    free(tmp_str);
                }
                final_str = malloc(sumOfLetters + 1);
                final_str[0] = '\0'; // strcat searches for a null terminator in the destination string
                for(int k=0; k<numOfTokens(spaces_str); k++){
                    symbol_finder = strchr(spaces_str[k], '$');
                    tmp_str = tokenize(spaces_str[k], "$"); // Tokenize with respect to '$'
                    for(int j=0; j<numOfTokens(tmp_str); j++){
                        env_var = getenv(tmp_str[j]);
                        if(env_var != NULL){
                            strcat(final_str, getenv(tmp_str[j])); // If the current token was a global variable, then concatenate its value in final string
                        }
                        else if(symbol_finder != spaces_str[k]){
                            strcat(final_str, tmp_str[j]); // If the current token was not a global variable, then concatenate it, in final string
                        }
                    }

                    // Add space to the final string, if multiple space tokens exist
                    if(numOfTokens(spaces_str)>1){
                        strcat(final_str, " ");
                    }
                }
                args[i] = strdup(final_str);
                free(final_str);
            }
        }
    }
    else{
        fprintf(stderr, "Global variable handler mode error\n");
        exit(0);
    }

    return args;
}

/* Receives as input the commands of each pipe and executes them */
int pipes_handler(char **args){
    int fd[2]; /* fd[0] -> read | fd[1] -> write */
    int pid;
    int numOfCommands = numOfTokens(args);
    char **command_args;
    int tmp_fd = -1; // Keeps the file descriptor of the previous command

    int stdin_fd = dup(STDIN_FILENO);
    int stdout_fd = dup(STDOUT_FILENO);

    for(int i=0; i<numOfCommands; i++){
        _pipe(fd);

        pid = _fork();

        // In child process
        if(pid == 0){

            // If there is no previous pipe
            if(tmp_fd != -1){

                // Redirects stdin to tmp_fd
                dup2(tmp_fd, STDIN_FILENO);
                close(tmp_fd);
            }

            if(i < numOfCommands-1){
                close(fd[0]);

                // Redirects stdout to fd[1]
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }

            // Check for input redirection
            if(find_symbol(args[i], '<') && !is_in_quotes(args[i], '<')){
                command_args = tokenize(args[i], "<");
                if(input_redirection_handler(command_args, 1)==-1){
                    exit(0);
                }
            }

            // Check for setting global variables
            else if(find_symbol(args[i], '=') && !is_in_quotes(args[i], '=')){
                command_args = global_vars_tokenize(args[i]);
                global_var_handler(command_args, 0);
            }

            // Check for getting global variables
            else if(find_symbol(args[i], '$')){
                command_args = global_var_handler(tokenize(args[i], " "), 1);
                if(execvp(command_args[0], command_args) < 0){
                    fprintf(stderr, "Command not found\n");
                    return -1;
                }
            }

            //Executes normal command
            else{
                command_args = tokenize(args[i], " ");
                for(int j=0; j<numOfTokens(command_args); j++){
                    command_args[j] = elim_quotes(command_args[j]);
                }
                if(execvp(command_args[0], command_args) < 0){
                    fprintf(stderr, "Command not found\n");
                    return -1;
                }
            }

        }

        // In parent process
        else{
            wait(NULL);

            if(i < numOfCommands-1){
                tmp_fd = fd[0];
                close(fd[1]);
            }
        }
    }

    if(dup2(stdin_fd, STDIN_FILENO) == -1){
        fprintf(stderr, "Syntax Error (dup2 failed)\n");
        return -1;
    }

    if(dup2(stdout_fd, STDOUT_FILENO) == -1){
        fprintf(stderr, "Syntax Error (dup2 failed)\n");
        return -1;
    }

    close(stdin_fd);
    close(stdout_fd);

    return 1;
}

/* Receives as input the commands and the file names and performs input redirection. 
Forked = 1 if there is already a child running and the function is careful not to make unnecessary forks*/
int input_redirection_handler(char **args, int forked){
    int numOfArgs = numOfTokens(args);
    int fd; // File descriptor that will represent the file to be read
    char **new_args; // Stores the new args that will be executed
    int terminal_fd; // Saves the original file descriptor of stdin to restore it


    for(int i=1; i<=numOfArgs-1; i++){
        // Open the file
        fd = open(elim_char(args[i], ' '), O_RDONLY);
        if(fd == -1){
            fprintf(stderr, "%s: No such file or directory\n", elim_char(args[i], ' '));
            return -1;
        }
        if(i<numOfArgs-1){
            close(fd);
        }
    }

    // Stores the initial stdin file descriptor
    terminal_fd = dup(STDIN_FILENO);

    new_args = tokenize(args[0], " ");

    // Redirects stdin to tmp_fd
    if(dup2(fd, STDIN_FILENO) == -1){
        fprintf(stderr, "Syntax Error (dup2 failed)\n");
        return -1;
    }

    // If already a child process exists (forked == 1)
    if(forked){
        if(execvp(new_args[0], new_args) < 0){
            fprintf(stderr, "Command not found\n");
            return -1;
        }
    }
    else{
        execute_command(new_args);
    }

    close(fd);

    // Returns stdin to its original state
    if(dup2(terminal_fd, STDIN_FILENO) == -1){
        fprintf(stderr, "Syntax Error (dup2 failed)\n");
        return -1;
    }

    close(terminal_fd);
    return 1;
}

/* Receives as input the commands and the file names and performs output redirection */
int output_redirection_handler(char **args){
    int numOfArgs = numOfTokens(args);
    int fd; // File descriptor for the output file
    int tmp_fd; // Temporary file descriptor fοr intermediate files
    char **new_args; // Contains the command to be executed
    int terminal_fd; // Stores the original file descriptor of stdin to restore it
    int dup2_res; // Stores dup2 function result

    for(int i=1; i<=numOfArgs-1; i++){
        // Eliminate spaces
        args[i] = elim_char(args[i], ' ');

        // Check for more than two redirection symbols
        if(args[i][0] == '>'){
            if(args[i][1] == '>'){
                fprintf(stderr, "Output redirection error\n");
                return -1;
            }
        }
    }

    // Handles the intermediate files
    for(int i=1; i<=numOfArgs-2; i++){
        if(args[i][0] == '>'){

            // If double redirection exists
            tmp_fd = open(elim_char(args[i], '>'), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
        }
        else{

            // If single redirection exists
            tmp_fd = open(args[i], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        }
        close(tmp_fd);
    }
    
    // Handles the last file
    if(args[numOfArgs-1][0] == '>'){

        // If double redirection exists
        fd = open(elim_char(args[numOfArgs-1], '>'), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    }
    else{

        // If single redirection exists
        fd = open(args[numOfArgs-1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    }

    // Stores the initial stdout file descriptor
    terminal_fd = dup(STDOUT_FILENO);

    // Redirects stdin to fd
    if(dup2(fd, STDOUT_FILENO) == -1){
        fprintf(stderr, "Syntax Error (dup2 failed)\n");
        return -1;
    }

    // Check for pipes
    if(find_symbol(args[0], '|') && !is_in_quotes(args[0], '|')){
        if(pipes_handler(tokenize(args[0], "|")) == -1){
            if(dup2(terminal_fd, STDOUT_FILENO) == -1){
                fprintf(stderr, "Syntax Error (dup2 failed)\n");
            }
            return -1;
        }
    }

    // Check for input redirections
    else if(find_symbol(args[0], '<') && !is_in_quotes(args[0], '<')){
        if(input_redirection_handler(tokenize(args[0], "<"), 0) == -1){
            if(dup2(terminal_fd, STDOUT_FILENO) == -1){
                fprintf(stderr, "Syntax Error (dup2 failed)\n");
            }
            return -1;
        }
    }

    // Check for setting global variables
    else if(find_symbol(args[0], '=') && !is_in_quotes(args[0], '=')){
        args = global_vars_tokenize(args[0]);
        global_var_handler(args, 0);
    }

    // Check for getting global variables
    else if(find_symbol(args[0], '$')){
        args = global_var_handler(tokenize(args[0], " "), 1);
        execute_command(args);
    }

    else{
        new_args = tokenize(args[0], " ");
        execute_command(new_args);
    }

    // Returns stdin to its original state
    if(dup2(terminal_fd, STDOUT_FILENO) == -1){
        fprintf(stderr, "Syntax Error (dup2 failed)\n");
        return -1;
    }

    close(fd);
    close(terminal_fd);

    return 1;
}

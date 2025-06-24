#include "basic_funcs.h"

char currDirectory[10000];

pid_t _fork(){
    pid_t pid;
    if((pid = fork()) < 0){
        printf("Fork error");
        exit(0);
    }

    return pid;
}

void command_prompt(){
    getcwd(currDirectory, sizeof(currDirectory));
    printf("csd4641@hy345:%s$ ", currDirectory);
}

void read_command(char *cmd_input){
    char *delim_pipe = "|";
    char *delim_input_red = "<";
    char *delim_output_red1 = ">";
    char *delim_output_red2 = ">>";
    char *delim_mult_inst = ";";
    char **args;
    char **mult_inst_args = tokenize(cmd_input, delim_mult_inst);
    char **pipes_args = tokenize(cmd_input, delim_pipe);
    
    if(numOfTokens(mult_inst_args) > 1){
        for(int i = 0; i<numOfTokens(mult_inst_args); i++){
            args = tokenize(mult_inst_args[i], " ");
            execute_command(args);

            free(args);
        }
    }
    else if(numOfTokens(pipes_args) > 1){
        pipes_handler(pipes_args);
        free(pipes_args);
    }
    else{
        args = tokenize(cmd_input, " ");
        execute_command(args);

        free(args);
    }

}

void execute_command(char **args){
    int pid;
    int status;

    if(args[0] != NULL){
        if(strcmp(args[0],"quit")==0){
            exit(0);
        }
        
        else if(strcmp(args[0],"chdir")==0){
            if (args[1] != NULL){
                if (chdir(args[1])<0) {
                    printf("Can't change directory to %s\n",args[1]);
                }
            }
        }

        else{
            pid = _fork();
            if(pid == 0){
                if(execvp(args[0],args) < 0){
                    printf("Command not found\n");
                }
            }
            else{
                wait(&status);
            }
        }
    }
}

char **tokenize(char *input, char *delim){
    char *input_ptr = strdup(input);
    int counter = 0;
    char *curr_token;
    char **tokens;

    curr_token = strtok(input_ptr, delim);

    while(curr_token){
        counter++;
        curr_token = strtok(NULL, delim);
    }

    tokens = malloc((counter+1)*sizeof(char*));

    free(input_ptr);

    input_ptr = strdup(input);

    counter = 0;
    curr_token = strtok(input_ptr, delim);

    while(curr_token){
        tokens[counter++] = strdup(curr_token);
        curr_token = strtok(NULL, delim);
    }

    tokens[counter] = NULL;

    free(input_ptr);

    return tokens;
}

size_t numOfTokens(char **args){
    size_t counter = 0;
    while (args[counter] != NULL) {
        counter++;
    }
    return counter;
}

int _pipe(int *fd){
    int res = pipe(fd);
    if(res == -1){
        printf("Problem with pipes");
        exit(0);
    }

    return res;
}
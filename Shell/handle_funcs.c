#include "handle_funcs.h"
#include "basic_funcs.h"

void pipes_handler(char **args){
    int fd[2]; /* fd[0] -> read | fd[1] -> write */
    int pid;
    int prev_fd = 0;
    int numOfCommands = numOfTokens(args);

    for(int i=0; i<numOfCommands; i++){
        if(i<numOfCommands - 1){
            _pipe(fd);
        }

        pid = _fork();

        if(pid == 0){
            if(prev_fd!=0){
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }

            if(i < numOfCommands - 1){
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }

            char **command_args = tokenize(args[i], " ");

            execvp(command_args[0], command_args);
        }
        else{
            if(i < numOfCommands-1){
                close(fd[1]);
            }

            prev_fd = fd[0];

        }
    }
    for(int i=0; i<numOfCommands; i++){
        wait(NULL);
    }
}

void input_redirection_handler(char **args){

}

void output_simple_redirection_handler(char **args){

}

void output_double_redirection_handler(char **args){

}
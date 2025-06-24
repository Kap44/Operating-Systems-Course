#include "basic_funcs.h"

#define MAXCMDINPUT 10000
char *cmd_input;

int main() { 
    while(1){
        command_prompt();
        cmd_input = malloc(MAXCMDINPUT);
		fgets(cmd_input,MAXCMDINPUT,stdin);
        if((strlen(cmd_input) > 0) && (cmd_input[strlen(cmd_input)-1] == '\n')){
            cmd_input[strlen(cmd_input)-1] = ' ';
        }
		read_command(cmd_input);
    }
    return 0; 
}
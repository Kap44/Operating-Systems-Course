#include "basic_funcs.h"

char *cmd_input; // string that stores the input of the user in command prompt
char *user; // string that stores the name of the user

int main() { 
    char *cmd_input; // string that stores the input of the user in command prompt
    char *user = getenv("LOGNAME"); // string that stores the name of the user
    while(1){
        command_prompt(user);
        cmd_input = malloc(MAXCMDINPUT);
        fgets(cmd_input,MAXCMDINPUT,stdin); // read and store the input of the user

        // replace the new line that exists in the end of the input, with null terminator
        if((strlen(cmd_input) > 0) && (cmd_input[strlen(cmd_input)-1] == '\n')){
            cmd_input[strlen(cmd_input)-1] = '\0';
        }

        // parse
        read_command(cmd_input);
    }
    return 0; 
}

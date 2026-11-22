#include "s3.h"

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///The last (previous) working directory 
    char lwd[MAX_PROMPT_LEN-6]; 

    init_lwd(lwd);///Implement this function: initializes lwd with the cwd (using getcwd)

    if(argc > 1){
        strncpy(line, argv[1], MAX_LINE-1);
        line[MAX_LINE - 1] = '\0';

        if(command_with_batch(line)){
            launch_batch_commands(line, lwd);
        }else{
            run_unit(line, lwd);
        }
        return 0;
    }


    while (1) {
        read_command_line(line, lwd);

        if(command_with_batch(line)){
            launch_batch_commands(line, lwd);
        }
        else{///Implement this function
            run_unit(line, lwd);
        }
    }
    return 0;
    
}

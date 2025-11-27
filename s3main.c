#include "s3.h"

//This is the main function that runs the shell.
//It reads the command line input, parses the command, and launches the program. 
//It also reaps the child process.
//It's in an infinite loop so it can keep reading and launching programs.
/*
int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///Stores pointers to command arguments.
    ///The first element of the array is the command name.
    char *args[MAX_ARGS];

    ///Stores the number of arguments
    int argsc;

    while (1) { //Edit the main loop to handle redirection and pipes
        read_command_line(line);

        if (command_with_pipes(line)) {
            char *pipeline_cmds[MAX_ARGS];
            int pipeline_count = 0;

            if (tokenize_pipeline(line, pipeline_cmds, &pipeline_count)) { //Split the line into separate command strings so that each command is parsed
                launch_pipeline(pipeline_cmds, pipeline_count); //Handles launching each command
            } else {
                fprintf(stderr, "Pipeline parse error\n"); //Error handling
            }

            reap();

        } else if (command_with_redirection(line)) { //Inspect the raw line for '>', '>>' or '<', handle it with separate functions
            parse_command(line, args, &argsc);
            launch_program_with_redirection(args, argsc);
            reap();
        } else { //Else, just launch as normal
            parse_command(line, args, &argsc);
            launch_program(args, argsc);
            reap();
        }
    }

    return 0;
    
}
*/


int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///The last (previous) working directory 
    char lwd[MAX_PROMPT_LEN-6]; 

    init_lwd(lwd);///Implement this function: initializes lwd with the cwd (using getcwd)

    //Stores pointers to command arguments.
    ///The first element of the array is the command name.
    char *args[MAX_ARGS];

    ///Stores the number of arguments
    int argsc;

    while (1) {

        read_command_line(line, lwd); ///Notice the additional parameter (required for prompt construction)

        if(is_cd(line)){///Implement this function
            parse_command(line, args, &argsc);
            run_cd(args, argsc, lwd);
        }
        else if(command_with_batch(line)){
            char *batch_cmds[MAX_ARGS];
            int batch_count = 0;

            if (tokenize_batched_commands(line, batch_cmds, &batch_count)) {
                launch_batched_commands(batch_cmds, batch_count, lwd);
            } else {
                fprintf(stderr, "Batch parse error\n");
            }
        }
        else if(command_with_pipes(line)){
            char *pipeline_cmds[MAX_ARGS];
            int pipeline_count = 0;

            if (tokenize_pipeline(line, pipeline_cmds, &pipeline_count)) {
                launch_pipeline(pipeline_cmds, pipeline_count);
            } else {
                fprintf(stderr, "Pipeline parse error\n");
            }

            reap();
        }
        else if(command_with_redirection(line)){
            ///Command with redirection
           parse_command(line, args, &argsc);
           launch_program_with_redirection(args, argsc);
           reap();
       }
       else ///Basic command
       {
           parse_command(line, args, &argsc);
           launch_program(args, argsc);
           reap();
       }
    }

    return 0;
}

#include "s3.h"

int main(int argc, char *argv[]){
    ///Stores the command line input
    char line[MAX_LINE];

    ///The last (previous) working directory 
    ///We set it to -6 to account for "[s3]$ " <- 6 characters which is defined in s3.c
    ///Shell prompt is constructed in construct_shell_prompt() function
    char lwd[MAX_PROMPT_LEN-6]; 

    init_lwd(lwd);///Implement this function: initializes lwd with the cwd (using getcwd)

    //Stores pointers to command arguments.
    ///The first element of the array is the command name.
    char *args[MAX_ARGS];

    ///Stores the number of arguments
    int argsc;

    //If shell is invoked with arguments, execute them as commands (for subshell execution)
    if (argc > 1) {
        //Shell was invoked as: ./s3 "cd txt ; ls"
        //argv[1] contains the command string to execute
        strncpy(line, argv[1], MAX_LINE - 1);
        line[MAX_LINE - 1] = '\0';
        
        //Process the command using existing logic (order must match main loop)
        if (command_with_batch(line)) {
            char *batch_cmds[MAX_ARGS];
            int batch_count = 0;
            if (tokenize_batched_commands(line, batch_cmds, &batch_count)) {
                launch_batched_commands(batch_cmds, batch_count, lwd);
            }
        }
        else if (is_cd(line)) {
            parse_command(line, args, &argsc);
            run_cd(args, argsc, lwd);
        }
        else if (command_with_pipes(line)) {
            char *pipeline_cmds[MAX_ARGS];
            int pipeline_count = 0;
            if (tokenize_pipeline(line, pipeline_cmds, &pipeline_count)) {
                launch_pipeline(pipeline_cmds, pipeline_count);
            }
            // reap() is now called inside launch_pipeline() for all children
        }
        else if (command_with_redirection(line)) {
            parse_command(line, args, &argsc);
            launch_program_with_redirection(args, argsc);
            reap();
            
        }
        //Checks for subshell and runs subshell. Allows for implementation of nested subshell
        //Allows for commands like ((((echo "HI"))))
        else if (command_with_subshell(line)) { 
            char subshell_cmd[MAX_LINE];
            //extracts subshell and laucnhes subshell
            if (extract_subshell_commands(line, subshell_cmd)){
                launch_subshell(subshell_cmd);
            }
            reap();

        }
        else {
            parse_command(line, args, &argsc);
            launch_program(args, argsc);
            reap();
        }
        
        return 0; //Exit after executing the command (subshells are one-shot)
    }

    //Normal interactive shell mode
    while (1) {

        read_command_line(line, lwd); ///Notice the additional parameter (required for prompt construction)

        if(command_with_batch(line)){
            char *batch_cmds[MAX_ARGS];
            int batch_count = 0;

            if (tokenize_batched_commands(line, batch_cmds, &batch_count)) {
                launch_batched_commands(batch_cmds, batch_count, lwd);
            } else {
                fprintf(stderr, "Batch parse error\n");
            }
        }
        else if(is_cd(line)){///Implement this function
            parse_command(line, args, &argsc);
            run_cd(args, argsc, lwd);
        }
        else if(command_with_pipes(line)){
            char *pipeline_cmds[MAX_ARGS];
            int pipeline_count = 0;

            if (tokenize_pipeline(line, pipeline_cmds, &pipeline_count)) {
                launch_pipeline(pipeline_cmds, pipeline_count);
            } else {
                fprintf(stderr, "Pipeline parse error\n");
            }

            // reap() is now called inside launch_pipeline() for all children
        }
        else if(command_with_redirection(line)){
            ///Command with redirection
           parse_command(line, args, &argsc);
           launch_program_with_redirection(args, argsc);
           reap();
        } 
       else if(command_with_subshell(line)){
            char subshell_cmd[MAX_LINE];

            if (extract_subshell_commands(line, subshell_cmd)){
                launch_subshell(subshell_cmd);
            } else {
                fprintf(stderr, "Subshell command syntax error\n");
            }
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

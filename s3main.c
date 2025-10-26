#include "s3.h"

//This is the main function that runs the shell.
//It reads the command line input, parses the command, and launches the program. 
//It also reaps the child process.
//It's in an infinite loop so it can keep reading and launching programs.

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///Stores pointers to command arguments.
    ///The first element of the array is the command name.
    char *args[MAX_ARGS];

    ///Stores the number of arguments
    int argsc;

    while (1) {
        read_command_line(line);

        parse_command(line, args, &argsc);

        launch_program(args, argsc); 

        reap();
    }

    return 0;
    
}

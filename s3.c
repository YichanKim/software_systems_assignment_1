#include "s3.h"

//This file contains the functions that are used in the shell. 

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    strcpy(shell_prompt, "[s3]$ ");
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    ///See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL)
    {
        perror("fgets failed");
        exit(1);
    }
    ///Remove newline (enter)
    line[strlen(line) - 1] = '\0';
}

void parse_command(char line[], char *args[], int *argsc)
{
    ///Implements simple tokenization (space delimited)
    ///Note: strtok puts '\0' (null) characters within the existing storage, 
    ///to split it into logical cstrings.
    ///There is no dynamic allocation.

    ///See the man page of strtok(...)
    char *token = strtok(line, " ");
    *argsc = 0;
    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        args[(*argsc)++] = token;
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL; ///args must be null terminated
}

///Launch related functions
void child(char *args[], int argsc)
{
    ///Use execvp to load the binary 
    ///of the command specified in args[ARG_PROGNAME].
    if (execvp(args[ARG_PROGNAME], args) == -1) { //execvp() finds and runs the program, returns -1 if it fails
        perror("execvp failed");
        exit(1);
    }
}

//Main shell calls the launch program
//Checks for exit, else continues
//Forks child process, get's execvp to run the command
//Reaps the child process
void launch_program(char *args[], int argsc)
{
    //Handle the special "exit" command - shell should exit, not the child
    if (args[0] != NULL && strcmp(args[0], "exit") == 0) {
        exit(0);
    }

    //Create a child process
    pid_t pid = fork();

    if (pid == 0) { 
        //This code runs the CHILD process
        child(args, argsc);
    } else if(pid > 0) {
        //This code runs the PARENT process
        //Do nothing - let reap() handle the waiting
    } else {
        //fork() failed
        perror("fork failed");
    }
}

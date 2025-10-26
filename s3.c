#include "s3.h"

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

/**
 * child
 * 
 * Use execvp to load the binary of the command specified in args[ARG_PROGNAME].
 * 
 * Reference: Lecture 3
 * 
 * args = array of strings. args contains the command and its arguments
 * argsc = number of strings in args
 * 
 * We do not need to worry about empty args because it is taken care of at launch_program
 */
void child(char *args[], int argsc)
{
    //transforms the current process into a new program
    execvp(args[0], args);

    //if execvp is successful, we will not get here so we can do error handling here
    fprintf(stderr, "Incorrect args\n");
    exit(1);


}


/**
 * launch_program
 * 
 * fork() a child process.
 * In the child part of the code, call child(args, argv)
 *
 * Reference: Lecture 2 
 *  
 * args = array of strings. args contains the command and its arguments
 * argsc = number of strings in args
 * 
 * Edge case to handle: 
 *  1) Empty args **MUST BE CONSIDERED**
 *  2) "exit" command: shell (not the child) should exit
 *
 */
void launch_program(char *args[], int argsc)
{

    if (strcmp(args[0], "exit")==0){
        exit(0); //success status code
    }
    int rc = fork();
    if (rc < 0) { //fork failed, exit
        fprintf(stderr, "fork failed\n");
        exit(1); // error status code
    } else if (rc==0) { //child process, run child
        child(args, argsc);
    } else { //parent node
        int wc = wait(NULL); //wait for child process to finish    
    }
    return;


}
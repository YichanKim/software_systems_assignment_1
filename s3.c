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
    execvp(args[ARG_PROGNAME], args);

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

    if (args[0] != NULL && strcmp(args[0], "exit") == 0){
        exit(0); //success status code
    }
    int pid = fork();
    if (pid < 0) { //fork failed, exit
        fprintf(stderr, "fork failed\n");
        exit(1); // error status code
    } else if (pid==0) { //child process, run child
        child(args, argsc);
    } else { //parent node
        //Do nothing, wait for reap() to handle the waiting   
    }
}

int command_with_redirection(char line[])
{
    for (size_t i = 0; line[i] != '\0'; i++) {
        if (line[i] == '>' ||  line[i] == '<')  //We only want to parse the detailed redirection info when the operators appear
            return 1;
    }
    return 0;
}

//This returns the index of the operator if found, -1 otherwise.
//It sets the file, appends, input depends on the operator type.
//The key idea of this function is to record the file to use, whether it's append mode, and whether it's input redirection.
int find_redirection(char *tokens[], int count, char **file, int *append, int *input)
{
    for (int i = 0; i < count; i++) {
        if (strcmp(tokens[i], ">") == 0) { //String compare
            *file = tokens[i + 1];
            *append = 0;
            *input = 0;
            return i;
        } else if (strcmp(tokens[i], ">>") == 0) {
            *file = tokens[i + 1];
            *append = 1;
            *input = 0;
            return i;
        } else if (strcmp(tokens[i], "<") == 0) {
            *file = tokens[i + 1];
            *append = 0;
            *input = 1;
            return i;
        }
    }
    return -1;
}

//Child helper function for redirection - The child process needs to rewire its stdin/stdout before calling execvp.
static void child_with_redirection(char *args[], int argsc, char *filename, int append, int input)
{
    int fd; //file descriptor
    if (input) {
        fd = open(filename, O_RDONLY);
        if (fd == -1) {
            perror("open failed");
            exit(1);
        }
        if (dup2(fd, STDIN_FILENO) == -1) {
            perror("dup2 failed");
            close(fd);
            exit(1);
        }
    } else {
        int flags = O_WRONLY | O_CREAT;
        flags |= append ? O_APPEND : O_TRUNC;
        fd = open(filename, flags, 0644);
        if (fd == -1) {
            perror("open failed");
            exit(1);
        }
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2 failed");
            close(fd);
            exit(1);
        }
    }
    close(fd);

    if (execvp(args[ARG_PROGNAME], args) == -1) {
        perror("execvp failed");
        exit(1);
    }
}

//This is the launch program function with redirection support.
//We NUL-out the redirection tokens so that execvp sees only the real command arguments.
//The child uses the helper function above to perform the actual redirection (child_with_redirection).
//Parent path mirrors the basic launch_program logic.
void launch_program_with_redirection(char *args[], int argsc)
{
    char *filename = NULL;
    int append = 0;
    int input = 0;

    int idx = find_redirection(args, argsc, &filename, &append, &input);
    if (idx == -1 || filename == NULL) {
        fprintf(stderr, "Redirection syntax error\n");
        return;
    }

    args[idx] = NULL;
    if (idx + 1 < argsc) 
        args[idx + 1] = NULL;
    
    pid_t pid = fork();

    if (pid == 0) {
        child_with_redirection(args, argsc, filename, append, input);
    } else if (pid > 0) {
        return; //Parent waits using reap()
    } else {
        perror("fork failed");
    }
}
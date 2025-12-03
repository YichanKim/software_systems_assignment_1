#include <ctype.h>
#include "s3.h"

//This file contains the functions that are used in the shell. 


//Helper function to do trimming
char* trim(char *str){
    if (!str){
        return str;
    }

    //Move pointer forward if it is just a space
     while (*str == ' '){
        str++;
    }

    char *end = str + strlen(str);
        while (end > str && *(end - 1) == ' ') {
            end--;
        }
    *end = '\0'; // Null-terminate at the trimmed position
    
    return str;
}


///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[], char lwd[])
{
    char cwd[MAX_PROMPT_LEN];

    if (getcwd(cwd, sizeof(cwd)) != NULL){
        strcpy(shell_prompt, "[");
        strcat(shell_prompt, cwd);
        strcat(shell_prompt, " s3]$ ");
    } else{
        strcpy(shell_prompt, "[s3]$ ");
    }
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[], char lwd[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt, lwd);
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
        //add tokenised command to args while incrementing argsc
        args[(*argsc)++] = token;
        //find next string seperated by " "
        token = strtok(NULL, " ");

        if (token == NULL){
            break;
        }
        // Now before we add the token to the new line, we must remove surrounding quotes from the token
        // This accounts for edge cases like echo "Hi" where it would output "Hi" instead of Hi  

        size_t len = strlen(token);

        //compare with 2 as quote removal requires at least two characters
        if (len >= 2) {

            //flag to check if it has double quotes on both lines
            int has_double_quotes = (token[0] == '"' && token[len-1] == '"');

            int has_single_quotes = (token[0] == '\'' && token[len-1] == '\'');

            if (has_double_quotes || has_single_quotes){
                //shift the token by 1 to the left
                // null terminate the token
                memmove(token, token + 1, len - 2);
                token[len - 2] = '\0';
            }
        }
        
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
 * is_cd
 * 
 * checks if command is a cd command
 * returns 1 if true, 0 otherwise
 */
int is_cd(const char *line) {
    //input to is_cd is a null pointer, return 0
    if (!line){
        return 0;
    }

    //if character in line exists ('\0' will output false) & character in line is space
    //move pointer to next character
    while (*line && isspace((unsigned char) *line)) { //Essentially scrolls through white spcae
        line++;
    }
    
    //line[0] is a valid check as line[0] = '\0' for edge case of just spaces
    if (line[0] == 'c' && line[1] == 'd' &&
        (line[2] == '\0' || isspace((unsigned char)line[2]))) { //prevents lines like cdfoo
        return 1;
    }
    return 0;
}

/**
 * init_lwd
 * 
 * initialises a buffer to store previous directory
 * sets lwd[0] to '\0' which signals that there is no previous directory at init
 */
void init_lwd(char lwd[]){
    char cwd[MAX_PROMPT_LEN];

    size_t max_available_size = MAX_PROMPT_LEN - 6;

    if (getcwd(cwd, sizeof(cwd)) != NULL){
        if(strlen(cwd) >= max_available_size){
            lwd[0] = '\0';
        } else {
            //copies until we find '\0' and copies '\0' too
            strcpy(lwd, cwd);
        }
    } else{
        lwd[0] = '\0'; //no prev directory
    }
}

/**
 * run_cd
 * 
 * Uses chdir() to change present working directory of the process
 *  
 * args = array of strings. args contains the command and its arguments
 * argsc = number of strings in args
 * lwd[] = array to store previous working directory
 * 
 * Edge case to handle: 
 *  1) cd -: goes to previous directory
 *  2) cd ..: goes to the directory above the current directory
 *  3) cd : goes to home directory
 *  4) cd a b: sends error (too many arguments)
 *  5) cd non/existent/path/ : sends error (invalid file)
 */
int run_cd(char *args[], int argsc, char lwd[]) {
    if (argsc > 2){ //too many arguments, throw cd error
        fprintf(stderr, "cd error, too many arguments\n");
        return -1;
    }
    
    const char *path = NULL;
    
    if (argsc > 1 && args[1]){
        if (strcmp(args[1], "-") == 0) { //if cd path is "-", go to previous directory
            if (!lwd || lwd[0] == '\0') { //lwd does not exist
                fprintf(stderr, "cd, no prev directory\n");
                return -1;
            }
            path = lwd;
        } else { //else go to specified path
            path = args[1];
        }
    } else { //if cd path is blank, go to home file directory
        path = getenv("HOME");
    }

    //path validation
    if (!path || path[0] == '\0') {
        fprintf(stderr, "cd error, path not set\n");
        return -1;
    }

    char *oldpwd = getcwd(NULL, 0);

    //Change directory to path with chdir
    if (chdir(path) != 0) { //chdir returns -1 on failure
        fprintf(stderr, "chdir error\n");
        free(oldpwd);
        return -1;
    }

    //set new lwd with oldpwd
    //to manage working directories to allow for "cd -"
    if (oldpwd){
        size_t max_available_size = MAX_PROMPT_LEN - 6;
        if(strlen(oldpwd) >= max_available_size){
            lwd[0] = '\0';
        } else {
            //copies until we find '\0' and copies '\0' too
            strcpy(lwd, oldpwd);
        }
    } else {
        lwd[0] = '\0';
    }

    free(oldpwd);
    return 0;
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
        if (line[i] == '>' ||  line[i] == '<')  //We only want to parse the detailed redirection info when the operators appear, we return 1 for the main loop if we find redirection
            return 1;
    }
    return 0;
}

//Use this helper function to check if the command contains a pipe (used in the main loop)
int command_with_pipes(char line[])
{
    for (size_t i = 0; line[i] != '\0'; i++) {
        if (line[i] == '|')
            return 1;
    }
    return 0;
}

// This function checks if the command contains a semicolon (.i.e if it is a batched command)
int command_with_batch(char line[])
{
    for (size_t i = 0; line[i] != '\0'; i++) {
        if (line[i] == ';') {
            return 1;
        }
    }
    return 0;
}

//Detects if the command line contains parentheses (i.e if it is a subshell command)
int command_with_subshell(char line[])
{
    for (size_t i = 0; line[i] != '\0'; i++) { // If we find a '(' or ')', return 1, as it contains a subshell command
        if (line[i] == '(' || line[i] == ')')
            return 1;
    }
    return 0; // Doesn't contain a subshell command
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


/**
 * tokenize_pipeline
 * 
 * Splits the raw line into individual commands separated by '|'
 * Ignores all '|' inside parentheses
 * If we find a pipeline character, append the command before the pipeline character to commands and increment command_count
 * 
 * Returns 1 if successful, and 0 if we have a parse failure
 * 
 * Input:
 * line[] =  input string
 * Output:
 * *commands[] = An array of pointers, each pointing to the start of each command in line[]
 * *command_count = number of commands
 * 
 */
int tokenize_pipeline(char line[], char *commands[], int *command_count) 
{
    *command_count = 0;

    int paren_depth = 0;

    char *begin = line;
    size_t len = strlen(line);

    for (size_t index = 0; index<= len; index++) {
        char ch = line[index];

        if (ch == '('){
            paren_depth++;
        } else if (ch == ')' && paren_depth > 0){
            paren_depth--;
        } else if (ch == ')' && paren_depth == 0){
            fprintf(stderr, "Unbalanced parentheses");
            return 0;
        }

        if ((ch == '|' && paren_depth == 0) || ch == '\0') {
            line[index] = '\0';

            char *token = begin;

            trim(token);

            // Add command if not empty
            if (*token == '\0') {
                if (ch == '\0'){
                    // Result of using '\0' as null seperator
                    // is that it will also look at the end of line null terminator.
                    // As a result, we just ignore it with break
                    break;
                } else {
                    fprintf(stderr, "Empty command in pipeline\n");
                    return 0;
                }
            } else { //Check if token is not empty. If not empty, and we did not exceed max args, append to commands
                if (*command_count >= MAX_ARGS-1){
                    fprintf(stderr, "Too many pipeline commands\n");
                    return 0;
                }
                commands[(*command_count)++] = token;
            }
            begin = line + index + 1;
        }
    }

    if (paren_depth != 0){
        fprintf(stderr, "Unbalanced parentheses");
        return 0;
    }

    //null terminate command
    commands[*command_count] = NULL;

    return *command_count > 0;
}

/**
 * tokenize_batched_commands
 * 
 * Splits the raw line into individual commands separated by semicolons
 * Ignores all ';' inside parentheses
 * If we find a batched character, append the command before the batched character to commands and increment command_count
 * 
 * ALMOST THE SAME AS TOKENIZE_PIPELINE_COMMANDS except for the chracter we are looking for - quite obviously.
 * 
 * Returns 1 if successful, and 0 if we have a parse failure
 * 
 * Input:
 * line[] =  input string
 * Output:
 * *commands[] = An array of pointers, each pointing to the start of each command in line[]
 * *command_count = number of commands
 */
int tokenize_batched_commands(char line[], char *commands[], int * command_count)
{
    *command_count = 0;

    int paren_depth = 0;

    char *begin = line; //beginnning of current command
    size_t len = strlen(line);

    for (size_t index = 0; index<= len; index++) {
        char ch = line[index];

        if (ch == '('){
            paren_depth++;
        } else if (ch == ')'){
            if (paren_depth == 0){
                fprintf(stderr, "Unmatched closing parentheses\n");
                return 0;
            }
            paren_depth--;
        }

        if ((ch == ';' && paren_depth == 0) || ch == '\0') {
            line[index] = '\0';

            char *token = begin;
            
            //trim token
            trim(token);

            // Add command if not empty
            if (*token != '\0') {
                if (*command_count >= MAX_ARGS-1){
                    fprintf(stderr, "Too many batched commands");
                    return 0;
                }
                commands[(*command_count)++] = token;
            }
            begin = line + index + 1; //start token on next character after the inputted token
        }
    }
    if (paren_depth != 0){ //edgecase, unbalanced parantehses
        fprintf(stderr, "Unbalanced parentheses");
        return 0;
    }
    
    //null terminate command <- not my code again
    commands[*command_count] = NULL;

    return *command_count > 0;
}

/**
 * extract_subshell_commands
 * 
 * Divides line by subshell commands. Appends to *subshell_cmd
 * 
 * Doesn't need to account for batched commands as it is already accounted for in s3main.c
 * 
 * Returns 1 if successful, and 0 if we have a failure
 * 
 * Input:
 * line[] =  input string
 * Output:
 * *subshell_command = pointer to start of command in the subshell
 * 
 * Edge case to handle: 
 *  1) Trailing + Leading spaces
 *  2) Incorrect parentheses depth (incorrect pairings of opening and closing parentheses)
 *  3) Null terminator in EOL
 */
int extract_subshell_commands(char line[], char *subshell_command) 
{
    int paren_depth = 0; //tracks depth of parentheses. 0 is top-level
    int top_open_paren = -1; //first index of open parentheses '('
    int top_close_paren = -1; //index of top-level closing parentheses ')'

    //find and input index for first opening and closing parentheses (top level)
    for (size_t index = 0; line[index] != '\0'; index++) { //index to scroll through string

        if (line[index] == '(') {
            // If paren_depth is 0 (top-level): Set top_open_paren to the index. else, skip
            // After checking for paren_depth, increment paren_depth by 1.
            if (paren_depth == 0){
                top_open_paren = index;
            }
            paren_depth++;

        } else if (line[index] == ')'){
            // If paren_depth is 0 (top-level): Return 0 (error)
            // Decrement paren_depth
            // If paren_depth is now 0, Set top_close_paren to the index. else, skip
            if (paren_depth == 0){
                //unbalanced set of parentheses
                return 0;
            }

            paren_depth--;

            if (paren_depth == 0){
                top_close_paren = index;
                break;
            }
        }
    }

    //Make sure there are open/close paren pair, or we dont have something like )echo xx(
    if (top_open_paren == -1 || top_close_paren == -1 || top_close_paren <= top_open_paren) {
        return 0;
    }

    size_t len = top_close_paren - top_open_paren - 1; // -1 to account for the double counting of the parentheses

    //size bound may be neccessary
    strncpy(subshell_command, line + top_open_paren + 1, len); // Copy the command string inside the parentheses
    subshell_command[len] = '\0'; // Null-terminate the string

    // Trim string
    trim(subshell_command);
    return strlen(subshell_command) > 0; // Returns 1 if successful, 0 if failure
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

//Launches a child command within a pipe.
//read_fd: fd to be duped onto stdin (or -1 if no pipe input)
//write_fd: fd to be duped onto stdout (or -1 if no pipe output)
static void child_with_pipes(char *args[], int argsc, int read_fd, int write_fd)
{
    if (read_fd != - 1) {
        if (dup2(read_fd, STDIN_FILENO) == -1) {
            perror("dup2 failed");
            exit(1);
        }
        close(read_fd);
    }

    if (write_fd != -1) {
        if (dup2(write_fd, STDOUT_FILENO) == -1) {
            perror("dup2 failed");
            exit(1);
        }
        close(write_fd);
    }

    execvp(args[ARG_PROGNAME], args);
    perror("execvp failed");
    exit(1);
}

//Launches a child command with both pipe input and file output redirection
//read_fd: fd to be duped onto stdin (or -1 if no pipe input)
//filename: file to redirect stdout to
//append: 1 for append mode (>>), 0 for truncate mode (>)
static void child_with_pipes_and_redirection(char *args[], int argsc, int read_fd, char *filename, int append)
{
    //Handle pipe input
    if (read_fd != -1) {
        if (dup2(read_fd, STDIN_FILENO) == -1) {
            perror("dup2 failed");
            exit(1);
        }
        close(read_fd);
    }

    //Handle file output redirection
    int flags = O_WRONLY | O_CREAT;
    flags |= append ? O_APPEND : O_TRUNC;
    int fd = open(filename, flags, 0644);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }
    if (dup2(fd, STDOUT_FILENO) == -1) {
        perror("dup2 failed");
        close(fd);
        exit(1);
    }
    close(fd);

    execvp(args[ARG_PROGNAME], args);
    perror("execvp failed");
    exit(1);
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

//Launches a pipeline of commands
void launch_pipeline(char *commands[], int command_count)
{
    int prev_read_fd = -1;

    for (int i = 0; i < command_count; i++) {
        char *args[MAX_ARGS];
        int argsc = 0;

        parse_command(commands[i], args, &argsc);

        if (argsc == 0) {
            fprintf(stderr, "Empty command in pipeline\n");
            if (prev_read_fd != -1) close(prev_read_fd);
            return;
        }

        //Check if this command is a subshell
        if (command_with_subshell(commands[i])) {
            char subshell_cmd[MAX_LINE];
            if (extract_subshell_commands(commands[i], subshell_cmd)) {
                //Subshell in pipeline - need special handling with I/O redirection
                int pipe_fds[2] = {-1, -1};
                if (i < command_count - 1) {
                    if (pipe(pipe_fds) == -1) {
                        perror("pipe failed");
                        if (prev_read_fd != -1) close(prev_read_fd);
                        return;
                    }
                }
                
                pid_t pid = fork();
                if (pid == -1) {
                    perror("fork failed");
                    if (prev_read_fd != -1) close(prev_read_fd);
                    if (pipe_fds[0] != -1) close(pipe_fds[0]);
                    if (pipe_fds[1] != -1) close(pipe_fds[1]);
                    return;
                }
                
                if (pid == 0) {
                    //Child: redirect stdin/stdout for pipeline
                    if (prev_read_fd != -1) {
                        if (dup2(prev_read_fd, STDIN_FILENO) == -1) {
                            perror("dup2 failed");
                            exit(1);
                        }
                        close(prev_read_fd);
                    }
                    if (pipe_fds[1] != -1) {
                        if (dup2(pipe_fds[1], STDOUT_FILENO) == -1) {
                            perror("dup2 failed");
                            exit(1);
                        }
                        close(pipe_fds[1]);
                    }
                    if (pipe_fds[0] != -1) close(pipe_fds[0]);
                    
                    //Launch subshell with redirected I/O
                    char *subshell_args[3];
                    subshell_args[0] = "./s3";
                    subshell_args[1] = subshell_cmd;
                    subshell_args[2] = NULL;
                    execvp("./s3", subshell_args);
                    perror("execvp failed");
                    exit(1);
                } else {
                    //Parent: close fds and continue
                    if (prev_read_fd != -1) close(prev_read_fd);
                    if (pipe_fds[1] != -1) close(pipe_fds[1]);
                    prev_read_fd = pipe_fds[0];
                }
                continue; //Skip normal command processing
            }
        }

        //Normal command processing (not a subshell)
        int pipe_fds[2] = {-1, -1};
        if (i < command_count - 1) {
            if (pipe(pipe_fds) == -1) {
                perror("pipe failed");
                if(prev_read_fd != -1) close(prev_read_fd);
                return;
            }
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            if (prev_read_fd != -1) close(prev_read_fd);
            if (pipe_fds[0] != -1) close(pipe_fds[0]);
            if (pipe_fds[1] != -1) close(pipe_fds[1]);
            return;
        }

        if (pid == 0) {
            int read_fd = prev_read_fd;
            int write_fd = (i < command_count - 1) ? pipe_fds[1] : -1;

            if (pipe_fds[0] != -1) 
                close(pipe_fds[0]); //Child doesn't need read end yet

            //Check if last command has output redirection
            if (i == command_count - 1) {
                char *filename = NULL;
                int append = 0;
                int input = 0;
                int redir_idx = find_redirection(args, argsc, &filename, &append, &input);

                if (redir_idx != -1 && !input && filename != NULL) {
                    //Last command has output redirection - strip operators and use combined helper
                    args[redir_idx] = NULL;
                    if (redir_idx + 1 < argsc)
                        args[redir_idx + 1] = NULL;
                    child_with_pipes_and_redirection(args, argsc, read_fd, filename, append);
                } else {
                    //Normal pipe handling (no redirection or input redirection)
                    child_with_pipes(args, argsc, read_fd, write_fd);
                }
            } else {
                //Not the last command - normal pipe handling
                child_with_pipes(args, argsc, read_fd, write_fd);
            }
        } else {
            if (prev_read_fd != -1)
                close(prev_read_fd);

            if (pipe_fds[1] != -1)
                close(pipe_fds[1]);

            prev_read_fd = pipe_fds[0];
        }
    }

    if (prev_read_fd != -1)
        close(prev_read_fd);

    //Parent returns; reap() in main waits for all children
}

// Executes a batch of commands sequentially, regardless of success/failure
// Each command can be basic, have redirection, use pipes, or be a cd command
void launch_batched_commands(char *commands[], int command_count, char lwd[])
{
    for (int i = 0; i < command_count; i++) {
        char * args[MAX_ARGS];
        int argsc = 0;

        // Check for cd command first (must run in parent process)
        if (is_cd(commands[i])) {
            parse_command(commands[i], args, &argsc);
            run_cd(args, argsc, lwd);
            continue; // cd doesn't need reap()
        }

        // Check for subshell
        if (command_with_subshell(commands[i])) {
            char subshell_cmd[MAX_LINE];
            if (extract_subshell_commands(commands[i], subshell_cmd)) {
                char *batch_commands[MAX_ARGS];
                int batch_count = 0;
                if (tokenize_batched_commands(subshell_cmd, batch_commands, &batch_count)){
                    pid_t pid = fork();
                    if (pid == 0){//child process
                        launch_batched_commands(batch_commands, batch_count,lwd);
                        exit(0);
                    } else if (pid > 0) {//parent
                        reap();
                    } else{
                        fprintf(stderr, "fork failed in batched commands - command with subshell");
                    }
                }
            } else {
                fprintf(stderr, "Subshell command syntax error\n");
            }
            continue;
        }   

        // Check command type before parsing (parsing modifies the string)
        if (command_with_pipes(commands[i])) {
            char * pipeline_cmds[MAX_ARGS];
            int pipeline_count = 0;

            if (tokenize_pipeline(commands[i], pipeline_cmds, &pipeline_count)) {
                launch_pipeline(pipeline_cmds, pipeline_count);
            } else {
                fprintf(stderr, "Pipeline parse error\n");
            }
            reap(); 
        }
        else if (command_with_redirection(commands[i])) {
            parse_command(commands[i], args, &argsc);
            if (argsc == 0) {
                continue;
            }
            launch_program_with_redirection(args, argsc);
            reap();
        } else {
            parse_command(commands[i], args, &argsc);
            if (argsc == 0) {
                continue;
            }
            launch_program(args, argsc);
            reap();
        }
    }
}

//Launches a subshell by forking and executing the shell binary with the subshell commands
void launch_subshell(char *subshell_cmd)
{
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        return;
    }

    if (pid == 0) { // Child process: execute the shell with the subshell command
        char *subshell_args[3];
        subshell_args[0] = "./s3"; // Path to shell binary
        subshell_args[1] = subshell_cmd; // Command to execute
        subshell_args[2] = NULL; // Null terminator

        execvp("./s3", subshell_args);
        perror("execvp failed"); // If you reach here, execvp failed
        exit(1);
        
    }
    // Parent process: wait for the subshell to complete
    // reap() will be called in the main loop
}
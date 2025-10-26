# S3 Shell Implementation Roadmap

This roadmap outlines the step-by-step implementation of the s3 (software-systems-shell) in C, covering the core features needed to build a functional shell.

## Overview
The shell will be built incrementally, with each section adding new functionality while building upon previous implementations. Test thoroughly after each section before proceeding to the next.

---

## Section 1: Basic Commands

### Objective
Implement basic command execution without redirection or pipes.

### Functions to Implement
1. **`launch_program(char *args[], int argsc)`**
   - Use `fork()` to create a child process
   - Handle the 'exit' command in the parent process (shell should exit, not child)
   - Call `child()` function from within the child process

2. **`child(char *args[], int argsc)`**
   - Use `execvp()` to replace process image with the specified command
   - Load binary from `args[ARG_PROGNAME]`

### Key Considerations
- Review `fork`, `execvp`, and `wait` system calls
- Handle error cases appropriately
- Ensure proper process management

### Testing Commands

| Command | Description |
|---------|-------------|
| `whoami` | Shows your current username |
| `pwd` | Prints the current working directory |
| `ls` | Lists files and folders in the current directory |
| `ls -R` | Recursively lists files in all subdirectories |
| `cal` | Displays the current month's calendar |
| `cal -y` | Shows the full calendar for the current year |
| `clear` | Clears the terminal screen |
| `man cat` | Opens the manual page for the cat command |
| `cat txt/phrases.txt` | Displays the contents of phrases.txt |
| `uniq txt/phrases.txt` | Removes duplicate lines (assumes sorted input) |
| `wc txt/phrases.txt` | Counts lines, words, and characters in the file |
| `sort txt/phrases.txt` | Sorts the lines in the file alphabetically |
| `grep burn txt/phrases.txt` | Finds lines containing the string "burn" |
| `grep -n burn txt/phrases.txt` | Finds "burn" and shows matching line numbers |
| `cp txt/phrases.txt txt/phrases_copy.txt` | Copies the file to a new file |
| `touch txt/new_file.txt` | Creates an empty file named new_file.txt |
| `chmod a-w txt/phrases_copy.txt` | Makes the file read-only for all users |
| `ls -l txt/phrases_copy.txt` | Shows file details like permissions and size |
| `less txt/phrases.txt` | Opens file in scrollable view (press q to quit) |
| `exit` | Exits the current shell session |

---

## Section 2: Commands with Redirection

### Objective
Implement input (`<`) and output (`>`, `>>`) redirection operators.

### Functions to Implement
1. **`command_with_redirection(char line[])`**
   - Detect if command contains redirection operators
   - Return boolean indicating presence of redirection

2. **`launch_program_with_redirection(char *args[], int argsc)`**
   - Fork child process for commands with redirection
   - Call appropriate child redirection functions

3. **Helper Functions** (recommended)
   - `child_with_output_redirected()` - handles `>` and `>>`
   - `child_with_input_redirected()` - handles `<`
   - Parsing functions to locate redirection operators

### Key Considerations
- Use `dup2()` system call for file descriptor redirection
- Use `open()` with appropriate flags (`O_RDONLY`, `O_WRONLY`, `O_CREAT`)
- `>` creates/overwrites files, `>>` appends to files
- **Limitation**: Only single redirection operator per command (no `< input.txt > output.txt`)

### Testing Commands

| Command | Description |
|---------|-------------|
| `ls > txt/folder_contents.txt` | Lists contents and writes to file |
| `ls -R > txt/folder_contents.txt` | Recursively lists and writes to file |
| `echo "=== Full-Year Calendar ===" > txt/calendar.txt` | Writes heading to calendar file |
| `cal -y >> txt/calendar.txt` | Appends full-year calendar to file |
| `tac txt/phrases.txt > txt/phrases_reversed.txt` | Reverses lines and writes to new file |
| `sort txt/phrases.txt > txt/phrases_sorted.txt` | Sorts lines and writes to new file |
| `head -n 5 txt/phrases.txt >> txt/phrases_sorted.txt` | Appends first 5 lines to existing file |
| `wc txt/phrases.txt > txt/phrases_stats.txt` | Counts and writes statistics to file |
| `grep June < txt/calendar.txt` | Searches for "June" in calendar file |
| `tr a-z A-Z < txt/phrases.txt` | Converts lowercase to uppercase from input file |

### Extension Challenge
Support both input and output redirection in same command: `sort < input.txt > output.txt`

---

## Section 3: Support for cd Command

### Objective
Implement the `cd` built-in command with directory navigation and prompt updates.

### Functions to Implement
1. **`init_lwd(char lwd[])`**
   - Initialize last working directory with current directory using `getcwd()`

2. **`is_cd(char line[])`**
   - Detect if command is a `cd` command
   - Return boolean

3. **`run_cd(char *args[], int argsc, char lwd[])`**
   - Execute directory change using `chdir()` system call
   - Handle special cases: `cd` (home), `cd -` (previous directory)
   - Update last working directory

4. **Update `construct_shell_prompt()`**
   - Include current working directory in prompt using `getcwd()`
   - Format: `[/current/path s3]$`

5. **Update `read_command_line()`**
   - Accept additional `lwd` parameter for prompt construction

### Key Considerations
- `cd` must run in parent process (shell), not child
- Use `chdir()` system call for directory changes
- Handle `cd` with no arguments (go to home directory)
- Handle `cd -` (go to previous directory)
- Update shell prompt to show current directory

### Updated Main Structure
```c
while (1) {
    read_command_line(line, lwd);
    
    if(is_cd(line)) {
        parse_command(line, args, &argsc);
        run_cd(args, argsc, lwd);
    }
    else if(command_with_redirection(line)) {
        parse_command(line, args, &argsc);
        launch_program_with_redirection(args, argsc);
        reap();
    }
    else {
        parse_command(line, args, &argsc);
        launch_program(args, argsc);
        reap();
    }
}
```

---

## Section 4: Commands with Pipes

### Objective
Implement pipeline execution using the pipe (`|`) operator for inter-process communication.

### Functions to Implement
1. **`command_with_pipes(char line[])`**
   - Detect if command contains pipe operators
   - Return boolean

2. **`tokenize_piped_commands(char line[], char *commands[])`**
   - Split command line into individual commands separated by pipes
   - Return array of command strings

3. **`launch_piped_commands(char *commands[], int num_commands)`**
   - Execute pipeline of commands
   - Create pipes between consecutive commands
   - Reuse existing functions where possible

4. **Augment Existing Functions**
   - Add pipe-related arguments to `launch_program()`, `child()`, etc.
   - Handle pipe file descriptors in child processes

### Key Considerations
- Use `pipe()` system call to create communication channels
- Use `dup2()` to redirect stdin/stdout between processes
- Support pipelines of any length (e.g., `cmd1 | cmd2 | cmd3 | cmd4`)
- Handle both parent and child process logic for pipe setup
- Consider edge cases: commands with redirection within pipelines

### Example Pipelines to Support
```bash
# Simple two-command pipeline
cat txt/phrases.txt | sort > txt/phrases_sorted.txt

# Multi-command pipeline
tr a-z A-Z < txt/phrases.txt | grep BURN | sort | wc -l

# Complex pipeline
ps aux | grep python | sort -k 3 -nr | head
```

### Programming Guidelines
- Write modular functions for pipe detection and tokenization
- Reuse existing `launch_program` and `child` functions with additional arguments
- Set pipe-related arguments to NULL/0 for non-piped commands
- Handle pipe creation and cleanup properly

---

## Section 5: Batched Commands

### Objective
Implement support for executing multiple commands sequentially using the semicolon (`;`) operator.

### Functions to Implement
1. **`command_with_batch(char line[])`**
   - Detect if command contains semicolon operators
   - Return boolean

2. **`tokenize_batched_commands(char line[], char *commands[])`**
   - Split command line into individual commands separated by semicolons
   - Return array of command strings

3. **`launch_batched_commands(char *commands[], int num_commands)`**
   - Execute commands sequentially, regardless of success/failure
   - Reuse existing functions for each command type (basic, redirection, pipes)

### Key Considerations
- Commands execute independently - failure of one doesn't stop others
- Each command can be basic, have redirection, or use pipes
- Maintain existing functionality while adding batch processing
- Consider updating main loop to handle batch detection

### Example Batch Commands
```bash
# Create directory, process file, confirm completion
mkdir results ; cat txt/phrases.txt | sort | tac > results/rev_sort_phr.txt ; echo "Processing complete."

# Multiple independent operations
pwd ; ls -l ; whoami ; date

# Mix of command types
echo "Starting..." ; ls > output.txt ; cat output.txt ; rm output.txt ; echo "Done."
```

### Programming Guidelines
- Write modular functions for batch detection and tokenization
- Reuse all existing command processing functions
- Handle each command in the batch according to its type (basic/redirection/pipes)
- Ensure proper cleanup between commands

---

## Proposed Extensions

### PE 1: Subshells

#### Objective
Implement subshells using parentheses `()` for isolated command execution.

#### Functions to Implement
1. **`command_with_subshell(char line[])`**
   - Detect if command contains parentheses
   - Return boolean

2. **`extract_subshell_commands(char line[], char *subshell_cmd)`**
   - Extract commands within parentheses
   - Handle subshells within larger command structures

3. **`launch_subshell(char *subshell_cmd)`**
   - Fork child process and execute shell binary with subshell commands
   - Use `execvp` to load shell binary with appropriate arguments

#### Key Considerations
- Subshells are child processes created with `fork()` and `execvp()`
- Changes in subshells don't affect parent shell (directory, variables)
- Subshells can be part of pipelines
- Shell program must handle `argc` and `argv` for subshell execution
- **Simplifying Assumption**: No nested subshells

#### Example Subshell Commands
```bash
# Subshell with directory change
echo "Start processing..." ; (cd /var/log ; cat syslog | sort > sorted_syslog.txt) ; echo "Finished."

# Subshell in pipeline
(cd /var/log ; cat syslog) | sort | head

# Multiple subshells in batch
(cd txt ; ls) ; (cd output ; ls) ; pwd
```

### PE 2: Nested Subshells

#### Objective
Remove nesting restriction from PE 1, allowing subshells within subshells.

#### Additional Functions
1. **`parse_nested_subshells(char line[])`**
   - Handle multiple levels of parentheses nesting
   - Use stack data structure to track matching parentheses

2. **`validate_parentheses_matching(char line[])`**
   - Ensure proper opening/closing of parentheses
   - Return boolean for valid nesting

#### Key Considerations
- Requires more sophisticated parsing logic
- Stack-based approach for tracking parentheses pairs
- Recursive handling of nested structures
- Maintain all functionality from PE 1

#### Example Nested Commands
```bash
# Two-level nesting
echo "Outer" ; (echo "Level 1" ; (echo "Level 2" ; pwd) ; echo "Back to 1") ; echo "Back to outer"

# Nested with pipes
(cat file1 ; (sort file2 | head -5)) | wc -l
```

---

## Further Enhancements (Optional)

These are additional features that go beyond the assignment requirements but can be explored for extended learning:

### Advanced I/O Features
- **Globbing**: Wildcard pattern matching with `*` and `?`
  - Example: `ls *.txt`, `rm test?.log`

- **Dual Redirection**: Support both input and output in same command
  - Example: `sort < unsorted.txt > sorted.txt`

- **Tee-like Behavior**: Alternative syntax for splitting output streams

### Job Control
- **Background Jobs**: Commands with `&` operator
- **Foreground Control**: `fg` and `bg` commands
- **Job Management**: List and control running processes

### User Interface Improvements
- **Command History**: Up/down arrow navigation through previous commands
- **Tab Completion**: Auto-complete commands and file names
- **Custom Prompts**: User-configurable prompt formats
- **Aliases**: User-defined command shortcuts

### Error Handling & Robustness
- **Signal Handling**: Proper handling of Ctrl+C, Ctrl+Z
- **Memory Management**: Dynamic allocation for large command lines
- **Error Recovery**: Graceful handling of malformed commands

### Performance & Advanced Features
- **Command Caching**: Cache frequently used command locations
- **Environment Variables**: Support for shell variables and `export`
- **Conditional Execution**: `&&` and `||` operators
- **Script Execution**: Run shell scripts from files

Remember: These enhancements are optional and primarily for those who want to explore shell programming further after completing the core requirements.

---

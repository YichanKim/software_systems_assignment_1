# S3 Shell Implementation - Assignment Report

## Student Information

**Student 1:**
- CID: 02327531
- Name: Yichan Kim

**Student 2:**
- CID: Sumukh Adiraju
- Name: 02563601

---

## Project Overview

This project implements a custom shell in C that provides a command-line interface fundamentally similar to standard Unix shells. The shell was built incrementally, adding features section by section as outlined in the project roadmap.

---
## Features Implemented

### Section 1: Basic Commands

**Description:** Implemented basic command execution without redirection or pipes. The shell can execute standard Unix commands by forking child processes and using `execvp()` to load binaries from the system PATH.

**Key Functions:**
- `launch_program()`: Handles command execution, including special handling for the "exit" command
- `child()`: Uses `execvp()` to replace the child process with the specified command programme

**Status:** Fully functional. All test commands from the project brief execute successfully, including `whoami`, `pwd`, `ls`, `cat`, `grep`, `sort`, `wc`, `cp`, `touch`, `chmod`, `less`, and `exit`.

---

### Section 2: Commands with Redirection

**Description:** Added support for input (`<`) and output (`>`, `>>`) redirection operators. Commands can read from files or write output to files.

**Key Functions:**
- `command_with_redirection()`: Detects redirection operators in command lines
- `find_redirection()`: Identifies redirection operator/filename pairs and determines mode (input, append, truncate)
- `launch_program_with_redirection()`: Handles command execution with file descriptor redirection
- `child_with_redirection()`: Helper function that uses `dup2()` to redirect stdin/stdout before executing commands

**Status:** Fully functional. Supports single redirection operator per command (as per requirements). All redirection test cases work correctly, including:
- Output redirection: `ls > file.txt`, `sort > file.txt`
- Append mode: `cal -y >> file.txt`
- Input redirection: `grep pattern < file.txt`

**Limitation:** Only single redirection operator per command is supported (no `sort < input.txt > output.txt` in the same command, as per section requirements).

---

### Section 3: Support for `cd` Command

**Description:** Implemented the `cd` built-in command with directory navigation and dynamic prompt updates showing the current working directory.

**Key Functions:**
- `is_cd()`: Detects if a command is a `cd` command (handles edge cases like "cdcd")
- `init_lwd()`: Initialises the last working directory buffer
- `run_cd()`: Executes directory changes using `chdir()` system call
- `construct_shell_prompt()`: Updated to display current working directory in format `[/current/path s3]$`

**Status:** Fully functional. Supports:
- `cd` (goes to home directory via `$HOME`)
- `cd /path/to/directory`
- `cd -` (switches to previous directory)
- `cd ..` (goes to parent directory)
- `cd .` (stays in current directory)
- Error handling for invalid paths and too many arguments

**Note:** Support for `cd ~` was not implemented (as noted in project brief).

---

### Section 4: Commands with Pipes

**Description:** Implemented pipeline execution using the pipe (`|`) operator for inter-process communication, allowing multiple commands to be chained together.

**Key Functions:**
- `command_with_pipes()`: Detects pipe operators in command lines
- `tokenize_pipeline()`: Splits command line into individual commands separated by pipes, with parentheses-aware parsing
- `launch_pipeline()`: Executes pipeline of commands, creating pipes between consecutive stages
- `child_with_pipes()`: Helper function that redirects stdin/stdout through pipes
- `child_with_pipes_and_redirection()`: Combined helper for pipelines ending with output redirection

**Status:** Fully functional. Supports:
- Multi-stage pipelines: `cmd1 | cmd2 | cmd3 | cmd4`
- Pipelines with redirection: `cat file | sort | tac > output.txt`
- Parentheses-aware parsing (ignores pipes inside subshells)
- Proper pipe cleanup and process management

**Bug Fix:** Fixed issue where pipelines ending with output redirection were passing redirection operators as arguments to commands. Now properly handles `cat file | sort > output.txt`.

---

### Section 5: Batched Commands

**Description:** Implemented support for executing multiple commands sequentially using the semicolon (`;`) operator. Commands execute independently, with failures not stopping subsequent commands.

**Key Functions:**
- `command_with_batch()`: Detects semicolon operators in command lines
- `tokenize_batched_commands()`: Splits command line into individual commands separated by semicolons, with parentheses-aware parsing
- `launch_batched_commands()`: Executes commands sequentially, routing each to appropriate handler (basic, redirection, pipes, cd, subshells)

**Status:** Fully functional. Supports:
- Sequential command execution: `cmd1 ; cmd2 ; cmd3`
- Mixed command types in batches: `echo "Start" ; ls > file.txt ; cat file.txt ; rm file.txt`
- Parentheses-aware parsing (ignores semicolons inside subshells)
- Proper error handling and cleanup between commands

---

## Proposed Extensions Implemented

### PE 1: Subshells

**Description:** Implemented subshell execution using parentheses `()` for isolated command execution. Subshells run in separate child processes, so directory changes and other state modifications don't affect the parent shell.

**Key Functions:**
- `command_with_subshell()`: Detects parentheses in command lines
- `extract_subshell_commands()`: Extracts command string between parentheses, handling whitespace trimming
- `launch_subshell()`: Forks child process and executes shell binary with subshell commands
- Subshell argument handler in `s3main.c`: Processes commands when shell is invoked with arguments

**Status:** Fully functional. Supports:
- Standalone subshells: `(cd txt ; ls)`
- Subshells in batches: `echo "Start" ; (cd txt ; ls) ; echo "End"`
- Subshells in pipelines: `(cd txt ; cat file.txt) | head -n 5`
- Complex combinations: `(cd txt ; cat file.txt | sort) | head -n 3`
- Process isolation verified: `pwd ; (cd txt ; pwd) ; pwd` confirms parent directory unchanged

**Implementation Details:**
- Subshells work by having the shell execute itself (`./s3`) with the subshell command as an argument
- Each subshell runs in a separate forked process
- I/O redirection properly handled in pipelines containing subshells

---

### PE 2: Nested Subshells

**Description:** Implemented nested subshells through recursive execution. The shell supports arbitrary levels of nesting (e.g., `(((echo "hi"))))`). When a subshell is launched, it executes the shell binary again with the subshell command as an argument. If that command contains nested parentheses, the shell recursively processes them, naturally supporting any nesting depth.

**Key Functions:**
- `extract_subshell_commands()`: Extracts top-level parentheses content, tracking depth to find matching pairs
- `launch_subshell()`: Executes shell recursively with subshell command
- Subshell argument handler in `s3main.c`: Processes commands when shell is invoked with arguments, detecting and handling nested subshells recursively

**Status:** Fully functional. Supports:
- Arbitrary nesting levels: `(((echo "hi"))))`
- Nested subshells in batches: `echo "Outer" ; (echo "lvl 1" ; (echo "lvl 2" ; pwd) ; echo "back to lvl 1") ; echo "back to outer subshell"`
- Nested subshells in pipelines: `(cat file1 ; (sort file2 | head -5)) | wc -l`
- Complex combinations with all features

**Implementation Approach:** Uses recursive execution rather than explicit stack-based parsing. When a subshell is launched, it spawns a new shell instance that processes the command. If that command contains nested parentheses, the new shell instance detects and processes them, creating a natural recursive solution that supports unlimited nesting depth.

---

## Extra Features Implemented

### 1. Quote Removal

**Description:** Added support for removing surrounding quotes from command arguments. This handles cases like `echo "Hi"` where the quotes should be stripped before passing to the command.

**Implementation:** Modified `parse_command()` to detect and remove surrounding single or double quotes from tokens using `memmove()`.

**Status:** Fully functional. Commands like `echo "Hello World"` and `echo 'Hello World'` properly strip quotes before execution.

---

### 2. Parentheses-Aware Parsing

**Description:** Enhanced parsing functions to ignore pipes '`|`' and semicolons '`;`' that appear inside parentheses. This allows proper handling of subshells within pipelines and batched commands.

**Implementation:** Updated `tokenize_pipeline()` and `tokenize_batched_commands()` to track parentheses depth and skip operators when inside parentheses. This meant scrapping the original `strtok_r` approach to split a raw line into two individual commands

**Status:** Fully functional.

---

### 3. Pipeline Redirection Fix

**Description:** Fixed bug where pipelines ending with output redirection (e.g., `cat file | sort > output.txt`) were passing redirection operators as arguments to commands instead of redirecting output.

**Implementation:** Added `child_with_pipes_and_redirection()` helper function and modified `launch_pipeline()` to detect and handle redirection in the last command of a pipeline.

**Status:** Fully functional.

---

### 4. Enhanced Error Handling

**Description:** Added comprehensive error handling throughout the codebase, including:
- Unbalanced parentheses detection
- Empty command detection in pipelines and batches
- Invalid redirection syntax handling
- File descriptor cleanup on errors

**Status:** Fully functional.

---
### Known Limitations:
1. Single redirection operator per command (as per Section 2 requirements)
2. `cd ~` not supported (not written in project brief)

---

## Compilation and Execution

### Prerequisites:
- Linux/Unix environment (Ubuntu/WSL tested)
- GCC compiler
- Standard C libraries

### Compilation:
```bash
gcc *.c -o s3
chmod +x s3
```

### Execution:
```bash
./s3
```

The shell will start with a prompt showing the current working directory.
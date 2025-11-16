# S3 Shell Implementation - Development Logbook

The s3 shell is being built incrementally. This logbook documents my progress and technical insights gained.

---

## Section 1: Basic Commands - COMPLETED

### What I Accomplished
Implemented basic command execution using `fork()` and `execvp()`. The shell can now run standard Unix commands like `ls`, `whoami`, `pwd`, `cat`, etc.

### Implementation
- **`child()`**: Uses `execvp()` to replace child process with the command program
- **`launch_program()`**: Handles "exit" command and creates child processes using `fork()`

### Key Insights I Learned

#### How `execvp()` Works
- Completely replaces the current process with a new program
- Searches PATH directories to find executables
- Never returns if successful - the process becomes the new program
- Only returns `-1` on failure

#### The Error Check Pattern
```c
if (execvp(args[ARG_PROGNAME], args) == -1) {
```
If `execvp()` succeeds, the process transforms and never reaches the if statement. If it fails, it returns `-1` and I can handle the error.

#### Shell as Program Launcher
My shell doesn't know what `ls` or `cat` do. It's just a program finder that uses the system PATH to locate and execute programs.

### Testing Results

Comprehensive testing of basic commands:

| Command | Description | Status |
|---------|-------------|--------|
| `whoami` | Shows current username | ✅ |
| `pwd` | Prints current working directory | ✅ |
| `ls` | Lists files and folders in directory | ✅ |
| `ls -R` | Recursively lists files in subdirectories | ✅ |
| `clear` | Clears the terminal screen | ✅ |
| `man cat` | Opens manual page for cat command | ✅ |
| `cat txt/phrases.txt` | Displays contents of phrases.txt | ✅ |
| `uniq txt/phrases.txt` | Removes duplicate lines | ✅ |
| `wc txt/phrases.txt` | Counts lines, words, and characters | ✅ |
| `sort txt/phrases.txt` | Sorts lines alphabetically | ✅ |
| `grep burn txt/phrases.txt` | Finds lines containing "burn" | ✅ |
| `grep -n burn txt/phrases.txt` | Finds "burn" with line numbers | ✅ |
| `cp txt/phrases.txt txt/phrases_copy.txt` | Copies file to new file | ✅ |
| `touch txt/new_file.txt` | Creates empty file | ✅ |
| `chmod a-w txt/phrases_copy.txt` | Makes file read-only | ✅ |
| `ls -l txt/phrases_copy.txt` | Shows file details and permissions | ✅ |
| `less txt/phrases.txt` | Opens file in scrollable view | ✅ |
| `exit` | Exits shell session | ✅ |

All tests passed. The shell successfully executes standard Unix commands by finding them in the system PATH.

---

## Section 2: Commands with Redirection - COMPLETED

### What I Accomplished
Added support for single-operator input and output redirection so commands can read from or write to files using `<`, `>`, or `>>`.

### Implementation
- **`command_with_redirection()`**: Scans the raw command line for redirection symbols before parsing.
- **`find_redirection()`**: Identifies the operator/filename pair in the tokenized argument list and captures mode (input vs append vs truncate).
- **`child_with_redirection()`**: Uses `open()` and `dup2()` to rewrite `stdin` or `stdout`, then executes the command with `execvp()`.
- **`launch_program_with_redirection()`**: Strips the operator and filename from the argument list, forks, and delegates to the redirection-aware child helper.

### Key Insights I Learned
- Removing the operator and filename (by setting their slots to `NULL`) ensures `execvp()` only sees real arguments.
- `dup2()` must be called in the child after `fork()`; doing so in the parent would redirect the shell itself.
- Using `O_WRONLY | O_CREAT` plus either `O_TRUNC` or `O_APPEND` matches the behavior of `>` vs `>>` in standard shells.
- Limiting support to a single redirection operator keeps parsing straightforward and matches the section requirements.

### Testing Results
All redirection test commands from the roadmap executed successfully (e.g., `ls > ...`, `sort > ...`, `grep < ...`, `tr < ...`). Files were created/updated with the expected contents, confirming the feature works as intended.

---

## Section 3: Support for `cd` Command - COMPLETED

### What I Accomplished
Added support for cd commands, including special cases such as "cd ..", "cd -", "cd", "cd .", "cd /path/to/invalid file", "cd path/to/file", "cd a b"

Note that support for cd ~ have not been added

### Implementation
- **`is_cd()`**: Detects if the command/line begins with "cd", ignores leading whitespace and ensures it is followed by end of line or a space. (accounts for cases like 'cdfoo' would not work).
- **`init_lwd()`**: initialises the last working directory (lwd) buffer so that "cd -" will not run when it is the first command that is being run after shell start (it will allow for the shell to return "no previous directory").
- **`run_cd()`**: Uses chdir() function to run the "cd" command. After a successful change, updates the lwd buffer accordingly. Handles:
    - cd -> goes to $HOME   
    - cd /path/ -> goes to /path/
    - cd - -> switches to previous directory
    - cd .. -> goes to upper directory
    - cd . -> stays in present directory
    - cd a b -> error, too many arguments
    - cd /invalid/path -> error, invalid path
Also additionally altered **`read_command_line`** and **`construct_shell_prompt`** to account for our newly created lwd buffer.

### Key Insights I Learned
- lwd's length is set to [MAX_PROMPT_LEN-6] to accout for the "[ s3]$" characters (there are six of them).
- cd must run in the parent shell process since the child changing directories would not change the directory of the shell itself.
- The `cd -` command must fail when there is an invalid previous directory (not set yet).
- Not implementing support for `cd ~` keeps the cd straightforward and matches the section requirements.

### Testing Results
All cd test commands from the roadmap executed successfully. The shell prompt was able to navigate between directories and output them as a prompt, confirming the feature works as intended.

---

## Section 4: Commands with Pipes - COMPLETED

### What I Accomplished
Implemented pipeline execution so multiple commands can be chained with `|`, letting the stdout of one command flow into the stdin of the next.

### Implementation
- **`command_with_pipes()`**: Detects pipe symbols before any parsing work is done.
- **`tokenize_pipeline()`**: Splits the raw line into per-command segments, trimming whitespace and rejecting empty stages.
- **`child_with_pipes()`**: Duplicates designated pipe ends onto `stdin` and/or `stdout` before calling `execvp()`.
- **`launch_pipeline()`**: Iterates through the pipeline, wiring pipes between adjacent stages, forking children, and closing unused descriptors in parent and child.

### Key Insights I Learned
- By reusing `parse_command()` on each pipeline segment, the existing argument parsing logic stays consistent.
- The parent must close each write end immediately and carry only the read end forward; otherwise, downstream processes never see EOF.
- Passing `-1` for unused read/write descriptors keeps the helper generic and avoids accidentally duping invalid fds.
- Letting the main loop call `reap()` afterward keeps process cleanup centralized, even for multi-stage pipelines.

### Testing Results
All pipeline test commands from the roadmap ran successfully (e.g., `cat | sort | uniq`, multi-stage pipes with redirection). Outputs matched the expected data, showing that inter-process plumbing works end-to-end.

---

## Section 5: Batched Commands - COMPLETED

### What I Accomplished
Implemented batched command execution using `;`, allowing multiple commands to run sequentially and independently (a failure does not stop the rest).

### Implementation
- **`command_with_batch()`**: Detects semicolons in the raw line to route to batch handling early.
- **`tokenize_batched_commands()`**: Splits the line on `;`, trims whitespace on both ends, and skips empty segments so accidental doubles like `cmd1;;cmd2` don’t break execution.
- **`launch_batched_commands()`**: Iterates each segment and reuses existing paths: pipelines (`launch_pipeline()`), redirection (`launch_program_with_redirection()`), or basic (`launch_program()`), calling `reap()` after each.
- Main loop updated to prioritize: `cd` → batch → pipes → redirection → basic.

### Key Insights I Learned
- Trimming both leading and trailing spaces per segment prevents subtle parse errors (e.g., missing operand symptoms).
- Rechecking the original segment string (not just parsed args) is important to detect pipes/redirection reliably before altering tokens.
- Keeping `cd` in the parent path (even inside batches) preserves shell state; other commands can safely fork/exec.

### Testing Results
All batch test scenarios (mixed basic, redirection, and pipelines; failure within batch; directory changes) executed as expected, confirming independent, sequential execution.

---

### Environment
- **Platform**: Ubuntu/WSL 
- **Compilation**: `gcc *.c -o s3`

---

## Next Steps

- **PE 1**: Subshells (parentheses) — launch nested shell processes for grouped execution
- **PE 2**: Nested Subshells — stack-based parsing for arbitrary nesting
- Further enhancements (optional): history, tab completion, job control, globbing

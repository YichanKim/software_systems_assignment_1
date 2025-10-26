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

### Environment
- **Platform**: Ubuntu/WSL 
- **Compilation**: `gcc *.c -o s3`

---

## Next Steps

- **Section 2**: Commands with Redirection
- **Section 3**: Support for `cd` command  
- **Section 4**: Commands with Pipes
- **Section 5**: Batched Commands

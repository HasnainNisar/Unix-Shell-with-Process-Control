# Unix Shell with Process Control

A Unix-like command-line shell built in C, supporting process management,
piping, I/O redirection, signal handling, and command history.
Built at NED University of Engineering & Technology, 2026.

## Features

- Basic built-in commands: `exit`, `pwd`, `clear`, `cd`, `help`
- Process creation using `fork()` and `execvp()` system calls
- Piping (`cmd1 | cmd2`) via `pipe()` and `dup2()`
- Output redirection (`>`) using `dup2()` system call
- Background process execution with `&`
- Signal handling — `Ctrl+C` does not exit the shell
- Command history — type `!!` to repeat last command

## Key System Calls Used

| Call | Purpose |
|------|---------|
| `fork()` | Creates child process |
| `execvp()` | Executes command in child process |
| `waitpid()` | Parent waits for child to finish |
| `pipe()` | Creates communication channel between processes |
| `dup2()` | Redirects standard input/output |
| `signa

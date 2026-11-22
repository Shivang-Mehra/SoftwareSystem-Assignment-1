# SoftwareSys shell

A simple shell written in C for Software Systems Assignment 1.

## What's done so far

**Basic commands**
Runs normal programs like `ls`, `cat file.txt`, `grep -n word file.txt`. The line gets split into words, then we `fork()` a child and the child runs the program with `execvp`. The parent waits for it to finish. `exit` is handled by the shell itself so it closes the shell, and an empty line just gets ignored.

**cd**
`cd` is built into the shell and uses `chdir()`, because if a child process did it, the shell's own folder wouldn't change. It supports `cd folder`, `cd ..`, `cd` on its own (goes to home) and `cd -` (goes back to the previous folder).

**Prompt**
The prompt shows the current folder using `getcwd()` and updates after every `cd`, e.g. `[/home/user/project s3]$`.

**Pipes**
Pipes work for any number of commands, e.g. `cat txt/phrases.txt | grep burn | sort | wc -l`.
- The line is split at each `|` and every part is parsed into its own list of words.
- For N commands we make N-1 pipes with `pipe()`.
- Each command gets its own child. It reads from the pipe before it and writes to the pipe after it using `dup2` (the first one only writes, the last one only reads).
- Each child closes all the pipe ends before running its program, otherwise the pipeline hangs waiting for input that never ends.
- The parent closes its pipe ends too and waits for every child.

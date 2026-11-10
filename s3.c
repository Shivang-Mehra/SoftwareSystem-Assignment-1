#include "s3.h"

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    char current[MAX_PROMPT_LEN - 6];
     if (getcwd(current, sizeof(current)) == NULL) {
        perror("getcwd failed");
        return;
    }
    snprintf(shell_prompt, MAX_PROMPT_LEN, "[%s s3]$ ", current);

}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[], char lwd[])
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
    ///Implement this function:

    ///Use execvp to load the binary 
    ///of the command specified in args[ARG_PROGNAME].
    ///For reference, see the code in lecture 3.
    execvp(args[ARG_PROGNAME], args);

    perror("execvp failed");
    exit(1);
}

void launch_program(char *args[], int argsc)
{
    
    if (argsc == 0) return;
    if (strcmp(args[ARG_PROGNAME], "exit") == 0)
    {
        exit(0);
    }

    int lp = fork();
    if (lp < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (lp == 0) {
        child(args, argsc);
    }

    ///Implement this function:
    ///fork() a child process.
    ///In the child part of the code,
    ///call child(args, argv)
    ///For reference, see the code in lecture 2.

    ///Handle the 'exit' command;
    ///so that the shell, not the child process,
    ///exits.
}

int command_with_redirection(char line[]) {

    return(strchr(line, '>') != NULL || strchr(line, '<') != NULL);

}

void launch_program_with_redirection(char *args[], int argsc) {

}

void init_lwd(char lwd[]) {
    if (getcwd(lwd, MAX_PROMPT_LEN-6) == NULL){
        perror("getcwd failed");
        exit(1);
    };
}

int is_cd(char line[]) {
    char copy[MAX_LINE];
    strcpy(copy, line);

    char *token = strtok(copy, " ");
    return (token != NULL && strcmp(token, "cd") == 0);
    
}

void run_cd(char *args[], int argsc, char lwd[]) {

    if (argsc == 0) return;
    if (argsc > 2) {
        fprintf(stderr, "cd: too many arguments\n");
        return;
    }
    char *destination;
    if (argsc == 1) {  //cd to home
        destination = getenv("HOME");
        if (destination == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return;
        }
    } else if (strcmp(args[1], "-") == 0){ //cd to -
        destination = lwd;
    } else { //cd to specific location
        destination = args[1];
    }
    char current[MAX_PROMPT_LEN - 6];
    if (getcwd(current, sizeof(current)) == NULL) {
        perror("getcwd failed");
        return;
    }

    if (chdir(destination) == -1) {
        perror("cd failed");
        return;
    } else {
        strcpy (lwd, current);
        return;
    }
}

int command_with_pipe(char line[]){

    return(strchr(line, '|') != NULL);
}

void launch_program_with_pipes(char line[]) {
    
    char *individual[MAX_ARGS];
    int count = 0;
    char *token = strtok(line, "|");
    while (token != NULL && count < MAX_ARGS - 1) //spilt pipe command
    {
        individual[count] = token;
        count++;
        token = strtok(NULL, "|");
    }
    individual[count] = NULL;


    char *all_args[MAX_ARGS][MAX_ARGS];   
    int all_argsc[MAX_ARGS];              
    
    for (int i = 0; i < count; i++)
    {
        parse_command(individual[i], all_args[i], &all_argsc[i]);
    }

    int pipes[MAX_ARGS - 1][2];
    for (int i = 0; i < count - 1; i++) 
    {
        pipe(pipes[i]);
    }

    for (int i = 0; i < count; i++) 
    {
        int lp = fork();
        if (lp < 0) {
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
        else if (lp == 0) {
            if (i == 0){
                dup2(pipes[i][1], STDOUT_FILENO);
                
            }
            else if (i < (count - 1)) {
                dup2(pipes[i-1][0], STDIN_FILENO);
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            else {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            for (int j = 0; j < count - 1; j++) 
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            child(all_args[i], all_argsc[i]);
            
        }
    }
    for (int i = 0; i < count - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

for (int i = 0; i < count; i++)
{
    wait(NULL);
}
}
void launch_program_with_redirection(char *args[], int *argsc) {
    
}

void child_with_input_redirected()

void child_with_output_redirected()


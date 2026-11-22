#include "s3.h"

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    char current[MAX_PROMPT_LEN - 8];
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
    }else {
        wait(NULL);   // parent reaps the child
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
    int op_type = 0;
    char *name = NULL; 

    for(int i=0; i<argsc; i++){
        if (strcmp(args[i], ">") == 0){
            name = args[i+1];
            args[i] = NULL;
            
            op_type = 1;
            break;
        }        //output + overwrite
        else if (strcmp(args[i], ">>") == 0){
            name = args[i+1];
            args[i] = NULL;
            
            op_type = 2;
            break;
        }  // output, append
        else if (strcmp(args[i], "<") == 0){
            name = args[i+1];
            args[i] = NULL;
            
            op_type = 3;
            break;
        }
    }

        if (op_type == 0 || name == NULL) {
                fprintf(stderr, "syntax error, bad redirection\n");
                return;
        }

        int pid = fork();
        if (pid < 0) { 
            printf("Child creation not work");
        }
        else if (pid == 0) { /* choose helper */
            if((op_type == 1) || (op_type == 2)){
                child_with_output_redirected(args, argsc, name, op_type == 2);
            }
            else if(op_type ==3){
                child_with_input_redirected(args, argsc, name);
            }
            else{
                printf("idk but we messed up somewhere");
            }
        }
        
        else { wait(NULL);         
        
    }
}

void child_with_input_redirected(char *args[], int argsc, char *filename){
    int fd = open(filename, O_RDONLY);
    if (fd < 0) { perror("open failed"); exit(1); }
    if (dup2(fd, STDIN_FILENO) < 0) { 
        perror("dup2 failed"); exit(1); 
    }

    close(fd);
    child(args, argsc);
}

void child_with_output_redirected(char *args[], int argsc, char *filename, int append)
{
    int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
    int fd = open(filename, flags, 0644);
    if (fd < 0) { perror("open failed"); exit(1); }
    if (dup2(fd, STDOUT_FILENO) < 0) { 
        perror("dup2 failed"); exit(1); 
    }

    close(fd);
    child(args, argsc);
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

    return has_delim_at_depth0(line, '|');
}

void launch_program_with_pipes(char line[]) {
    
    char *individual[MAX_ARGS];
    char *name = NULL;
    int op_type = 0;

    int count = split_at_depth0(line, '|', individual);

    char *all_args[MAX_ARGS][MAX_ARGS];   
    int all_argsc[MAX_ARGS];  
    char *inner_text[MAX_ARGS];            
    
    for (int i = 0; i < count; i++)
    {
        if(is_subshell(individual[i])){
            inner_text[i] = subshell_inner(individual[i]);
            all_argsc[i] = 0;
        }else{
            inner_text[i] = NULL;
            parse_command(individual[i], all_args[i], &all_argsc[i]);
        }
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

            for (int j = 0; j < count - 1; j++){
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            if (inner_text[i] != NULL){
                child_subshell(inner_text[i]);
            }

            for(int k=0; k<all_argsc[i]; k++){
                if (strcmp(all_args[i][k], ">") == 0){
                    name = all_args[i][k+1];
                    all_args[i][k] = NULL;
                    
                    op_type = 1;
                    break;
                }        //output + overwrite
                else if (strcmp(all_args[i][k], ">>") == 0){
                    name = all_args[i][k+1];
                    all_args[i][k] = NULL;
                    
                    op_type = 2;
                    break;
                }  // output, append
                else if (strcmp(all_args[i][k], "<") == 0){
                    name = all_args[i][k+1];
                    all_args[i][k] = NULL;
                    
                    op_type = 3;
                    break;
                }
            }
            
            if (op_type != 0 && name == NULL) {
                fprintf(stderr, "syntax error, bad redirection\n");
                exit(1);
            }else if (op_type != 0) {
                int flags;
                int target;

                if (op_type == 1) {
                    flags = O_WRONLY | O_CREAT | O_TRUNC;
                    target = STDOUT_FILENO;
                } else if (op_type == 2) {
                    flags = O_WRONLY | O_CREAT | O_APPEND;
                    target = STDOUT_FILENO;
                } else {
                    flags = O_RDONLY;
                    target = STDIN_FILENO;
                }

                int fd = open(name, flags, 0644);
                if (fd < 0) { perror("open failed"); exit(1); }
                if (dup2(fd, target) < 0) { perror("dup2 failed"); exit(1); }
                close(fd);
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

int command_with_batch(char line[]) {
    return has_delim_at_depth0(line, ';');
}

int has_delim_at_depth0(char line[], char delim){
    int depth = 0;
    int len = strlen(line);
    for(int i=0; i<len; i++){
        if(line[i] == '('){
            depth++;
        }else if(line[i] == ')'){
            depth --;
        }else{
            if(line[i] == delim && depth == 0){
                return 1;
            }
        }
    }
    return 0;
}

void launch_batch_commands(char line[], char lwd[]){
    char *individual[MAX_ARGS];
    int count = split_at_depth0(line, ';', individual);

    for (int i = 0; i < count; i++) {
        run_unit(individual[i], lwd);
    }
}

void run_unit(char unit[], char lwd[]){
    char *args[MAX_ARGS];
    int argsc;

    if(is_cd(unit)){
        parse_command(unit, args, &argsc);
        run_cd(args, argsc, lwd);
    }else if(command_with_pipe(unit)){
        launch_program_with_pipes(unit);
    }else if(command_with_redirection(unit)){
        parse_command(unit, args, &argsc);
        launch_program_with_redirection(args, argsc);
    }else if(is_subshell(unit)){
        launch_subshell(unit);
    }else{
        parse_command(unit, args, &argsc);
        launch_program(args, argsc);
    }
    
}

int split_at_depth0(char line[], char delim, char *pieces[]){
    int depth =0;
    int count =0;
    int len = strlen(line);

    pieces[count++] = line;

    for(int i=0; i<len; i++){
        if(line[i] == '('){
            depth ++;
        }else if(line[i] == ')'){
            depth --;
        }else if(line[i] == delim && depth ==0){
            line[i] = '\0';
            pieces[count++] = &line[i+1];
        }
    }
    return count;
}
 
int is_subshell(char unit[]){
    int index=0;
    int len = strlen(unit);

    while(unit[index] == ' '){
        index++; //find index of the first non space
    }

    if(unit[index] != '('){
        return 0;
    }

    int depth = 1; //already 1 as the bracket is open

    for(int i=index+1; i<len; i++){
        if(unit[i] == '('){
            depth ++;
        }else if(unit[i] == ')'){
            depth --;
        }
        if(depth ==0){
            for(int j=i+1; j<len; j++){
                if(unit[j] != ' '){
                    return 0;
                }
            }
            return 1;
        }
    }

    return 0;
}

void child_subshell(char inner[]) {
    char shell_path[MAX_LINE];
    ssize_t n = readlink("/proc/self/exe", shell_path, sizeof(shell_path) - 1);
    if (n < 0) { perror("readlink failed"); exit(1); }
    shell_path[n] = '\0';

    char *argv[] = { shell_path, inner, NULL };
    execv(shell_path, argv);
    perror("execv failed");
    exit(1);
}

char *subshell_inner(char unit[]) {
    char *start = strchr(unit, '(');          // the opening bracket
    int end = strlen(unit) - 1;
    while (end > 0 && unit[end] == ' ') {
        end--;                                // skip trailing spaces
    }
    unit[end] = '\0';                         // overwrite the closing bracket
    return start + 1;
}

void launch_subshell(char unit[]) {
    char *inner = subshell_inner(unit);

    int pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork failed\n");
        return;
    }
    else if (pid == 0) {
        child_subshell(inner);
    }
    else {
        wait(NULL);
    }
}
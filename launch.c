#include "launch.h"
#include "built-in.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <linux/limits.h>


// Print out error when allocation failed
void ksh_allocate_error()
{
    fprintf(stderr, "ksh: allocation failed...\n");
    exit(EXIT_FAILURE);
}

// 1. Read a line from standrad input (stdin)
char* ksh_read_line(void) 
{
    // (1) use 'getline' to read a line from standard input
    #ifdef KSH_USE_GETLINE
        char* line = NULL;
        ssize_t bufsize = 0;
        if (getline(&line, &bufsize, stdin) == -1)
        // "getline" automatically allocates memory for line
        {
            if (feof(stdin)) exit(EXIT_SUCCESS); 
            // check the file pointer of stdin to see if it is at the end of the file
            // if it is, exit the program, else print error message and exit
            else
            {
                perror("ksh: getline\n");
                exit(EXIT_FAILURE);
            }
        }
    #endif

    // (2) use 'getchar' to read a line from standard input, 
    //     one character by one character, and store them in a buffer
    int bufsize = KSH_RL_BUFSIZE;                       
    // buffer size
    int position = 0;                                   
    // position of the buffer
    char* buffer = malloc(sizeof(char) * bufsize);      
    // allocate memory for buffer, buffer points to a memory block of size bufsize
    int c;

    // if malloc failed, return NULL to buffer to indicate error
    if (!buffer) ksh_allocate_error();

    while (1)
    {
        // Read a character
        c = getchar();

        // If user hits EOF or Enter, replace it with null character and return
        // Else add the character to buffer and keep reading
        buffer[position] = (c == EOF || c == '\n') ? '\0' : c;
        if (c == EOF || c == '\n') return buffer;
        position ++ ;

        // If buffer is maxed out, reallocate the memory space for buffer
        if (position >= bufsize)
        {
            buffer = realloc(buffer, bufsize += KSH_RL_BUFSIZE);
            // realloc is used to reallocate memory space when buffer is allocated with malloc already
            if (!buffer) ksh_allocate_error();
        }
    }
}

// 2. Split the input 'line' into several tokens(several strings)
char** ksh_split_line(char* line)
{
    int bufsize = KSH_TOKEN_BUFSIZE;
    int position = 0;
    char** tokens = malloc(bufsize * sizeof(char*));    // sizeof char* is 8 bytes
    char* token;

    if (!token) ksh_allocate_error();

    token = strtok(line, KSH_TOKEN_DELIMTERS);
    // strtok is used to split a string into tokens
    while (token != NULL)
    {
        tokens[position ++ ] = token;

        if (position >= bufsize)
        {
            bufsize += KSH_TOKEN_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) ksh_allocate_error();
        }

        token = strtok(NULL, KSH_TOKEN_DELIMTERS);
        // pass NULL to strtok to continue splitting the string
        // with internal state maintained by strtok, providing the string to be split
    }

    tokens[position] = NULL;

    return tokens;
}

// 3. Execute the command (not built-in type)
int ksh_launch(char** args)
{
    pid_t pid, wpid;    
    // pid is process id, wpid is wait process id
    int status;         
    // status of the process

    pid = fork();
    // (1) in parent process, fork() returns the process ID of the child process
    // (2) in child process, fork() returns 0
    // (3) if fork() fails, it returns -1

    // fork a child process
    if (pid == 0)
    {
        if (execvp(args[0], args) == -1) perror("ksh: excecution failed...");
        // execvp is used to set up a new program in the current process space
        // args[0] is the executable file name, args is the arguments
        // like execvp("ls", ["ls", "-l"]), then search "ls" in PATH ("/bin/ls") and execute it
        // execvp will search for the executable file in the PATH environment variable
        // only need to pass the file name, not the full path
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) perror("ksh: fork failed...");
    else  
    // pid is the process id of the child process
    {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            // waitpid is used to wait for a child process to change state
            // Return value "wpid" is the process ID of the child process whose state has changed
            // (1) some child process has changed state, return the process ID
            // (2) no child process has changed state, return 0
            // (3) error, return -1
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        // expands to !(((status) & 0x7f) == 0) and 
        // !(((signed char) (((status) & 0x7f) + 1) >> 1) > 0)
        // (1) when the child process has exited normally, WIFEXITED(status) is true
        // (2) when the child process has exited due to a signal, WIFSIGNALED(status) is true
    }

    return 1;
}

// 4. Execute the built-in commands (built-in type and not built-in type)
int ksh_execute(char** args)
{
    // User typed in nothing, return 1 and continue
    if (args[0] == NULL) return 1;
    for (int i = 0; i < ksh_num_builtins(); i ++ )
        if (strcmp(args[0], builtin_str[i]) == 0)
            return (*builtin_func[i])(args);
            // call the built-in function by passing the arguments
    
    // If the command is not a built-in command, execute it with ksh_launch method
    return ksh_launch(args);
}

// 6. Main loop of the shell
void ksh_loop(void)
{
    char* line;
    char** args;
    int status;
    char cwd[PATH_MAX];                 // cwd is the current working directory
    char* username = getenv("USER");    // get the username from the environment variable

    do {
        if (getcwd(cwd, sizeof(cwd)) != NULL) 
        {
            if (username != NULL) printf("\033[35m%s\033[0m in \033[32m%s\033[0m \033[33mλ\033[0m ", username, cwd);
            else printf("ksh: unknown user@ksh: ");
        }
        else perror("ksh: getcwd failed...\n");

        line = ksh_read_line();
        args = ksh_split_line(line);
        status = ksh_execute(args);
        // (1) read a line from standard input
        // (2) split the line into tokens
        // (3) execute the command with the tokens

        free(line);
        free(args);
    } while (status); // status is 1, continue the loop
}
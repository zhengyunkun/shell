#pragma once

#define KSH_RL_BUFSIZE 1024 // 1 KB of buffer size
#define KSH_TOKEN_BUFSIZE 64    
// buffer size for tokens, tokens are used to store the command and arguments
#define KSH_TOKEN_DELIMTERS " \t\r\n\a"
// like ' ', '\t', '\r', '\n', '\a' are delimiters (分界符)

// Function declarations for shell lauching
extern void ksh_allocate_error();
extern char* ksh_read_line(void);
extern char** ksh_split_line(char* line);
extern int ksh_launch(char** args);
extern int ksh_execute(char** args);
extern void ksh_loop(void);

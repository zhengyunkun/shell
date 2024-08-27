#pragma once

// Function declarations for built-in shell commands
int ksh_cd(char** args);
int ksh_ls(char** args);
int ksh_pwd(char** args);
int ksh_echo(char** args);
int ksh_cat(char** args);
int ksh_cp(char** args);
int ksh_mv(char** args);
int ksh_mkdir(char** args);
int ksh_rmdir(char** args);
int ksh_rm(char** args);
int ksh_touch(char** args);
int ksh_chmod(char** args);
int ksh_help(char** args);
int ksh_exit(char** args);

// List of built-in commands
extern char* builtin_str[];
extern int (*builtin_func[]) (char**);
// use extern to declare the variables in the header file

// Number of built-in commands
int ksh_num_builtins();
#include "built-in.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

// List of built-in commands
char* builtin_str[] = {
    "cd",
    "ls",
    "pwd",
    "echo",
    "cat",
    "cp",
    "mv",
    "mkdir",
    "rmdir",
    "rm",
    "touch",
    "chmod",
    "help",
    "exit"
};

int (*builtin_func[]) (char**) = {
    &ksh_cd,
    &ksh_ls,
    &ksh_pwd,
    &ksh_echo,
    &ksh_cat,
    &ksh_cp,
    &ksh_mv,
    &ksh_mkdir,
    &ksh_rmdir,
    &ksh_rm,
    &ksh_touch,
    &ksh_chmod,
    &ksh_help,
    &ksh_exit
};

int ksh_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char*);
}

// Shell built-in commands

// 1. cd command
int ksh_cd(char** args)
{
    if (args[1] == NULL) fprintf(stderr, "ksh: expected arguments to \'cd\'...\n");
    else if (chdir(args[1]) != 0) perror("ksh: chdir failed..."); 
    // Use chdir to change the current working directory
    // chdir receive a string as the path
    // if chdir success, return 0, else return -1
    // Use syscall "chdir" to change the current working directory

    // (1) parse the input path, get the inode
    // (2) check if the path is valid
    // (3) check permission of the path
    // (4) change the current working directory by changing the inode of the current process
    return 1;
}

// 2. ls command
void print_file_info(const char* name, const struct stat* st) {
    // (1) Print file type
    printf((S_ISDIR(st->st_mode)) ? "d" : "-");

    // (2) Print file permissions
    printf((st->st_mode & S_IRUSR) ? "r" : "-");
    printf((st->st_mode & S_IWUSR) ? "w" : "-");
    printf((st->st_mode & S_IXUSR) ? "x" : "-");
    printf((st->st_mode & S_IRGRP) ? "r" : "-");
    printf((st->st_mode & S_IWGRP) ? "w" : "-");
    printf((st->st_mode & S_IXGRP) ? "x" : "-");
    printf((st->st_mode & S_IROTH) ? "r" : "-");
    printf((st->st_mode & S_IWOTH) ? "w" : "-");
    printf((st->st_mode & S_IXOTH) ? "x" : "-");

    // (3) Print link count of the file
    printf(" %2ld", st->st_nlink);

    // (4) Print file owner and group
    struct passwd *pw = getpwuid(st->st_uid);
    struct group  *gr = getgrgid(st->st_gid);
    printf(" %-8s %-8s", pw->pw_name, gr->gr_name);

    // (5) Print file size in bytes
    printf(" %10ld", st->st_size);

    // (6) Print file modification time
    char timebuf[80];
    struct tm* timeinfo = localtime(&st->st_mtime);
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", timeinfo);
    printf(" %s", timebuf);

    // (7) Print file name
    printf(" %s\n", name);
}

// supporting "-a" and "-l" options
int ksh_ls(char** args)
{
    if (args[1] == NULL) 
    {
        fprintf(stderr, "ksh: missing directory argument\n");
        exit(EXIT_FAILURE);
    }

    const char* dir_path = ".";
    // Default directory is the current directory "."
    int show_all = 0;
    // "-a" option: show all files, including hidden files
    int long_format = 0;
    // "-l" option: show long format

    // Parse the options
    for (int i = 1; args[i] != NULL; i++) 
    {
        if (strcmp(args[i], "-a") == 0)  show_all = 1;             // show all files
        else if (strcmp(args[i], "-l") == 0) long_format = 1;      // show long format
        else 
        {
            // First non-option argument is the directory path
            dir_path = args[i];
            break;
        }
    }

    // Ensure no more options after directory path
    for (int i = 1; args[i] != NULL; i++) 
    {
        if (strcmp(args[i], "-a") != 0 && strcmp(args[i], "-l") != 0 && i != 1) {
            fprintf(stderr, "ksh: unknown option \'%s\'...\n", args[i]);
            exit(EXIT_FAILURE);
        }
    }
    // Open the specified directory
    DIR* d;
    // DIR is a type representing a directory stream, 
    // contains information about the directory like file name, inode, etc.
    struct dirent* dir;
    // dirent contains all the information about the directory, like file name, inode, etc.
    d = opendir(dir_path);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        // readdir reads all the items in the directory (依次读取所有的目录项)
        {
            // -a option: show all files, including hidden files
            // Hidden files start with a dot "." like ".bashrc"
            // So if show_all is 0, skip the hidden files
            if (!show_all && dir->d_name[0] == '.') continue;
            
            // -l option: show long format
            if (long_format)
            {
                struct stat st;
                // Print detailed information of the file
                if (stat(dir->d_name, &st) == 0) print_file_info(dir->d_name, &st);
                else perror("stat");
            } 
            // Default option: show file name only
            else printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
    else
    {
        perror("ksh: opendir failed...");
        exit(EXIT_FAILURE);
    }
    return 1;
}

// 3. pwd command
int ksh_pwd(char** args)
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n", cwd);
    else perror("ksh: getcwd failed...");
    return 1;
}

// 4. echo command
int ksh_echo(char** args)
{
    for (int i = 1; args[i] != NULL; i++) printf("%s ", args[i]);
    printf("\n");
    return 1;
}

// 5. cat command
int ksh_cat(char** args)
{
    if (args[1] == NULL) 
    {
        fprintf(stderr, "ksh: missing file argument\n");
        exit(EXIT_FAILURE);
    }
    
    // Open and read the file
    for (int i = 1; args[i] != NULL; i++) 
    {
        FILE* file = fopen(args[i], "r");
        if (file == NULL) 
        {
            perror("ksh: fopen failed...");
            exit(EXIT_FAILURE);
        }

        char c;
        while ((c = fgetc(file)) != EOF) putchar(c);
        fclose(file);
    }
    return 1;
}

// 6. cp command
int ksh_cp(char** args)
{
    if (args[1] == NULL || args[2] == NULL) 
    {
        fprintf(stderr, "ksh: missing source and destination arguments\n");
        exit(EXIT_FAILURE);
    }

    // Open the source and destination files
    FILE* src = fopen(args[1], "r");
    if (src == NULL) 
    {
        perror("ksh: fopen failed...");
        exit(EXIT_FAILURE);
    }

    FILE* dest = fopen(args[2], "w");
    if (dest == NULL) 
    {
        perror("ksh: fopen failed...");
        exit(EXIT_FAILURE);
    }

    // Copy the contents of the source file to the destination file
    // One character at a time
    char c;
    while ((c = fgetc(src)) != EOF) fputc(c, dest);
    fclose(src);
    fclose(dest);
    return 1;
}

// 7. mv command
int ksh_mv(char** args)
{
    if (args[1] == NULL || args[2] == NULL) 
    {
        fprintf(stderr, "ksh: missing source and destination arguments\n");
        exit(EXIT_FAILURE);
    }

    // Rename the source file to the destination file
    // Essentially, this is just 
    if (rename(args[1], args[2]) != 0) 
    {
        perror("ksh: rename failed...");
        exit(EXIT_FAILURE);
    }
    return 1;
}

// 8. mkdir command
int ksh_mkdir(char** args)
{
    if (args[1] == NULL) 
    {
        fprintf(stderr, "ksh: missing directory argument\n");
        exit(EXIT_FAILURE);
    }

    // Create a new directory with the specified name with default permissions
    if (mkdir(args[1], 0777) != 0) 
    {
        perror("ksh: mkdir failed...");
        exit(EXIT_FAILURE);
    }
    return 1;
}

// 9. rmdir command
int ksh_rmdir(char** args)
{
    return 1;
}

// 10. rm command
int ksh_rm(char** args)
{
    return 1;
}

// 11. touch command
int ksh_touch(char** args)
{
    return 1;
}

// 12. chmod command
int ksh_chmod(char** args)
{
    return 1;
}

// 13. help command 
int ksh_help(char** args)
{
    printf("Zheng Yunkun's Fisrt Shell, ksh\n");
    printf("Type program names and arguments, and hit enter to execute.\n");
    printf("The following are built-in commands:\n");
    for (int i = 1; i <= ksh_num_builtins(); i ++) printf("(%d)  %s\n", i, builtin_str[i - 1]);

    printf("Use the \'man\' command for information on other programs.\n");
    return 1;
}

//  14. exit command
int ksh_exit(char** args)
{
    return 0;
}
#include "built-in.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
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
    if (args[1] == NULL) 
    {
        fprintf(stderr, "ksh: expected argument to \"rmdir\"\n");
        exit(EXIT_FAILURE);
    }

    // Use rmdir to remove the specified directory
    // If directory is not empty, rmdir will fail and return -1
    // If directory is removed successfully, rmdir will return 0
    if (rmdir(args[1]) != 0) 
    {
        perror("ksh: rmdir failed...");
        exit(EXIT_FAILURE);
    }

    return 1;
}

// 10. rm command with four options: 
// (1) -r remove directories and their contents recursively,
// (2) -f remove files without prompting, which means remove files forcefully,
// (3) -v remove files verbosely, which means print the name of each file before removing it,
// (4) -i remove files interactively, which means prompt before every removal.
int remove_directory(const char* path, int force, int verbose, int interactive)
{
    DIR* d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1; // return value

    // 1. Remove all the content in path
    if (d)  // Open the directory
    {
        struct dirent* dir;
        r = 0; // Initialize the return value
        while (!r && (dir = readdir(d)))
        // readdir reads all the items in the directory (依次读取所有的目录项)
        {
            int r2 = -1;
            char* buf;
            size_t len;

            // Skip the "." and ".." directories
            // "." is the current directory
            // ".." is the parent directory
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;

            len = path_len + strlen(dir->d_name) + 2;
            // add '/' and '\0' to the length to store the full path
            buf = malloc(len);

            // If the memory allocation is successful
            if (buf)
            {
                struct stat statbuf;
                // stat struct is used to store the information of the file like:
                // (1) st_dev: ID of device containing file
                // (2) st_ino: inode number
                // (3) st_mode: protection
                // (4) st_nlink: number of hard links
                // (5) st_uid: user ID of owner
                // (6) st_gid: group ID of owner
                // (7) st_size: total size, in bytes
                // (8) st_blksize: blocksize for file system I/O
                // (9) st_blocks: number of 512B blocks allocated
                // (10) st_atime: time of last access
                // (11) st_mtime: time of last modification
                // (12) st_ctime: time of last status change

                snprintf(buf, len, "%s/%s", path, dir->d_name);
                // write the full path to the buffer
                // * snprintf will automatically add a '\0' in the end of the string
                // * so the former len is right

                if (!stat(buf, &statbuf))
                {
                    // If the file is a directory, remove it recursively
                    if (S_ISDIR(statbuf.st_mode))
                    {
                        r2 = remove_directory(buf, force, verbose, interactive);
                        // call the remove_directory function recursively
                        // to remove the directory and all its contents
                    }
                    else
                    // If the file is a regular file
                    {
                        if (interactive)
                        {
                            int response;
                            do
                            {
                                printf("rm: remove file '%s'? ", buf);
                                response = getchar();
                                while (getchar() != '\n');
                                // Clear the input buffer, only keep the first character
                            } while (response != 'Y' && response != 'y' && response != 'N' && response != 'n');

                            if (response == 'N' || response == 'n')
                            {
                                // Skip the current file
                                continue;
                            }

                            r2 = unlink(buf);
                            // Delete the file
                            if (verbose && r2 == 0) printf("removed file '%s'\n", buf);
                        }
                    }
                }
                free(buf);  
            }
            r = r2;
        }
        closedir(d);
    }

    // 2. Remove the directory itself
    if (!r)
    {
        if (interactive)
        {
            int response;
            do
            {
                printf("rm: remove file '%s'? ", path);
                response = getchar();
                while (getchar() != '\n');
                // Clear the input buffer, only keep the first character
            } while (response != 'Y' && response != 'y' && response != 'N' && response != 'n');
        }
        r = rmdir(path);
        if (verbose && r == 0) printf("removed directory '%s'\n", path);
    }

    return r;
}

int ksh_rm(char** args)
{
    int recursive = 0;
    int force = 0;
    int verbose = 0;
    int interactive = 0;
    int i = 1;              
    // Start from the first argument

    // Parse the options
    while (args[i] != NULL && args[i][0] == '-')
    {
        if (strcmp(args[i], "-r") == 0 || strcmp(args[i], "--recursive") == 0) recursive = 1;
        else if (strcmp(args[i], "-f") == 0 || strcmp(args[i], "--force") == 0) force = 1;
        else if (strcmp(args[i], "-v") == 0 || strcmp(args[i], "--verbose") == 0) verbose = 1;
        else if (strcmp(args[i], "-i") == 0 || strcmp(args[i], "--interactive") == 0) interactive = 1;
        else 
        {
            fprintf(stderr, "ksh: unknown option \'%s\'...\n", args[i]);
            exit(EXIT_FAILURE);
        }
        i ++ ;
    }

    // Ensure whether there is at least one file or directory to remove
    if (args[i] == NULL)
    {
        fprintf(stderr, "ksh: missing file or directory argument\n");
        exit(EXIT_FAILURE);
    }

    // Remove all the provided files and directories
    for (; args[i] != NULL; i ++ )
    {
        struct stat statbuf;
        // Use statbuf to store the information of the file
        if (stat(args[i], &statbuf) != 0)
        // If get the information, return 0
        // If not, return -1
        {
            if (!force)
            {
                perror("ksh: stat failed...");
                exit(EXIT_FAILURE);
            }
            else continue;
        }

        // 1. If it's a directory
        if (S_ISDIR(statbuf.st_mode))
        {
            if (recursive)
            {
                if (remove_directory(args[i], force, verbose, interactive) != 0)
                {
                    if (!force)
                    {
                        perror("ksh: remove_directory failed...");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            else    
            // If the file is a dir, but recursive option is not set
            {
                fprintf(stderr, "ksh: cannot remove \'%s\': Is a directory\n", args[i]);
                exit(EXIT_FAILURE);
            }
        }
        // 2. If the file is a regular file
        else
        {
            if (interactive)
            {
                int response;
                do
                {
                    printf("rm: remove file '%s'? ", args[i]);
                    response = getchar();
                    while (getchar() != '\n');
                    // Clear the input buffer, only keep the first character
                } while (response != 'Y' && response != 'y' && response != 'N' && response != 'n');

                if (response == 'N' || response == 'n')
                {
                    // Skip the current file
                    continue;
                }
            }

            // Use unlink to remove the file
            if (unlink(args[i]) != 0)
            {
                if (!force)
                {
                    perror("ksh: unlink failed...");
                    exit(EXIT_FAILURE);
                }
            }
            else if (verbose) printf("removed '%s'\n", args[i]);
        }
    }

    return 1;
}

// 11. touch command
int ksh_touch(char** args)
{
    if (args[1] == NULL) 
    {
        fprintf(stderr, "Usage: touch <file>\n");
        exit(EXIT_FAILURE);
    }

    const char* filepath = args[1];

    // Try to update the file times
    // If the files does exist, update the access and modification time
    if (utime(filepath, NULL) != 0)
    {
        // If the file does not exist, create it
        int fd = open(filepath, O_CREAT | O_WRONLY, 0644);
        if (fd < 0) 
        {
            perror("touch failed...");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }

    return 1;
}

// 12. chmod command
int ksh_chmod(char** args)
{
    if (args[1] == NULL || args[2] == NULL) 
    {
        fprintf(stderr, "Usage: chmod <mode> <file>\n");
        exit(EXIT_FAILURE);
    }

    // Convert mode from string to octal
    mode_t mode = strtol(args[1], NULL, 8);
    if (mode == 0 && args[1][0] != '0') 
    {
        fprintf(stderr, "Invalid mode: %s\n", args[1]);
        exit(EXIT_FAILURE);
    }

    // Change the file mode
    if (chmod(args[2], mode) != 0) 
    {
        perror("chmod failed...\n");
        exit(EXIT_FAILURE);
    }

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
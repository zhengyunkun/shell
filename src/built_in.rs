use std::env;
use std::path::{Path, PathBuf};

// List of built-in commands
pub const BUILTIN_STR: [&str; 14] = [
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
    "exit",
];

// Define a type for built-in functions
pub type BuiltinFunc = fn(args: Vec<String>) -> i32;

// List of built-in functions
pub const BUILTIN_FUNC: [BuiltinFunc; 14] = [
    ksh_cd,
    ksh_ls,
    ksh_pwd,
    ksh_echo,
    ksh_cat,
    ksh_cp,
    ksh_mv,
    ksh_mkdir,
    ksh_rmdir,
    ksh_rm,
    ksh_touch,
    ksh_chmod,
    ksh_help,
    ksh_exit,
];

pub fn ksh_num_builtins() -> usize {
    BUILTIN_STR.len()
}



// Function declarations for built-in shell commands
pub fn ksh_cd(args: Vec<String>) -> i32 {
    if args.len() < 2 {
        eprintln!("ksh: expected arguments to 'cd'...");
    }

    if let Err(e) = env::set_current_dir(&Path::new(&args[1])) {
        eprintln!("ksh: chdir failed: {}", e);
    }

    return 1;
}

pub fn ksh_ls(args: Vec<String>) -> i32 {
    let _ = args;
    // Implementation of ls command
    0
}

pub fn ksh_pwd(args: Vec<String>) -> i32 {
    let _ = args;
    // Implementation of pwd command
    0
}

pub fn ksh_echo(args: Vec<String>) -> i32 {
    let _ = args;
    // Implementation of echo command
    0
}

pub fn ksh_cat(args: Vec<String>) -> i32 {
    let _ = args;
    // Implementation of cat command
    0
}

pub fn ksh_cp(args: Vec<String>) -> i32 {
    let _ = args;
    // Implementation of cp command
    0
}

pub fn ksh_mv(args: Vec<String>) -> i32 {
    let _ = args;
    // Implementation of mv command
    0
}

pub fn ksh_mkdir(args: Vec<String>) -> i32 {
    let _ = args;
    // Implementation of mkdir command
    0
}

pub fn ksh_rmdir(args: Vec<String>) -> i32 {
    let _ = args;
    // Implementation of rmdir command
    0
}

pub fn ksh_rm(args: Vec<String>) -> i32 {
    let _ = args;
    // Implementation of rm command
    0
}

pub fn ksh_touch(args: Vec<String>) -> i32 {
    let _ = args;
    // Implementation of touch command
    0
}

pub fn ksh_chmod(args: Vec<String>) -> i32 {
    let _ = args;
    // Implementation of chmod command
    0
}

pub fn ksh_help(args: Vec<String>) -> i32 {
    let _ = args;
    let cwd: PathBuf = match env::current_dir() {
        Ok(path) => path,
        Err(_) => {
            eprintln!("ksh: getcwd failed...");
            return 1;
        }
    };

    let username = match env::var("USER") {
        Ok(user) => user,
        Err(_) => String::from("unknown"),
    };

    let num_builtins = ksh_num_builtins();
    let columns = 4;
    let width = 15;

    println!("******************************************************************************");
    println!("*                                                                            *");
    println!("*                    Welcome to Zheng Yunkun's First Shell!                  *");
    println!("*                                    ksh                                     *");
    println!("*                                                                            *");
    println!("******************************************************************************\n");
    println!("Type program names and arguments, and hit enter to execute.");
    println!("The following are built-in commands:\n");

    // Print top border of the table
    print!("        +");
    for _ in 0..columns {
        print!("---------------+");
    }
    println!();

    // Print commands in table format
    for i in 1..=num_builtins {
        if i % columns == 1 {
            print!("        ");
        }
        print!("|  ({}){}  \x1b[0;31m{:<width$}\x1b[0m", i, if i < 10 { " " } else { "" }, BUILTIN_STR[i - 1], width = width - 8);
        if i % columns == 0 {
            println!("|");
            print!("        +");
            for _ in 0..columns {
                print!("---------------+");
            }
            println!();
        }
    }

    // Print bottom border if the last row is not complete
    if num_builtins % columns != 0 {
        for _ in 0..columns - (num_builtins % columns) {
            print!("|               ");
        }
        println!("|");
        print!("        +");
        for _ in 0..columns {
            print!("---------------+");
        }
        println!();
    }

    println!("\nUse the 'man' command for information on other programs.");
    println!("\nExamples:");
    println!("  \x1b[35m{}\x1b[0m in \x1b[32m{}\x1b[0m \x1b[33mλ\x1b[0m ls -l", username, cwd.display());
    println!("  \x1b[35m{}\x1b[0m in \x1b[32m{}\x1b[0m \x1b[33mλ\x1b[0m cd /home/user", username, cwd.display());
    println!("  \x1b[35m{}\x1b[0m in \x1b[32m{}\x1b[0m \x1b[33mλ\x1b[0m echo \"Hello, World!\"", username, cwd.display());

    println!("\nTips:");
    println!("  - Use 'cd' to change directories.");
    println!("  - Use 'exit' to quit the shell.");
    println!("  - Use 'help' to see this message again.");

    println!("\n******************************************************************************");
    println!("*                                  Bye-bye                                   *");
    println!("******************************************************************************");

    1
}

pub fn ksh_exit(args: Vec<String>) -> i32 {
    let _ = args;
    // Implementation of exit command
    return 0
}
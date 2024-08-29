mod built_in;

use std::io::{self, Write};
use std::env;
use std::path::PathBuf;
use std::process::Command;

use built_in::{BUILTIN_FUNC, BUILTIN_STR};

const KSH_RL_BUFSIZE: usize = 1024;
// 'usize' is unsigned, 1KB of memory
const KSH_TOKEN_DELIMITERS: &str = " \t\r\n\x07";

// 1. Read a line from stdin
fn ksh_read_line() -> String {
    let mut buffer = String::with_capacity(KSH_RL_BUFSIZE);
    // let mut means buffer is mutable, let is immutable

    io::stdin().read_line(&mut buffer).unwrap_or_else(|err| {
        eprintln!("ksh: read_line failed: {}", err);
        std::process::exit(1);
    });
    // Read a line from stdin and store it in buffer
    // If failed, use unwrap_or_else to print error message and exit
    // closure receives one argument, 'err'
    // read_line returns a Result, so we use unwrap_or_else to handle the error

    return buffer.trim_end().to_string();
    // Remove trailing whitespace and return buffer as a String
}

// 2. Parse the line into tokens
fn ksh_split_line(line: &str) -> Vec<String> {
    let tokens: Vec<String> = line
        .split(|c| KSH_TOKEN_DELIMITERS.contains(c))
         // Passing a closure to split, |c| is the argument, others is the closure body
         // (1) split() judges the character in 'line' one by one, 
         // (2) if it is in KSH_TOKEN_DELIMITERS, it will be split
         // (3) if not, it will be kept
         // (4) return a iterator 
        .map(|s| s.to_string())
        .collect();

    return tokens;
}

// 3. Execute the command (not built-in type)
fn ksh_launch(args: Vec<String>) -> i32 {
    if args.is_empty() {
        eprintln!("ksh: launch: no arguments provided");
        return 1;
    }

    // Get the command from the first argument
    let command = args[0].clone();
    let mut child = match Command::new(command)
        .args(&args[1..])
        .spawn() {
            Ok(child) => child,
            Err(err) => {
                eprintln!("ksh: launch: failed to execute command: {}", err);
                return 1;
            }
        };

    match child.wait() {
        Ok(status) => {
            if status.success() {
                return 1
            } else {
                eprintln!("ksh: process exited with status: {}", status);
                return 1;
            }
        },
        Err(e) => {
            eprintln!("ksh: wait failed: {}", e);
            return 1;
        }
    }
}

// 4. Execute the command (built-in type)
fn ksh_execute(args: Vec<String>) -> i32 {
    // User typed in nothing, return 1 and continue
    if args.is_empty() {
        return 1;
    }

    for i in 0..BUILTIN_STR.len() {
        if args[0] == BUILTIN_STR[i] {
            return BUILTIN_FUNC[i](args.clone());
        }
    }

    // If the command is not built-in, execute it
    return ksh_launch(args);
}

// 5. Main loop
fn ksh_loop() {
    let mut status = 1;

    while status != 0 {
        let cwd = env::current_dir().unwrap_or_else(|_| PathBuf::from("unknown"));
        let username = env::var("USER").unwrap_or_else(|_| "unknow".to_string());

        print!("\x1b[35m{}\x1b[0m in \x1b[32m{}\x1b[0m \x1b[33mλ\x1b[0m ", username, cwd.display());
        io::stdout().flush().unwrap();

        let line = ksh_read_line();
        let args = ksh_split_line(&line);
        status = ksh_execute(args);
    }
}

fn main() {
    // Start the shell loop
    ksh_loop();
    std::process::exit(0);
}
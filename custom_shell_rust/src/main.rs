use std::io::{stdin, stdout, Write};
use std::process::{Command, Stdio, Child}; // system commands
use std::env;
use std::path::Path;

fn main() {
    loop {
        print!("🐱 ");
        let _ = stdout().flush(); // flush output buffer

        let mut input = String::new(); // input string, mut->mutable
    
        // reads line from stdin and appends to "input"
        // &mut passes it as reference, mutable
        stdin().read_line(&mut input).unwrap(); 

        let mut commands = input.trim().split(" | ").peekable();
        let mut previous_command = None;

        while let Some(command) = commands.next() { // different commands in the piping

            let mut parts = command.trim().split_whitespace();
            let command = parts.next().unwrap(); // makes sure there is atleast 1 word
            let args = parts; // holds the rest of the command -> arguments
        
            match command {
                "cd" => { // cd cannot be sent to another process
                    // peekable lets you look at next element without consumingz
                    // peek is like next
                    // map_or uses / is value is none, |x| *x dereferences x to get the value
                    let new_dir = args.peekable().peek().map_or("/", |x| *x);
                    let root = Path::new(new_dir);
                    if let Err(e) = env::set_current_dir(&root) {
                        eprintln!("{}", e);
                    }
                    previous_command = None;
                },
                "exit" => return,
                command => {
                    let stdin = previous_command
                        .map_or(
                            Stdio::inherit(),
                            |output: Child| Stdio::from(output.stdout.unwrap())
                        );
                    let stdout = if commands.peek().is_some() {
                        Stdio::piped()
                    }
                    else {
                        Stdio::inherit()
                    };
                    let output = Command::new(command)
                        .args(args)
                        .stdin(stdin)
                        .stdout(stdout)
                        .spawn();
                    
                    match output {
                        Ok(output) => { previous_command = Some(output); },
                        Err(e) => {
                            previous_command = None;
                            eprintln!("{}", e);
                        },
                    };
                }
            }

        }

        if let Some(mut final_command) = previous_command {
            final_command.wait();
        }
    }
}

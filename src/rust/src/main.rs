use chrono::Local;
use flate2::read::GzDecoder;
use glob::glob;
use std::env;
use std::fs::{self, File};
use std::io::{self, BufRead, BufReader, Write};
use std::path::PathBuf;
use std::process;

struct Config {
    accepted: bool,
    failed: bool,
    method: String,
    output_to_file: bool,
}

fn print_usage(prog_name: &str) {
    println!("Usage: {} [-a] [-f] [-m method] [-o]", prog_name);
    println!("  -a         Show only accepted logins");
    println!("  -f         Show only failed logins");
    println!("  -m method  Filter by authentication method (e.g., password, publickey)");
    println!("  -o         Save output to ~/ssh-logs in a timestamped file");
    println!("  -h         Show this help menu");
}

fn parse_args() -> Config {
    let args: Vec<String> = env::args().collect();
    let mut config = Config {
        accepted: false,
        failed: false,
        method: String::new(),
        output_to_file: false,
    };

    let mut i = 1;
    while i < args.len() {
        match args[i].as_str() {
            "-a" => config.accepted = true,
            "-f" => config.failed = true,
            "-m" => {
                if i + 1 < args.len() {
                    config.method = args[i + 1].clone();
                    i += 1;
                } else {
                    eprintln!("Error: -m requires a method argument.");
                    process::exit(1);
                }
            }
            "-o" => config.output_to_file = true,
            "-h" => {
                print_usage(&args[0]);
                process::exit(0);
            }
            _ => {
                print_usage(&args[0]);
                process::exit(1);
            }
        }
        i += 1;
    }
    config
}

fn process_file(filepath: &str, out_file: &mut dyn Write, config: &Config) {
    // Open the file normally or via GzDecoder if it ends with .gz
    let reader: Box<dyn BufRead> = if filepath.ends_with(".gz") {
        match File::open(filepath) {
            Ok(file) => Box::new(BufReader::new(GzDecoder::new(file))),
            Err(e) => {
                eprintln!("Warning: Could not open {}: {}", filepath, e);
                return;
            }
        }
    } else {
        match File::open(filepath) {
            Ok(file) => Box::new(BufReader::new(file)),
            Err(e) => {
                eprintln!("Warning: Could not open {}: {}", filepath, e);
                return;
            }
        }
    };

    // Read line by line
    for line_result in reader.lines() {
        // Ignore lines that cannot be read (e.g., binary garbage)
        let line = match line_result {
            Ok(l) => l,
            Err(_) => continue,
        };

        if !line.contains("sshd") {
            continue;
        }

        let mut is_match = true;

        if config.accepted && !line.contains("Accepted") {
            is_match = false;
        }
        if config.failed && !line.contains("Failed") {
            is_match = false;
        }
        if !config.method.is_empty() && !line.contains(&config.method) {
            is_match = false;
        }

        if is_match {
            if let Err(e) = writeln!(out_file, "{}", line) {
                eprintln!("Error writing to output: {}", e);
                break;
            }
        }
    }
}

fn main() {
    let config = parse_args();

    // Determine output destination (stdout or file)
    let mut out_file: Box<dyn Write> = if config.output_to_file {
        if let Some(mut log_dir) = dirs::home_dir() {
            log_dir.push("ssh-logs");

            // Attempt to create the directory
            if let Err(e) = fs::create_dir(&log_dir) {
                if e.kind() != io::ErrorKind::AlreadyExists {
                    eprintln!("Error: Could not create directory {:?}: {}", log_dir, e);
                    process::exit(1);
                }
            }

            // Generate timestamp for filename
            let timestamp = Local::now().format("%Y%m%d_%H%M%S");
            let filename = format!("sshlog_{}.txt", timestamp);
            log_dir.push(filename);

            // Create output file
            match File::create(&log_dir) {
                Ok(file) => {
                    println!("Output being saved to: {:?}", log_dir);
                    Box::new(file)
                }
                Err(e) => {
                    eprintln!("Error: Could not create output file: {}", e);
                    process::exit(1);
                }
            }
        } else {
            eprintln!("Warning: Could not find user's home directory. Defaulting to standard output.");
            Box::new(io::stdout())
        }
    } else {
        Box::new(io::stdout())
    };

    // Find and process all matching log files
    let mut files_found = false;
    
    // Read the glob pattern
    match glob("/var/log/auth.log*") {
        Ok(paths) => {
            for entry in paths.flatten() {
                files_found = true;
                let filepath = entry.to_string_lossy();
                
                // Optional: Print the file being processed if not writing to stdout
                if config.output_to_file {
                    println!("Processing: {}", filepath);
                }
                
                process_file(&filepath, &mut out_file, &config);
            }
        }
        Err(e) => eprintln!("Error reading glob pattern: {}", e),
    }

    if !files_found {
        eprintln!("Error: Found no files matching /var/log/auth.log*");
    }
}

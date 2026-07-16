#!/usr/bin/env python3
import sys
import os
import glob
import gzip
import getopt
from datetime import datetime
from pathlib import Path

def print_usage(prog_name):
    print(f"Usage: {prog_name} [-a] [-f] [-m method] [-o]")
    print("  -a         Show only accepted logins")
    print("  -f         Show only failed logins")
    print("  -m method  Filter by authentication method (e.g., password, publickey)")
    print("  -o         Save output to ~/ssh-logs in a timestamped file")
    print("  -h         Show this help menu")

def process_file(filepath, out_file, config):
    try:
        # Transparently handle .gz files using Python's built-in gzip module
        if filepath.endswith(".gz"):
            f = gzip.open(filepath, 'rt', encoding='utf-8', errors='replace')
        else:
            f = open(filepath, 'r', encoding='utf-8', errors='replace')
    except Exception as e:
        print(f"Warning: Could not open {filepath}: {e}", file=sys.stderr)
        return

    with f:
        for line in f:
            if "sshd" not in line:
                continue

            match = True
            
            if config['accepted'] and "Accepted" not in line:
                match = False
            if config['failed'] and "Failed" not in line:
                match = False
            if config['method'] and config['method'] not in line:
                match = False

            if match:
                out_file.write(line)

def main():
    config = {
        'accepted': False,
        'failed': False,
        'method': "",
        'output_to_file': False
    }

    try:
        opts, args = getopt.getopt(sys.argv[1:], "afm:oh")
    except getopt.GetoptError as err:
        print(f"Error: {err}", file=sys.stderr)
        print_usage(sys.argv[0])
        sys.exit(1)

    for opt, arg in opts:
        if opt == '-a':
            config['accepted'] = True
        elif opt == '-f':
            config['failed'] = True
        elif opt == '-m':
            config['method'] = arg
        elif opt == '-o':
            config['output_to_file'] = True
        elif opt == '-h':
            print_usage(sys.argv[0])
            sys.exit(0)

    out_file = sys.stdout

    # Handle creation of output directory and file if '-o' is provided
    if config['output_to_file']:
        try:
            # Path.home() safely resolves the user's home directory on all platforms
            log_dir = Path.home() / "ssh-logs"
            log_dir.mkdir(mode=0o755, parents=True, exist_ok=True)

            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            out_path = log_dir / f"sshlog_{timestamp}.txt"
            
            out_file = open(out_path, "w", encoding="utf-8")
            print(f"Output being saved to: {out_path}")
        except Exception as e:
            print(f"Error: Could not create output file: {e}", file=sys.stderr)
            sys.exit(1)

    # Find all files matching /var/log/auth.log*
    files = glob.glob("/var/log/auth.log*")
    
    if not files:
        print("Error: Found no files matching /var/log/auth.log*", file=sys.stderr)
    else:
        for filepath in files:
            # Optional: Print the current file being processed if not writing to stdout
            if out_file is not sys.stdout:
                print(f"Processing: {filepath}")
            
            process_file(filepath, out_file, config)

    # Clean up
    if out_file is not sys.stdout:
        out_file.close()

if __name__ == "__main__":
    main()

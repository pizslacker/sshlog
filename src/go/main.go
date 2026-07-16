package main

import (
	"bufio"
	"compress/gzip"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"time"
)

// Config holds the filtering rules and output preferences
type Config struct {
	Accepted     bool
	Failed       bool
	Method       string
	OutputToFile bool
}

// processFile reads a single log file (transparently handling .gz) and applies filters
func processFile(filePath string, outWriter io.Writer, cfg Config) {
	file, err := os.Open(filePath)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Warning: Could not open %s: %v\n", filePath, err)
		return
	}
	defer file.Close()

	var reader io.Reader = file

	// Transparently decompress if the file ends with .gz
	if strings.HasSuffix(filePath, ".gz") {
		gzReader, err := gzip.NewReader(file)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Warning: Could not read gzip file %s: %v\n", filePath, err)
			return
		}
		defer gzReader.Close()
		reader = gzReader
	}

	// Read line by line efficiently using a Scanner
	scanner := bufio.NewScanner(reader)
	for scanner.Scan() {
		line := scanner.Text()

		// Ignore lines not related to sshd
		if !strings.Contains(line, "sshd") {
			continue
		}

		match := true

		if cfg.Accepted && !strings.Contains(line, "Accepted") {
			match = false
		}
		if cfg.Failed && !strings.Contains(line, "Failed") {
			match = false
		}
		if cfg.Method != "" && !strings.Contains(line, cfg.Method) {
			match = false
		}

		if match {
			fmt.Fprintln(outWriter, line)
		}
	}

	if err := scanner.Err(); err != nil {
		fmt.Fprintf(os.Stderr, "Warning: Error reading %s: %v\n", filePath, err)
	}
}

func main() {
	// Setup command-line flags
	accepted := flag.Bool("a", false, "Show only accepted logins")
	failed := flag.Bool("f", false, "Show only failed logins")
	method := flag.String("m", "", "Filter by authentication method (e.g., password, publickey)")
	outputToFile := flag.Bool("o", false, "Save output to ~/ssh-logs in a timestamped file")

	// Custom usage message
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, "Usage: %s [-a] [-f] [-m method] [-o]\n", os.Args[0])
		fmt.Fprintln(os.Stderr, "  -a         Show only accepted logins")
		fmt.Fprintln(os.Stderr, "  -f         Show only failed logins")
		fmt.Fprintln(os.Stderr, "  -m method  Filter by authentication method (e.g., password, publickey)")
		fmt.Fprintln(os.Stderr, "  -o         Save output to ~/ssh-logs in a timestamped file")
		fmt.Fprintln(os.Stderr, "  -h         Show this help menu")
	}
	flag.Parse()

	cfg := Config{
		Accepted:     *accepted,
		Failed:       *failed,
		Method:       *method,
		OutputToFile: *outputToFile,
	}

	var outWriter io.Writer = os.Stdout

	// Handle output file creation if '-o' is set
	if cfg.OutputToFile {
		home, err := os.UserHomeDir()
		if err != nil {
			fmt.Fprintf(os.Stderr, "Warning: Could not find user's home directory. Defaulting to standard output.\n")
			cfg.OutputToFile = false
		} else {
			logDir := filepath.Join(home, "ssh-logs")
			
			// MkdirAll is the equivalent of 'mkdir -p'
			if err := os.MkdirAll(logDir, 0755); err != nil {
				fmt.Fprintf(os.Stderr, "Error: Could not create directory %s: %v\n", logDir, err)
				os.Exit(1)
			}

			// In Go, time formats use the magical reference date: "Mon Jan 2 15:04:05 MST 2006"
			timestamp := time.Now().Format("20060102_150405")
			outPath := filepath.Join(logDir, fmt.Sprintf("sshlog_%s.txt", timestamp))

			outFile, err := os.Create(outPath)
			if err != nil {
				fmt.Fprintf(os.Stderr, "Error: Could not create output file: %v\n", err)
				os.Exit(1)
			}
			defer outFile.Close() // Ensure file is closed when main() exits

			outWriter = outFile
			fmt.Printf("Output being saved to: %s\n", outPath)
		}
	}

	// Find all matching log files
	files, err := filepath.Glob("/var/log/auth.log*")
	if err != nil || len(files) == 0 {
		fmt.Fprintln(os.Stderr, "Error: Found no files matching /var/log/auth.log*")
		os.Exit(1)
	}

	// Process each file
	for _, file := range files {
		// Optional: Print progress to stdout if the actual data is going to a file
		if cfg.OutputToFile {
			fmt.Printf("Processing: %s\n", file)
		}
		processFile(file, outWriter, cfg)
	}
}

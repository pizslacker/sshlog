#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <pwd.h>
#include <time.h>
#include <errno.h>
#include <glob.h>

#define MAX_LINE 2048

// Global state for our custom pager
int current_line = 0;
FILE *tty = NULL;

void print_usage(const char *prog_name) {
    printf("Usage: %s [-a] [-f] [-m method] [-o]\n", prog_name);
    printf("  -a         Show only accepted logins\n");
    printf("  -f         Show only failed logins\n");
    printf("  -m method  Filter by authentication method (e.g., password, publickey)\n");
    printf("  -o         Save output to ~/ssh-logs in a timestamped file\n");
    printf("  -h         Show this help menu\n");
}

// Function to handle terminal pagination
// Returns 1 to continue printing, 0 if the user pressed 'q' to quit
int paginate() {
    // If output is not going to the terminal, skip pagination
    if (!tty) return 1;

    int term_height = 24; // Default fallback
    struct winsize w;
    
    // Dynamically fetch the current terminal height (handles window resizing)
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_row > 0) {
        term_height = w.ws_row;
    }

    current_line++;
    
    // Pause when we reach the bottom of the screen (-1 to leave room for the prompt)
    if (current_line >= term_height - 1) {
        // Print inverted colors for the prompt
        fprintf(tty, "\033[7m-- More -- (Press Space for next page, 'q' to quit)\033[0m");
        fflush(tty);

        struct termios old_t, new_t;
        tcgetattr(fileno(tty), &old_t);
        new_t = old_t;
        
        // Turn off canonical mode (wait for Enter) and echo (printing the keys)
        new_t.c_lflag &= ~(ICANON | ECHO); 
        tcsetattr(fileno(tty), TCSANOW, &new_t);

        int c;
        int continue_printing = 1;
        while (1) {
            c = fgetc(tty);
            if (c == 'q' || c == 'Q') {
                continue_printing = 0;
                break;
            }
            if (c == ' ' || c == '\n' || c == '\r') {
                break; // Space or Enter goes to the next page
            }
        }

        // Restore normal terminal behavior
        tcsetattr(fileno(tty), TCSANOW, &old_t);
        
        // Clear the prompt line using ANSI escape codes (\r carriage return, \033[K clear line)
        fprintf(tty, "\r\033[K"); 
        
        // Reset line counter for the next page
        current_line = 0; 
        
        return continue_printing;
    }
    return 1;
}

// Returns 1 to continue processing files, 0 if user aborted
int process_file(const char *filepath, FILE *out_file, int filter_accepted, int filter_failed, const char *filter_method) {
    FILE *log_file = NULL;
    int is_gz = 0;

    size_t len = strlen(filepath);
    if (len > 3 && strcmp(filepath + len - 3, ".gz") == 0) {
        is_gz = 1;
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "zcat \"%s\" 2>/dev/null", filepath);
        log_file = popen(cmd, "r");
    } else {
        log_file = fopen(filepath, "r");
    }

    if (!log_file) {
        fprintf(stderr, "Warning: Could not open %s\n", filepath);
        return 1;
    }

    char line[MAX_LINE];
    int continue_processing = 1;

    while (fgets(line, sizeof(line), log_file)) {
        if (strstr(line, "sshd") == NULL) continue;

        int match = 1;
        if (filter_accepted && strstr(line, "Accepted") == NULL) match = 0;
        if (filter_failed && strstr(line, "Failed") == NULL) match = 0;
        if (filter_method[0] != '\0' && strstr(line, filter_method) == NULL) match = 0;

        if (match) {
            fprintf(out_file, "%s", line);
            
            // If writing to the terminal, check if we need to paginate
            if (out_file == stdout) {
                if (!paginate()) {
                    continue_processing = 0;
                    break;
                }
            }
        }
    }

    if (is_gz) {
        pclose(log_file);
    } else {
        fclose(log_file);
    }
    
    return continue_processing;
}

int main(int argc, char *argv[]) {
    int opt;
    int filter_accepted = 0;
    int filter_failed = 0;
    char filter_method[256] = {0};
    int output_to_file = 0;

    while ((opt = getopt(argc, argv, "afm:oh")) != -1) {
        switch (opt) {
            case 'a': filter_accepted = 1; break;
            case 'f': filter_failed = 1; break;
            case 'm': strncpy(filter_method, optarg, sizeof(filter_method) - 1); break;
            case 'o': output_to_file = 1; break;
            case 'h': 
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    FILE *out_file = stdout;

    if (output_to_file) {
        const char *homedir = getenv("HOME");
        if (!homedir) {
            struct passwd *pw = getpwuid(getuid());
            if (pw) homedir = pw->pw_dir;
        }
        
        if (homedir) {
            char log_dir[512];
            snprintf(log_dir, sizeof(log_dir), "%s/ssh-logs", homedir);
            
            if (mkdir(log_dir, 0755) == -1 && errno != EEXIST) {
                perror("Error: Could not create directory ~/ssh-logs");
                return EXIT_FAILURE;
            }

            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y%m%d_%H%M%S", tm_info);

            char out_path[1024];
            snprintf(out_path, sizeof(out_path), "%s/sshlog_%s.txt", log_dir, time_buf);
            
            out_file = fopen(out_path, "w");
            if (!out_file) {
                perror("Error: Could not create output file");
                return EXIT_FAILURE;
            }
            printf("Output being saved to: %s\n", out_path);
        } else {
            fprintf(stderr, "Warning: Could not find user's home directory. Defaulting to standard output.\n");
            output_to_file = 0; 
        }
    }

    // Initialize the pager if we are writing to a terminal
    if (out_file == stdout && isatty(STDOUT_FILENO)) {
        // Open /dev/tty directly to ensure we can read keystrokes 
        // even if someone pipes data into our program later.
        tty = fopen("/dev/tty", "r+");
    }

    glob_t glob_result;
    if (glob("/var/log/auth.log*", 0, NULL, &glob_result) == 0) {
        for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
            if (out_file != stdout) {
                printf("Processing: %s\n", glob_result.gl_pathv[i]);
            }
            
            // If process_file returns 0, the user pressed 'q'
            if (!process_file(glob_result.gl_pathv[i], out_file, filter_accepted, filter_failed, filter_method)) {
                break; 
            }
        }
    } else {
        fprintf(stderr, "Error: Found no files matching /var/log/auth.log*\n");
    }

    globfree(&glob_result);

    if (tty) fclose(tty);
    if (out_file != stdout) fclose(out_file);

    return EXIT_SUCCESS;
}

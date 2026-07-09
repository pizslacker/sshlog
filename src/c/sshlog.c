#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>
#include <errno.h>
#include <glob.h>

#define MAX_LINE 2048

void print_usage(const char *prog_name) {
    printf("Usage: %s [-a] [-f] [-m method] [-o]\n", prog_name);
    printf("  -a         Show only accepted logins\n");
    printf("  -f         Show only failed logins\n");
    printf("  -m method  Filter by authentication method (e.g., password, publickey)\n");
    printf("  -o         Save output to ~/ssh-logs in a timestamped file\n");
    printf("  -h         Show this help menu\n");
}

void process_file(const char *filepath, FILE *out_file, int filter_accepted, int filter_failed, const char *filter_method) {
    FILE *log_file = NULL;
    int is_gz = 0;

    // Check if the file is a gzip file based on the extension
    size_t len = strlen(filepath);
    if (len > 3 && strcmp(filepath + len - 3, ".gz") == 0) {
        is_gz = 1;
        char cmd[1024];
        // Use zcat to read compressed files on the fly
        snprintf(cmd, sizeof(cmd), "zcat \"%s\" 2>/dev/null", filepath);
        log_file = popen(cmd, "r");
    } else {
        log_file = fopen(filepath, "r");
    }

    if (!log_file) {
        fprintf(stderr, "Warning: Could not open %s\n", filepath);
        return;
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), log_file)) {
        // Ignore lines that do not relate to sshd
        if (strstr(line, "sshd") == NULL) {
            continue;
        }

        int match = 1;
        
        // Apply filters
        if (filter_accepted && strstr(line, "Accepted") == NULL) match = 0;
        if (filter_failed && strstr(line, "Failed") == NULL) match = 0;
        if (filter_method[0] != '\0' && strstr(line, filter_method) == NULL) match = 0;

        if (match) {
            fprintf(out_file, "%s", line);
        }
    }

    // Close the file properly depending on how it was opened
    if (is_gz) {
        pclose(log_file);
    } else {
        fclose(log_file);
    }
}

int main(int argc, char *argv[]) {
    int opt;
    int filter_accepted = 0;
    int filter_failed = 0;
    char filter_method[256] = {0};
    int output_to_file = 0;

    // Parse command line arguments
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

    // Handle creation of output directory and file if '-o' is provided
    if (output_to_file) {
        const char *homedir = getenv("HOME");
        if (!homedir) {
            struct passwd *pw = getpwuid(getuid());
            if (pw) homedir = pw->pw_dir;
        }
        
        if (homedir) {
            char log_dir[512];
            snprintf(log_dir, sizeof(log_dir), "%s/ssh-logs", homedir);
            
            // Try to create the directory with 0755 permissions
            if (mkdir(log_dir, 0755) == -1 && errno != EEXIST) {
                perror("Error: Could not create directory ~/ssh-logs");
                return EXIT_FAILURE;
            }

            // Generate timestamp for the filename
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

    // Find all files matching /var/log/auth.log*
    glob_t glob_result;
    int glob_ret = glob("/var/log/auth.log*", 0, NULL, &glob_result);
    
    if (glob_ret == 0) {
        // Iterate through each matched file (including .1, .2.gz etc.)
        for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
            // Optional: Print the current file being processed (skip if just printing to stdout to avoid clutter)
            if (out_file != stdout) {
                printf("Processing: %s\n", glob_result.gl_pathv[i]);
            }
            process_file(glob_result.gl_pathv[i], out_file, filter_accepted, filter_failed, filter_method);
        }
    } else {
        fprintf(stderr, "Error: Found no files matching /var/log/auth.log*\n");
    }

    // Free the memory allocated by glob
    globfree(&glob_result);

    // Clean up
    if (out_file != stdout) {
        fclose(out_file);
    }

    return EXIT_SUCCESS;
}

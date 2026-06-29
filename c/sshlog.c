#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>

#define MAX_LINE 2048

void print_usage(const char *prog_name) {
    printf("Brug: %s [-a] [-f] [-m metode] [-o]\n", prog_name);
    printf("  -a         Vis kun accepterede logins (Accepted)\n");
    printf("  -f         Vis kun mislykkede logins (Failed)\n");
    printf("  -m metode  Filtrer efter godkendelsesmetode (f.eks. password, publickey)\n");
    printf("  -o         Gem output i ~/ssh-logs i en tidsstemplet fil\n");
    printf("  -h         Vis denne hjælpemenu\n");
}

int main(int argc, char *argv[]) {
    int opt;
    int filter_accepted = 0;
    int filter_failed = 0;
    char filter_method[256] = {0};
    int output_to_file = 0;

    // Håndter kommandolinjeargumenter med getopt
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

    // Åpne SSH-loggfilen (krever sudo for å lese /var/log/auth.log)
    FILE *log_file = fopen("/var/log/auth.log", "r");
    if (!log_file) {
        perror("Fejl: Kunne ikke åbne /var/log/auth.log (Kør evt. programmet med sudo)");
        return EXIT_FAILURE;
    }

    FILE *out_file = stdout;

    // Oppsett av output til ~/ssh-logs, hvis '-o' er valgt
    if (output_to_file) {
        const char *homedir = getenv("HOME");
        if (!homedir) {
            // Finn hjemmemappen via bruker-id
            struct passwd *pw = getpwuid(getuid());
            if (pw) homedir = pw->pw_dir;
        }
        
        if (homedir) {
            char log_dir[512];
            snprintf(log_dir, sizeof(log_dir), "%s/ssh-logs", homedir);
            
            // Oppret mappe (svarer til 'mkdir -p ~/ssh-logs')
            mkdir(log_dir, 0755); 

            // Generer tidsstempel til filnavnet
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y%m%d_%H%M%S", tm_info);

            char out_path[1024];
            snprintf(out_path, sizeof(out_path), "%s/sshlog_%s.txt", log_dir, time_buf);
            
            out_file = fopen(out_path, "w");
            if (!out_file) {
                perror("Fejl: Kunne ikke oprette output-fil i ~/ssh-logs");
                fclose(log_file);
                return EXIT_FAILURE;
            }
            printf("Output dirigeres til: %s\n", out_path);
        } else {
            fprintf(stderr, "Fejl: Kunne ikke finde brugerens hjemmemappe.\n");
            output_to_file = 0; // Fald tilbage til stdout
        }
    }

    // Gjennomgå loggfilen linje for linje
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), log_file)) {
        // Ignorer linjer som ikke relaterer til sshd
        if (strstr(line, "sshd") == NULL) {
            continue;
        }

        int match = 1; // Anta en match, inntil en regel feiler

        if (filter_accepted && strstr(line, "Accepted") == NULL) match = 0;
        if (filter_failed && strstr(line, "Failed") == NULL) match = 0;
        if (filter_method[0] != '\0' && strstr(line, filter_method) == NULL) match = 0;

        // Skriv ut linjen, hvis den består filter-kravene
        if (match) {
            fprintf(out_file, "%s", line);
        }
    }

    // Rydd opp
    fclose(log_file);
    if (out_file != stdout) {
        fclose(out_file);
    }

    return EXIT_SUCCESS;
}

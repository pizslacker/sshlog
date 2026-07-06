#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>
#include <errno.h> // Nødvendig for å sjekke spesifikke feilkoder (som EEXIST)

#define MAX_LINE 2048

void print_usage(const char *prog_name) {
    printf("Bruk: %s [-a] [-f] [-m metode] [-o]\n", prog_name);
    printf("  -a         Vis kun aksepterte logins (Accepted)\n");
    printf("  -f         Vis kun mislykkede logins (Failed)\n");
    printf("  -m metode  Filtrer etter autentiseringsmetode (f.eks. password, publickey)\n");
    printf("  -o         Lagre utdata i ~/ssh-logs i en tidsstemplet fil\n");
    printf("  -h         Vis denne hjelpemenyen\n");
}

int main(int argc, char *argv[]) {
    int opt;
    int filter_accepted = 0;
    int filter_failed = 0;
    char filter_method[256] = {0};
    int output_to_file = 0;

    // Håndter kommandolinjeargumenter
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

    // Åpne SSH-loggfilen (krever typisk sudo på Linux)
    FILE *log_file = fopen("/var/log/auth.log", "r");
    if (!log_file) {
        perror("Feil: Kunne ikke åpne /var/log/auth.log (Kjør evt. programmet med sudo)");
        return EXIT_FAILURE;
    }

    FILE *out_file = stdout;

    // Håndter opprettelse av mappe og utdatafil hvis '-o' ble valgt
    if (output_to_file) {
        const char *homedir = getenv("HOME");
        if (!homedir) {
            struct passwd *pw = getpwuid(getuid());
            if (pw) homedir = pw->pw_dir;
        }
        
        if (homedir) {
            char log_dir[512];
            snprintf(log_dir, sizeof(log_dir), "%s/ssh-logs", homedir);
            
            // Forsøk å opprette mappen med rettigheter 0755 (rwxr-xr-x)
            if (mkdir(log_dir, 0755) == -1) {
                // Sjekk om feilen er at mappen allerede eksisterer
                if (errno != EEXIST) {
                    perror("Feil: Kunne ikke opprette mappen ~/ssh-logs");
                    fclose(log_file);
                    return EXIT_FAILURE;
                }
                // Hvis feilen var EEXIST, fortsetter vi bare som normalt.
            }

            // Generer tidsstempel for filnavnet
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y%m%d_%H%M%S", tm_info);

            char out_path[1024];
            snprintf(out_path, sizeof(out_path), "%s/sshlog_%s.txt", log_dir, time_buf);
            
            out_file = fopen(out_path, "w");
            if (!out_file) {
                perror("Feil: Kunne ikke opprette filen i ~/ssh-logs");
                fclose(log_file);
                return EXIT_FAILURE;
            }
            printf("Utdata lagres i: %s\n", out_path);
        } else {
            fprintf(stderr, "Advarsel: Kunne ikke finne brukerens hjemmemappe. Skriver til konsollen (stdout) i stedet.\n");
            output_to_file = 0; 
        }
    }

    // Behandle loggfilen linje for linje
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), log_file)) {
        if (strstr(line, "sshd") == NULL) {
            continue;
        }

        int match = 1; 

        if (filter_accepted && strstr(line, "Accepted") == NULL) match = 0;
        if (filter_failed && strstr(line, "Failed") == NULL) match = 0;
        if (filter_method[0] != '\0' && strstr(line, filter_method) == NULL) match = 0;

        // Skriv ut linjen hvis den passerer filteret
        if (match) {
            fprintf(out_file, "%s", line);
        }
    }

    // Lukk filene for å frigjøre ressurser
    fclose(log_file);
    if (out_file != stdout) {
        fclose(out_file);
    }

    return EXIT_SUCCESS;
}
De viktigste endringene:
Inkludert <errno.h>.

Sjekker eksplisitt returverdien til mkdir() (if (mkdir(...) == -1)).

Bruker if (errno != EEXIST) for å ignorere feilen dersom mappen allerede ligger der fra en tidligere kjøring av skriptet, men likevel fange opp reelle feil (som mangel på skrivetilgang til hjemmemappen).

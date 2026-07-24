#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <unistd.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <termios.h>

namespace fs = std::filesystem;

struct Config {
    bool accepted = false;
    bool failed = false;
    std::string method = "";
    bool output_to_file = false;
};

// Pager class encapsulates terminal pagination (like 'more' or 'less')
class Pager {
    int current_line = 0;
    FILE* tty = nullptr;

public:
    explicit Pager(bool enable) {
        if (enable) {
            tty = fopen("/dev/tty", "r+");
        }
    }

    ~Pager() {
        if (tty) fclose(tty);
    }

    // Returns true to continue, false if the user pressed 'q'
    bool paginate() {
        if (!tty) return true; // Pagination disabled or not a TTY

        int term_height = 24;
        struct winsize w;
        
        // Dynamically fetch terminal height
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_row > 0) {
            term_height = w.ws_row;
        }

        current_line++;
        
        if (current_line >= term_height - 1) {
            std::cout << "\033[7m-- More -- (Press Space for next page, 'q' to quit)\033[0m" << std::flush;

            struct termios old_t, new_t;
            tcgetattr(fileno(tty), &old_t);
            new_t = old_t;
            
            // Disable canonical mode (waiting for Enter) and echo
            new_t.c_lflag &= ~(ICANON | ECHO); 
            tcsetattr(fileno(tty), TCSANOW, &new_t);

            int c;
            bool continue_printing = true;
            while (true) {
                c = fgetc(tty);
                if (c == 'q' || c == 'Q') {
                    continue_printing = false;
                    break;
                }
                if (c == ' ' || c == '\n' || c == '\r') {
                    break;
                }
            }

            // Restore terminal settings
            tcsetattr(fileno(tty), TCSANOW, &old_t);
            
            // Clear the prompt line
            std::cout << "\r\033[K" << std::flush;
            current_line = 0;
            
            return continue_printing;
        }
        return true;
    }
};

void print_usage(const std::string& prog_name) {
    std::cerr << "Usage: " << prog_name << " [-a] [-f] [-m method] [-o]\n"
              << "  -a         Show only accepted logins\n"
              << "  -f         Show only failed logins\n"
              << "  -m method  Filter by authentication method (e.g., password, publickey)\n"
              << "  -o         Save output to ~/ssh-logs in a timestamped file\n"
              << "  -h         Show this help menu\n";
}

Config parse_args(int argc, char* argv[]) {
    Config config;
    std::string prog_name = argv[0];

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg == "-a") {
            config.accepted = true;
        } else if (arg == "-f") {
            config.failed = true;
        } else if (arg == "-m") {
            if (i + 1 < argc) {
                config.method = argv[++i];
            } else {
                std::cerr << "Error: -m requires a method argument.\n";
                std::exit(EXIT_FAILURE);
            }
        } else if (arg == "-o") {
            config.output_to_file = true;
        } else if (arg == "-h") {
            print_usage(prog_name);
            std::exit(EXIT_SUCCESS);
        } else {
            print_usage(prog_name);
            std::exit(EXIT_FAILURE);
        }
    }
    return config;
}

// Returns true to continue processing files, false to abort entirely
bool process_file(const fs::path& filepath, std::ostream& out, const Config& config, Pager& pager) {
    std::string path_str = filepath.string();
    bool is_gz = filepath.extension() == ".gz";

    auto close_file = [](FILE* f) { if (f) fclose(f); };
    auto close_pipe = [](FILE* f) { if (f) pclose(f); };

    std::unique_ptr<FILE, decltype(close_file)> file(nullptr, close_file);
    std::unique_ptr<FILE, decltype(close_pipe)> pipe(nullptr, close_pipe);
    
    FILE* in_fp = nullptr;

    if (is_gz) {
        std::string cmd = "zcat \"" + path_str + "\" 2>/dev/null";
        pipe.reset(popen(cmd.c_str(), "r"));
        in_fp = pipe.get();
    } else {
        file.reset(fopen(path_str.c_str(), "r"));
        in_fp = file.get();
    }

    if (!in_fp) {
        std::cerr << "Warning: Could not open " << path_str << "\n";
        return true; // Return true to continue with the next file
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), in_fp)) {
        std::string_view line(buffer);

        if (line.find("sshd") == std::string_view::npos) continue;

        bool match = true;
        if (config.accepted && line.find("Accepted") == std::string_view::npos) match = false;
        if (config.failed && line.find("Failed") == std::string_view::npos) match = false;
        if (!config.method.empty() && line.find(config.method) == std::string_view::npos) match = false;

        if (match) {
            out << line;
            if (!pager.paginate()) {
                return false; // User hit 'q', abort everything
            }
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    Config config = parse_args(argc, argv);

    std::ostream* out_stream = &std::cout;
    std::ofstream file_stream;

    if (config.output_to_file) {
        const char* homedir = std::getenv("HOME");
        if (!homedir) {
            struct passwd* pw = getpwuid(getuid());
            if (pw) homedir = pw->pw_dir;
        }

        if (homedir) {
            fs::path log_dir = fs::path(homedir) / "ssh-logs";
            
            try {
                fs::create_directories(log_dir);
                
                auto now = std::chrono::system_clock::now();
                auto in_time_t = std::chrono::system_clock::to_time_t(now);
                
                std::stringstream time_ss;
                time_ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
                
                fs::path out_path = log_dir / ("sshlog_" + time_ss.str() + ".txt");
                
                file_stream.open(out_path);
                if (!file_stream.is_open()) {
                    throw std::runtime_error("Failed to open file for writing.");
                }
                
                out_stream = &file_stream;
                std::cout << "Output being saved to: " << out_path.string() << "\n";
                
            } catch (const std::exception& e) {
                std::cerr << "Error initializing output file: " << e.what() << "\n";
                return EXIT_FAILURE;
            }
        } else {
            std::cerr << "Warning: Could not find user's home directory. Defaulting to standard output.\n";
            config.output_to_file = false;
        }
    }

    // Initialize the Pager. Only enable it if we are writing to an actual terminal window.
    bool is_tty = !config.output_to_file && isatty(STDOUT_FILENO);
    Pager pager(is_tty);

    std::vector<std::string> log_files;
    try {
        for (const auto& entry : fs::directory_iterator("/var/log")) {
            std::string filename = entry.path().filename().string();
            if (filename.find("auth.log") == 0) { 
                log_files.push_back(entry.path().string());
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing /var/log: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    if (log_files.empty()) {
        std::cerr << "Error: Found no files matching /var/log/auth.log*\n";
        return EXIT_FAILURE;
    }

    std::sort(log_files.begin(), log_files.end());

    for (const auto& filepath : log_files) {
        if (config.output_to_file) {
            std::cout << "Processing: " << filepath << "\n";
        }
        
        // If process_file returns false, the user pressed 'q'
        if (!process_file(filepath, *out_stream, config, pager)) {
            break;
        }
    }

    return EXIT_SUCCESS;
}

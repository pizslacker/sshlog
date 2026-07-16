const std = @import("std");

const Config = struct {
    accepted: bool = false,
    failed: bool = false,
    method: []const u8 = "",
    output_to_file: bool = false,
};

fn printUsage(prog_name: []const u8) void {
    const stderr = std.io.getStdErr().writer();
    stderr.print("Usage: {s} [-a] [-f] [-m method] [-o]\n", .{prog_name}) catch return;
    stderr.print("  -a         Show only accepted logins\n", .{}) catch return;
    stderr.print("  -f         Show only failed logins\n", .{}) catch return;
    stderr.print("  -m method  Filter by authentication method (e.g., password, publickey)\n", .{}) catch return;
    stderr.print("  -o         Save output to ~/ssh-logs in a timestamped file\n", .{}) catch return;
    stderr.print("  -h         Show this help menu\n", .{}) catch return;
}

// Vi bruker `anytype` for I/O-parametere. Dette er Zigs måte å gjøre funksjonen 
// generisk på ("duck typing" ved kompileringstidspunktet), slik at den takler 
// både standard utskrift og fil-utskrift sømløst.
fn readAndFilter(in_reader: anytype, out_writer: anytype, config: Config) !void {
    var buf: [4096]u8 = undefined;
    
    while (try in_reader.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        if (std.mem.indexOf(u8, line, "sshd") == null) continue;

        var is_match = true;
        if (config.accepted and std.mem.indexOf(u8, line, "Accepted") == null) is_match = false;
        if (config.failed and std.mem.indexOf(u8, line, "Failed") == null) is_match = false;
        if (config.method.len > 0 and std.mem.indexOf(u8, line, config.method) == null) is_match = false;

        if (is_match) {
            try out_writer.print("{s}\n", .{line});
        }
    }
}

fn processFile(allocator: std.mem.Allocator, filepath: []const u8, out_writer: anytype, config: Config) !void {
    if (std.mem.endsWith(u8, filepath, ".gz")) {
        // I stedet for C sin usikre `popen`, bruker vi Zigs Child Process API.
        const argv = &[_][]const u8{ "zcat", filepath };
        var child = std.process.Child.init(argv, allocator);
        child.stdout_behavior = .Pipe;
        child.stderr_behavior = .Ignore;
        
        try child.spawn();

        if (child.stdout) |stdout| {
            var buf_reader = std.io.bufferedReader(stdout.reader());
            try readAndFilter(buf_reader.reader(), out_writer, config);
        }

        _ = try child.wait();
    } else {
        // Vanlig fil-lesing
        var file = std.fs.openFileAbsolute(filepath, .{}) catch |err| {
            std.debug.print("Warning: Could not open {s}: {!}\n", .{ filepath, err });
            return;
        };
        defer file.close();

        var buf_reader = std.io.bufferedReader(file.reader());
        try readAndFilter(buf_reader.reader(), out_writer, config);
    }
}

pub fn main() !void {
    // ArenaAllocator er fantastisk for kortlivede CLI-verktøy. 
    // Alt vi allokerer her vil bli ryddet opp automatisk ved `arena.deinit()`.
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var args = try std.process.argsWithAllocator(allocator);
    const prog_name = args.next() orelse "sshlog";
    var config = Config{};

    // Parse argumenter (Zig har ikke getopt, så vi gjør det manuelt)
    while (args.next()) |arg| {
        if (std.mem.eql(u8, arg, "-a")) {
            config.accepted = true;
        } else if (std.mem.eql(u8, arg, "-f")) {
            config.failed = true;
        } else if (std.mem.eql(u8, arg, "-m")) {
            config.method = args.next() orelse {
                std.debug.print("Error: -m requires a method argument.\n", .{});
                std.process.exit(1);
            };
        } else if (std.mem.eql(u8, arg, "-o")) {
            config.output_to_file = true;
        } else if (std.mem.eql(u8, arg, "-h")) {
            printUsage(prog_name);
            return;
        } else {
            printUsage(prog_name);
            std.process.exit(1);
        }
    }

    var out_file: std.fs.File = std.io.getStdOut();
    defer {
        if (config.output_to_file) out_file.close();
    }

    if (config.output_to_file) {
        if (std.process.getEnvVarOwned(allocator, "HOME")) |home| {
            const log_dir = try std.fmt.allocPrint(allocator, "{s}/ssh-logs", .{home});
            
            // Lag mappen. Feiler "stille" (catch) hvis den allerede finnes
            std.fs.cwd().makePath(log_dir) catch |err| {
                std.debug.print("Error: Could not create directory {s}: {!}\n", .{ log_dir, err });
                std.process.exit(1);
            };

            // Zig's standardbibliotek har (bevisst) ikke en stor/kompleks datoformaterer innbygget.
            // Derfor bruker vi UNIX-timestamp direkte i filnavnet for å unngå å koble inn C-biblioteket.
            const timestamp = std.time.timestamp();
            const out_path = try std.fmt.allocPrint(allocator, "{s}/sshlog_{d}.txt", .{ log_dir, timestamp });
            
            out_file = std.fs.cwd().createFile(out_path, .{}) catch |err| {
                std.debug.print("Error: Could not create output file {s}: {!}\n", .{ out_path, err });
                std.process.exit(1);
            };
            std.debug.print("Output being saved to: {s}\n", .{out_path});
            
        } else |err| {
            std.debug.print("Warning: Could not find HOME directory ({!}). Defaulting to stdout.\n", .{err});
            config.output_to_file = false;
        }
    }

    var out_writer = out_file.writer();

    // I stedet for C sin POSIX `glob()`, itererer vi direkte i mappen.
    // Dette er mye tryggere og kryssplattform.
    var log_dir_iterable = std.fs.openIterableDirAbsolute("/var/log", .{}) catch |err| {
        std.debug.print("Error: Could not open /var/log: {!}\n", .{err});
        std.process.exit(1);
    };
    defer log_dir_iterable.close();

    var iterator = log_dir_iterable.iterate();
    var files_found = false;

    // Gå gjennom hver fil i /var/log/
    while (try iterator.next()) |entry| {
        if (std.mem.startsWith(u8, entry.name, "auth.log")) {
            files_found = true;
            const filepath = try std.fmt.allocPrint(allocator, "/var/log/{s}", .{entry.name});
            
            if (config.output_to_file) {
                std.debug.print("Processing: {s}\n", .{filepath});
            }
            
            try processFile(allocator, filepath, out_writer, config);
        }
    }

    if (!files_found) {
        std.debug.print("Error: Found no files matching /var/log/auth.log*\n", .{});
    }
}

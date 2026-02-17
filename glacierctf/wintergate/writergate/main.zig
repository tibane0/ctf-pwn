const std = @import("std");
const Io = std.Io;

pub fn main() !void {
    var write_buffer: [0x600]u8 = undefined;
    var read_buffer: [0x100]u8 = undefined;

    var stdin = std.fs.File.stdin().reader(&read_buffer);
    var stdout = std.fs.File.stdout().writer(&write_buffer);

    try stdout.interface.writeAll("- Hash command\n");
    try stdout.interface.writeAll("  summary: hash a file (but not the flag)\n");
    try stdout.interface.writeAll("  usage: hash <path>\n\n");

    try stdout.interface.writeAll("- Splat command\n");
    try stdout.interface.writeAll("  summary: fill terminal with favourite ascii char\n");
    try stdout.interface.writeAll("  usage: splat <n> <byte>\n\n");

    try stdout.interface.writeAll("- Crash command\n");
    try stdout.interface.writeAll("  summary: crash in a unique way\n");
    try stdout.interface.writeAll("  usage: crash <where> <byte>\n\n");
    try stdout.interface.flush();

    while (true) {
        if (stdin.interface.bufferedLen() == 0) { // no prompt when there is already buffered input
            try stdout.interface.writeAll("command> ");
            try stdout.interface.flush();
        }

        const command = try stdin.interface.takeDelimiterInclusive(' ');
        if (std.mem.eql(u8, command, "hash ")) {
            const path = try stdin.interface.takeDelimiterExclusive('\n');
            stdin.interface.toss(1);

            if (std.mem.containsAtLeast(u8, path, 1, "flag")) continue;
            const ff = std.fs.openFileAbsolute(path, .{ .mode = .read_only }) catch continue;

            var discarding_writer = Io.Writer.Discarding.init(&write_buffer);
            var file_reader = ff.reader(&.{});
            var file_read_buffer: [0x100]u8 = undefined;
            var hashed_file_reader = Io.Reader.Hashed(std.crypto.hash.sha2.Sha256).init(&file_reader.interface, std.crypto.hash.sha2.Sha256.init(.{}), &file_read_buffer);

            _ = try hashed_file_reader.reader.streamRemaining(&discarding_writer.writer);

            var hash: [std.crypto.hash.sha2.Sha256.digest_length]u8 = undefined;
            hashed_file_reader.hasher.final(&hash);

            try stdout.interface.print("path: {s}, hash: {x}, size: 0x{x}\n", .{ path, hash, discarding_writer.count });

        } else if (std.mem.eql(u8, command, "splat ")) {
            const n_str = try stdin.interface.takeDelimiterExclusive(' ');
            const n = try std.fmt.parseInt(usize, n_str, 16);

            stdin.interface.toss(1); // " "

            const b = try stdin.interface.takeByte();
            if (n > write_buffer.len) {
                try stdout.interface.print("Splat n={} too big for {*}\n", .{n, &write_buffer});
            } else _ = try stdout.interface.splatByte(b, n);

            _ = try stdin.interface.takeByte(); // "\n"

        } else if (std.mem.eql(u8, command, "crash ")) {
            const location_str = try stdin.interface.takeDelimiterExclusive(' ');
            const location = try std.fmt.parseInt(usize, location_str, 16);
            const v: *u8 = @ptrFromInt(location);

            stdin.interface.toss(1); // " "

            const b = try stdin.interface.takeByte();
            v.* = b;

            _ = try stdin.interface.takeByte(); // "\n"
            return;
        }
    }
}

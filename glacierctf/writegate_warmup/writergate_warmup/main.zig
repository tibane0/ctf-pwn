const std = @import("std");

// - NOTE: Implementations in root are irrelevant for this challenge
const root = @import("challenge");

const CryptState = root.CryptState;
const Encrypt = root.crypt_stream.Encrypt;
const frameSlice = root.frameSlice;
const c_sodium = root.c_sodium;
// -

pub fn main() !void {
    if (c_sodium.sodium_init() < 0) return error.Upsie;

    var key: [CryptState.KEYBYTES]u8 = undefined;
    c_sodium.crypto_secretstream_xchacha20poly1305_keygen(@ptrCast(&key));

    var crypt_state = try CryptState.init(&key);

    var read_buffer: [0x100]u8 = undefined;
    var encryptor_buffer: [0x100]u8 = undefined;
    var write_buffer: [0x100]u8 = undefined;

    var stdin = std.fs.File.stdin().reader(&read_buffer);
    var stdout = std.fs.File.stdout().writer(&write_buffer);

    try frameSlice(&stdout.interface, "  menu   ", true, false);
    try frameSlice(&stdout.interface, "         ", false, false);
    try frameSlice(&stdout.interface, "1 - frame", false, false);
    try frameSlice(&stdout.interface, "2 - truth", false, false);
    try frameSlice(&stdout.interface, "3 - claim", false, true);
    try stdout.interface.writeAll("\noption> ");
    try stdout.interface.flush();

    while (true) {
        switch (try stdin.interface.takeInt(u8, .big)) {
            '1' => {
                if (try stdin.interface.peekByte() == '\n') {stdin.interface.toss(1);}

                try stdout.interface.writeAll("quote> ");
                try stdout.interface.flush();

                var tmp_buffer: [(0x100-4)/3]u8 = undefined;
                var writer = std.Io.Writer.fixed(&tmp_buffer);
                const quote_len = try stdin.interface.streamDelimiter(&writer, '\n');

                try frameSlice(&stdout.interface, tmp_buffer[0..quote_len], true, true);
            },
            '2' => {
                try frameSlice(&stdout.interface, "To make a gain, take the claim", true, true);
            },
            '3' => {
                const ff = try std.fs.openFileAbsolute("/flag.txt", .{ .mode = .read_only });
                var file_reader = ff.reader(&read_buffer);

                var encryptor = try Encrypt.init(&crypt_state, &file_reader.interface, &encryptor_buffer);
                _ = try encryptor.interface.streamRemaining(&stdout.interface);

                try stdout.interface.writeAll("\n");
            },
            '\n' => {
                try stdout.interface.writeAll("option> ");
            },
            ' ' => continue,
            0 => break,
            else => |c| try stdout.interface.print("No such option \\x{x}\n", .{c}),
        }
        try stdout.interface.flush();

    }
}

//! By convention, main.zig is where your main function lives in the case that
//! you are building an executable. If you are making a library, the convention
//! is to delete this file and start with root.zig instead.

const std = @import("std");
const c = @cImport({
    @cInclude("nrf24l01.c");
});

test "c.byte_set_bits" {
    var byte: u8 = undefined;

    // Case 1: Set high 4 bits to 0x01
    byte = 0xff;
    c.byte_set_bits(&byte, 0xF0, 0x01);
    try std.testing.expectEqual(@as(u8, 0x1f), byte);

    // Case 2: Set high 4 bits to 0x0A
    byte = 0x00;
    c.byte_set_bits(&byte, 0xF0, 0x0A);
    try std.testing.expectEqual(@as(u8, 0xA0), byte);

    // Case 3: Set low 4 bits to 0x0D
    byte = 0xFF;
    c.byte_set_bits(&byte, 0x0F, 0x0D);
    try std.testing.expectEqual(@as(u8, 0xFD), byte);

    // Case 4: Set low 4 bits to 0x00
    byte = 0xFF;
    c.byte_set_bits(&byte, 0x0F, 0x00);
    try std.testing.expectEqual(@as(u8, 0xF0), byte);

    // Case 5: Set middle 4 bits (0x3C) to 0x0F
    byte = 0x00;
    c.byte_set_bits(&byte, 0x3C, 0x0F); // should be 0b00111100
    try std.testing.expectEqual(@as(u8, 0x3C), byte);

    // Case 6: Clear a single bit (mask = 0x08)
    byte = 0xFF;
    c.byte_set_bits(&byte, 0x08, 0x00);
    try std.testing.expectEqual(@as(u8, 0xF7), byte);

    // Case 7: Set same bit to 1
    byte = 0x00;
    c.byte_set_bits(&byte, 0x08, 0x01);
    try std.testing.expectEqual(@as(u8, 0x08), byte);

    // Case 8: Zero mask
    byte = 0xFF;
    c.byte_set_bits(&byte, 0x00, 0x00);
    try std.testing.expectEqual(@as(u8, 0xFF), byte);

    std.debug.print("c.byte_set_bits [\x1b[32mok\x1b[0m]\n", .{});
}

test "c.byte_get_bits" {
    var byte: u8 = undefined;
    var result: u8 = undefined;

    // Case 1: Get high 4 bits (0xF0) => should return bits [7:4]
    byte = 0x3A;
    result = c.byte_get_bits(byte, 0xF0);
    try std.testing.expectEqual(@as(u8, 0x03), result);

    // Case 2: Get low 4 bits (0x0F) => should return bits [3:0]
    byte = 0x3A;
    result = c.byte_get_bits(byte, 0x0F);
    try std.testing.expectEqual(@as(u8, 0x0A), result);

    // Case 3: Get middle 4 bits (0x3C) => bits [5:2]
    byte = 0x5A; // 0101 1010
    result = c.byte_get_bits(byte, 0x3C); // mask: 0011 1100 => bits 5-2: 0110
    try std.testing.expectEqual(@as(u8, 0x06), result);

    // Case 4: Get single bit (0x08) => bit 3
    byte = 0x0A; // 0000 1010
    result = c.byte_get_bits(byte, 0x08); // bit 3 is 1
    try std.testing.expectEqual(@as(u8, 0x01), result);

    // Case 5: Get zero mask (0x00) => should return 0
    byte = 0xFF;
    result = c.byte_get_bits(byte, 0x00);
    try std.testing.expectEqual(@as(u8, 0x00), result);

    // Case 6: Mask is 0x01 (LSB)
    byte = 0x01;
    result = c.byte_get_bits(byte, 0x01);
    try std.testing.expectEqual(@as(u8, 0x01), result);

    // Case 7: Mask is 0x80 (MSB)
    byte = 0x80;
    result = c.byte_get_bits(byte, 0x80);
    try std.testing.expectEqual(@as(u8, 0x01), result);

    // Case 8: Mask is 0x03 (two bits)
    byte = 0x03;
    result = c.byte_get_bits(byte, 0x03);
    try std.testing.expectEqual(@as(u8, 0x03), result);

    // Case 9: Mask is 0x03 on byte 0x0C (bits [3:0] = 1100)
    byte = 0x0C; // 0000 1100
    result = c.byte_get_bits(byte, 0x03); // mask: 0000 0011 → bits [1:0] are 00
    try std.testing.expectEqual(@as(u8, 0x00), result);

    std.debug.print("c.byte_get_bits [\x1b[32mok\x1b[0m]\n", .{});
}

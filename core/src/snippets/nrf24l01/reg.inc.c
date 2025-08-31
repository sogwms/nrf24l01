
/**
 * @brief Sets bits in a byte according to a contiguous mask.
 *
 * Clears the bits in `*byte` covered by `mask`, then writes `value` into those bits.
 * Mask must be a contiguous set of 1's (e.g., 0b0000_0001, 0b0011_1100).
 *
 * Example:
 *   uint8_t reg = 0xFF;
 *   byte_set_bits(&reg, 0xF0, 0x03); // => reg == 0x3F
 *   byte_set_bits(&reg, 0x0F, 0x0A); // => reg == 0x3A
 *
 * @param byte   Pointer to the byte to modify.
 * @param mask   Bitmask with contiguous 1's (e.g. 0b01110000).
 * @param value  Value to write into the masked bit field (unshifted).
 */
static void byte_set_bits(uint8_t *byte, uint8_t mask, uint8_t value)
{
    uint8_t lsb;
    for (lsb = 0; lsb < 8; lsb++) {
        if (mask & (1 << lsb)) break;
    }

    // Clear the bits covered by the mask
    *byte &= ~mask;

    // Shift the value to correct position and apply it
    *byte |= ((uint8_t)(value << lsb)) & mask;
}

/**
 * @brief Extracts bits from a byte according to a contiguous bitmask.
 *
 * Reads the bits from `byte` that are covered by `mask`, shifts them down,
 * and returns the extracted value.
 *
 * Example:
 *   uint8_t reg = 0x3A;
 *   uint8_t val = byte_get_bits(reg, 0xF0); // => val == 0x03
 *   val = byte_get_bits(reg, 0x0F);         // => val == 0x0A
 *
 * @param[in] byte   The byte to extract bits from.
 * @param[in] mask   Bitmask with contiguous 1's (e.g. 0b01110000).
 * @return           The extracted value, right-aligned.
 */
static uint8_t byte_get_bits(uint8_t byte, uint8_t mask)
{ 
    uint8_t lsb;
    for (lsb = 0; lsb < 8; lsb++) {
        if (mask & (1 << lsb)) break;
    }

    return (byte & mask) >> lsb;
}

/**
 * @brief Modifies specific bits in a register using a bitmask and a new value.
 *
 * Reads the current register value, applies the bit changes using byte_set_bits(),
 * and writes back the updated value.
 *
 * @param[in,out] dep     Device-dependent context structure.
 * @param[in]     reg     Register address to modify.
 * @param[in]     mask    Bitmask specifying which bits to change.
 * @param[in]     value   New value to write into the masked bit field (unshifted).
 * @return                0 on success, negative error code on failure.
 */
static int reg_modify_bits(nrf24_dep_t *dep, uint8_t reg, uint8_t mask, uint8_t value)
{
    uint8_t byte;

    if (read_reg(dep, reg, &byte)) return -1;
    byte_set_bits(&byte, mask, value);
    return write_reg(dep, reg, byte);
}

/**
 * @brief Resets (clears) specific bits in a register using a bitmask.
 *
 * Reads the current register value, clears the specified bits, and writes back.
 *
 * @param[in,out] dep     Device-dependent context structure.
 * @param[in]     reg     Register address to modify.
 * @param[in]     mask    Bitmask specifying which bits to reset (clear).
 * @return                0 on success, negative error code on failure.
 */
static int reg_reset_bits(nrf24_dep_t *dep, uint8_t reg, uint8_t mask)
{
    uint8_t byte;
    if (read_reg(dep, reg, &byte)) return -1;
    byte &= ~mask;
    return write_reg(dep, reg, byte);
}

/**
 * @brief Sets specific bits in a register using a bitmask.
 *
 * Reads the current register value, sets the specified bits, and writes back.
 *
 * @param[in,out] dep     Device-dependent context structure.
 * @param[in]     reg     Register address to modify.
 * @param[in]     mask    Bitmask specifying which bits to set.
 * @return                0 on success, negative error code on failure.
 */
static int reg_set_bits(nrf24_dep_t *dep, uint8_t reg, uint8_t mask)
{
    uint8_t byte;
    if (read_reg(dep, reg, &byte)) return -1;
    byte |= mask;
    return write_reg(dep, reg, byte);
}

/**
 * @brief Ensures the register has a specific value.
 *
 * Reads the current register value. If it does not match `val`, writes the desired value.
 *
 * @param[in,out] dep     Device-dependent context structure.
 * @param[in]     reg     Register address to synchronize.
 * @param[in]     val     Expected register value.
 * @return                0 on success
 */
static int reg_sync_val(nrf24_dep_t *dep, uint8_t reg, uint8_t val)
{
    int ret = 0;
    uint8_t cur;

    ret += read_reg(dep, reg, &cur);
    if (cur != val)
        ret += write_reg(dep, reg, val);

    return ret;
}

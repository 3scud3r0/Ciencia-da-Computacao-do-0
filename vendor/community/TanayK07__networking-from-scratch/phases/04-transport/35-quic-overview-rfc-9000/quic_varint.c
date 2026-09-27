/*
 * quic_varint.c -- QUIC variable-length integer encoding (RFC 9000 §16)
 *
 * The encoding uses the two most significant bits of the first byte
 * as a length prefix.  This lets QUIC encode small values (stream IDs,
 * frame types) in 1 byte while supporting values up to 2^62 - 1.
 */
#include "quic_varint.h"

size_t quic_varint_len(uint64_t val) {
    if (val <= 63)
        return 1;
    if (val <= 16383)
        return 2;
    if (val <= 1073741823)
        return 4;
    if (val <= QUIC_VARINT_MAX)
        return 8;
    return 0; /* too large */
}

int quic_varint_decode(const uint8_t *buf, size_t len, uint64_t *out, size_t *consumed) {
    if (len == 0)
        return -1;

    /* The two MSBs encode log2(byte_count): 00→1, 01→2, 10→4, 11→8 */
    uint8_t prefix = buf[0] >> 6;
    size_t need = (size_t)1 << prefix;

    if (len < need)
        return -1;

    /* Read the first byte with the prefix masked off */
    uint64_t val = buf[0] & 0x3F;

    /* Accumulate remaining bytes in network (big-endian) order */
    for (size_t i = 1; i < need; i++)
        val = (val << 8) | buf[i];

    *out = val;
    *consumed = need;
    return 0;
}

int quic_varint_encode(uint64_t val, uint8_t *buf, size_t len, size_t *written) {
    size_t need = quic_varint_len(val);
    if (need == 0 || len < need)
        return -1;

    /* Prefix byte to OR into the first encoded byte */
    uint8_t prefix;
    switch (need) {
    case 1:
        prefix = 0x00;
        break;
    case 2:
        prefix = 0x40;
        break;
    case 4:
        prefix = 0x80;
        break;
    case 8:
        prefix = 0xC0;
        break;
    default:
        return -1; /* unreachable */
    }

    /* Write val in big-endian, then stamp the prefix onto byte 0 */
    for (size_t i = need; i > 0; i--) {
        buf[i - 1] = (uint8_t)(val & 0xFF);
        val >>= 8;
    }
    buf[0] |= prefix;

    *written = need;
    return 0;
}

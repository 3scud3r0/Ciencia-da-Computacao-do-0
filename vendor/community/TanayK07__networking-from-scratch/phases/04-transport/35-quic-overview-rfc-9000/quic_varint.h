#ifndef NFS_QUIC_VARINT_H
#define NFS_QUIC_VARINT_H

#include <stddef.h>
#include <stdint.h>

/* ---------------------------------------------------------------
 * QUIC variable-length integer encoding (RFC 9000, Section 16).
 *
 * The two most significant bits of the first byte encode the
 * byte length of the integer:
 *
 *   00 → 1 byte  (6 usable bits,  max 63)
 *   01 → 2 bytes (14 usable bits, max 16383)
 *   10 → 4 bytes (30 usable bits, max 1073741823)
 *   11 → 8 bytes (62 usable bits, max 4611686018427387903)
 * --------------------------------------------------------------- */

#define QUIC_VARINT_MAX ((uint64_t)4611686018427387903ULL) /* 2^62 - 1 */

/* Decode a variable-length integer from buf[0..len).
 * On success: stores the decoded value in *out, the number of bytes
 * consumed in *consumed, and returns 0.
 * On error (truncated input): returns -1 and does not modify *out. */
int quic_varint_decode(const uint8_t *buf, size_t len, uint64_t *out, size_t *consumed);

/* Encode a variable-length integer into buf[0..len).
 * Picks the smallest encoding that fits val.
 * On success: stores the number of bytes written in *written, returns 0.
 * On error (val > QUIC_VARINT_MAX, or buffer too small): returns -1. */
int quic_varint_encode(uint64_t val, uint8_t *buf, size_t len, size_t *written);

/* Return the number of bytes needed to encode val.
 * Returns 0 if val > QUIC_VARINT_MAX. */
size_t quic_varint_len(uint64_t val);

#endif /* NFS_QUIC_VARINT_H */

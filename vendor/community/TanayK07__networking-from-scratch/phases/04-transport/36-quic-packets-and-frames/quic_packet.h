#ifndef NFS_QUIC_PACKET_H
#define NFS_QUIC_PACKET_H

#include <stddef.h>
#include <stdint.h>

/* ---------------------------------------------------------------
 * QUIC packet header parsing (RFC 9000, Sections 17.2 and 17.3).
 *
 * Two header forms:
 *   Long Header  (first byte bit 7 = 1) — handshake packets
 *   Short Header (first byte bit 7 = 0) — 1-RTT data packets
 * --------------------------------------------------------------- */

/* First-byte bit masks */
#define QUIC_HEADER_FORM_BIT  0x80
#define QUIC_FIXED_BIT        0x40
#define QUIC_LONG_TYPE_MASK   0x30
#define QUIC_LONG_TYPE_SHIFT  4
#define QUIC_PKT_NUM_LEN_MASK 0x03
#define QUIC_SHORT_SPIN_BIT   0x20
#define QUIC_SHORT_KEY_PHASE  0x04

/* Long Header packet types (RFC 9000, §17.2) */
#define QUIC_PKT_INITIAL   0
#define QUIC_PKT_0RTT      1
#define QUIC_PKT_HANDSHAKE 2
#define QUIC_PKT_RETRY     3

/* QUIC v1 version number */
#define QUIC_VERSION_1 0x00000001u
#define QUIC_VERSION_2 0x6b3343cfu

#define QUIC_MAX_CID_LEN 20

/* ---------------------------------------------------------------
 * Frame type constants (RFC 9000, §19)
 * --------------------------------------------------------------- */

#define QUIC_FRAME_PADDING              0x00
#define QUIC_FRAME_PING                 0x01
#define QUIC_FRAME_ACK                  0x02
#define QUIC_FRAME_ACK_ECN              0x03
#define QUIC_FRAME_RESET_STREAM         0x04
#define QUIC_FRAME_STOP_SENDING         0x05
#define QUIC_FRAME_CRYPTO               0x06
#define QUIC_FRAME_NEW_TOKEN            0x07
#define QUIC_FRAME_STREAM_BASE          0x08
#define QUIC_FRAME_STREAM_MAX           0x0f
#define QUIC_FRAME_MAX_DATA             0x10
#define QUIC_FRAME_MAX_STREAM_DATA      0x11
#define QUIC_FRAME_MAX_STREAMS_BIDI     0x12
#define QUIC_FRAME_MAX_STREAMS_UNI      0x13
#define QUIC_FRAME_DATA_BLOCKED         0x14
#define QUIC_FRAME_STREAM_DATA_BLOCKED  0x15
#define QUIC_FRAME_STREAMS_BLOCKED_BIDI 0x16
#define QUIC_FRAME_STREAMS_BLOCKED_UNI  0x17
#define QUIC_FRAME_NEW_CONNECTION_ID    0x18
#define QUIC_FRAME_RETIRE_CONNECTION_ID 0x19
#define QUIC_FRAME_PATH_CHALLENGE       0x1a
#define QUIC_FRAME_PATH_RESPONSE        0x1b
#define QUIC_FRAME_CONNECTION_CLOSE     0x1c
#define QUIC_FRAME_CONNECTION_CLOSE_APP 0x1d
#define QUIC_FRAME_HANDSHAKE_DONE       0x1e

/* STREAM frame sub-flags (bits 0-2 of type byte) */
#define QUIC_STREAM_FIN_BIT 0x01
#define QUIC_STREAM_LEN_BIT 0x02
#define QUIC_STREAM_OFF_BIT 0x04

/* Packet types for frame-allowed checks */
#define QUIC_PKTCTX_INITIAL   0x01
#define QUIC_PKTCTX_HANDSHAKE 0x02
#define QUIC_PKTCTX_0RTT      0x04
#define QUIC_PKTCTX_1RTT      0x08

/* ---------------------------------------------------------------
 * Parsed header structs
 * --------------------------------------------------------------- */

struct quic_long_hdr {
    uint8_t first_byte;
    uint32_t version;
    uint8_t dcid_len;
    uint8_t dcid[QUIC_MAX_CID_LEN];
    uint8_t scid_len;
    uint8_t scid[QUIC_MAX_CID_LEN];
};

struct quic_short_hdr {
    uint8_t first_byte;
    uint8_t dcid_len; /* caller must supply — not encoded on wire */
    uint8_t dcid[QUIC_MAX_CID_LEN];
};

/* ---------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------- */

/* Extract the Long Packet Type from a first byte (assumes long header). */
static inline int quic_long_hdr_type(uint8_t first_byte) {
    return (first_byte & QUIC_LONG_TYPE_MASK) >> QUIC_LONG_TYPE_SHIFT;
}

/* True if first byte indicates a long header. */
static inline int quic_is_long_header(uint8_t first_byte) {
    return (first_byte & QUIC_HEADER_FORM_BIT) != 0;
}

/* Parse a Long Header from buf[0..len).
 * Reads through SCID; does not parse type-specific fields (token, length).
 * On success: fills *out, stores bytes consumed in *consumed, returns 0.
 * On error: returns -1. */
int quic_long_hdr_parse(const uint8_t *buf, size_t len, struct quic_long_hdr *out,
                        size_t *consumed);

/* Build a Long Header into buf[0..len).
 * Returns bytes written, or 0 on error. */
size_t quic_long_hdr_build(const struct quic_long_hdr *hdr, uint8_t *buf, size_t len);

/* Parse a Short Header from buf[0..len).
 * Caller must supply dcid_len (known from connection state).
 * On success: fills *out, stores bytes consumed in *consumed, returns 0. */
int quic_short_hdr_parse(const uint8_t *buf, size_t len, uint8_t dcid_len,
                         struct quic_short_hdr *out, size_t *consumed);

/* Build a Short Header into buf[0..len).
 * Returns bytes written, or 0 on error. */
size_t quic_short_hdr_build(const struct quic_short_hdr *hdr, uint8_t *buf, size_t len);

/* Return a human-readable name for a QUIC frame type.
 * Returns "UNKNOWN" for unrecognized types. */
const char *quic_frame_name(uint64_t frame_type);

/* Return a human-readable name for a Long Packet type (0-3). */
const char *quic_pkt_type_name(int pkt_type);

/* Check if a frame type is allowed in a given packet context.
 * pkt_ctx is a bitmask of QUIC_PKTCTX_* values.
 * Returns 1 if allowed, 0 if forbidden per RFC 9000 Table 3. */
int quic_frame_allowed(uint64_t frame_type, int pkt_ctx);

#endif /* NFS_QUIC_PACKET_H */

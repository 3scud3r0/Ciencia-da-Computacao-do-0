#ifndef NFS_QUIC_STREAM_H
#define NFS_QUIC_STREAM_H

#include <stddef.h>
#include <stdint.h>

/* ---------------------------------------------------------------
 * QUIC streams (RFC 9000, Sections 2-3, 19.8).
 *
 * Stream ID encoding (§2.1):
 *   Bit 0: initiator    — 0 = client, 1 = server
 *   Bit 1: directionality — 0 = bidirectional, 1 = unidirectional
 *
 *   Type 0x00: client-initiated, bidirectional   (IDs: 0, 4, 8, ...)
 *   Type 0x01: server-initiated, bidirectional   (IDs: 1, 5, 9, ...)
 *   Type 0x02: client-initiated, unidirectional  (IDs: 2, 6, 10, ...)
 *   Type 0x03: server-initiated, unidirectional  (IDs: 3, 7, 11, ...)
 *
 * STREAM frame type byte (§19.8):
 *   Base type 0x08, low 3 bits encode flags:
 *     Bit 0 (0x01): FIN — last data on this stream
 *     Bit 1 (0x02): LEN — Length field is present
 *     Bit 2 (0x04): OFF — Offset field is present
 * --------------------------------------------------------------- */

/* STREAM frame type byte flags */
#define QUIC_STREAM_FIN_BIT   0x01
#define QUIC_STREAM_LEN_BIT   0x02
#define QUIC_STREAM_OFF_BIT   0x04
#define QUIC_STREAM_TYPE_BASE 0x08

/* Stream classification results */
#define QUIC_STREAM_CLIENT 0
#define QUIC_STREAM_SERVER 1
#define QUIC_STREAM_BIDI   0
#define QUIC_STREAM_UNI    1

/* Maximum data chunk size we support for reassembly */
#define QUIC_STREAM_MAX_CHUNKS 64
#define QUIC_STREAM_MAX_DATA   (64 * 1024)

/* ---------------------------------------------------------------
 * Stream classification (RFC 9000, §2.1)
 * --------------------------------------------------------------- */

struct quic_stream_info {
    int initiator; /* QUIC_STREAM_CLIENT (0) or QUIC_STREAM_SERVER (1) */
    int direction; /* QUIC_STREAM_BIDI (0) or QUIC_STREAM_UNI (1) */
    uint64_t seq;  /* Stream sequence number (stream_id >> 2) */
};

/* Classify a stream ID per RFC 9000 §2.1.
 * Fills *info with initiator, direction, and sequence number. */
void quic_stream_classify(uint64_t stream_id, struct quic_stream_info *info);

/* Return a human-readable label: "client-bidi", "server-uni", etc. */
const char *quic_stream_type_name(uint64_t stream_id);

/* ---------------------------------------------------------------
 * STREAM frame (RFC 9000, §19.8)
 * --------------------------------------------------------------- */

struct quic_stream_frame {
    uint8_t type; /* 0x08..0x0f */
    uint64_t stream_id;
    uint64_t offset;     /* 0 if OFF bit not set */
    uint64_t length;     /* == data_len; explicit if LEN bit set */
    int fin;             /* 1 if FIN bit set */
    const uint8_t *data; /* pointer into parsed buffer (parse only) */
    size_t data_len;
};

/* Build a STREAM frame into buf[0..len).
 * data/data_len: payload to include.
 * flags: combination of QUIC_STREAM_FIN_BIT, QUIC_STREAM_LEN_BIT,
 *        QUIC_STREAM_OFF_BIT.
 * On success: stores bytes written in *written, returns 0.
 * On error: returns -1. */
int quic_stream_frame_build(uint64_t stream_id, uint64_t offset, const uint8_t *data,
                            size_t data_len, int flags, uint8_t *buf, size_t len, size_t *written);

/* Parse a STREAM frame from buf[0..len).
 * The first byte must be in range 0x08..0x0f.
 * On success: fills *out, stores bytes consumed in *consumed, returns 0.
 * out->data points into buf (not a copy).
 * On error: returns -1. */
int quic_stream_frame_parse(const uint8_t *buf, size_t len, struct quic_stream_frame *out,
                            size_t *consumed);

/* ---------------------------------------------------------------
 * Stream reassembly buffer
 *
 * Accepts out-of-order data chunks at arbitrary offsets and
 * reassembles them into a contiguous byte stream.
 * --------------------------------------------------------------- */

struct quic_stream_chunk {
    uint64_t offset;
    size_t len;
};

struct quic_stream_reasm {
    uint8_t data[QUIC_STREAM_MAX_DATA];
    struct quic_stream_chunk chunks[QUIC_STREAM_MAX_CHUNKS];
    size_t num_chunks;
    uint64_t total_size; /* largest offset + length seen */
    int fin_received;
    uint64_t fin_offset; /* valid only if fin_received */
};

/* Initialize a reassembly buffer. */
void quic_stream_reasm_init(struct quic_stream_reasm *r);

/* Insert data at a given offset.
 * Returns 0 on success, -1 on error (overlap, out of space, etc.). */
int quic_stream_reasm_insert(struct quic_stream_reasm *r, uint64_t offset, const uint8_t *data,
                             size_t len, int fin);

/* Return the number of contiguous bytes available from offset 0. */
uint64_t quic_stream_reasm_readable(const struct quic_stream_reasm *r);

/* Return 1 if the entire stream has been received (all bytes from
 * 0..fin_offset are contiguous and FIN has been received). */
int quic_stream_reasm_complete(const struct quic_stream_reasm *r);

#endif /* NFS_QUIC_STREAM_H */

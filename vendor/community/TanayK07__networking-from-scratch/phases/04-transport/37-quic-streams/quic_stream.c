/*
 * quic_stream.c -- QUIC stream classification, STREAM frame codec,
 *                  and stream reassembly buffer.
 *
 * Implements RFC 9000 §2.1 (stream types), §19.8 (STREAM frame),
 * and basic stream-level reassembly for out-of-order delivery.
 *
 * Depends on the varint codec from Lesson 35.
 */
#include "quic_stream.h"
#include "../35-quic-overview-rfc-9000/quic_varint.h"
#include <string.h>

/* ---------------------------------------------------------------
 * Stream classification (RFC 9000, §2.1)
 *
 * The two least significant bits of the stream ID encode:
 *   bit 0 → initiator (0 = client, 1 = server)
 *   bit 1 → direction (0 = bidi, 1 = uni)
 * --------------------------------------------------------------- */

void quic_stream_classify(uint64_t stream_id, struct quic_stream_info *info) {
    info->initiator = (int)(stream_id & 0x01);
    info->direction = (int)((stream_id >> 1) & 0x01);
    info->seq = stream_id >> 2;
}

const char *quic_stream_type_name(uint64_t stream_id) {
    switch (stream_id & 0x03) {
    case 0x00:
        return "client-bidi";
    case 0x01:
        return "server-bidi";
    case 0x02:
        return "client-uni";
    case 0x03:
        return "server-uni";
    default:
        return "unknown"; /* unreachable */
    }
}

/* ---------------------------------------------------------------
 * STREAM frame builder (RFC 9000, §19.8)
 *
 * Wire format:
 *   Type (1 byte, 0x08-0x0f)
 *   Stream ID (varint)
 *   [Offset (varint)]    — present if OFF bit set
 *   [Length (varint)]     — present if LEN bit set
 *   Stream Data (..)
 * --------------------------------------------------------------- */

int quic_stream_frame_build(uint64_t stream_id, uint64_t offset, const uint8_t *data,
                            size_t data_len, int flags, uint8_t *buf, size_t len, size_t *written) {
    size_t pos = 0;

    /* Type byte: base 0x08 OR'd with flags */
    uint8_t type = QUIC_STREAM_TYPE_BASE | (uint8_t)(flags & 0x07);

    /* Calculate total size needed */
    size_t need = 1; /* type byte */
    need += quic_varint_len(stream_id);
    if (flags & QUIC_STREAM_OFF_BIT)
        need += quic_varint_len(offset);
    if (flags & QUIC_STREAM_LEN_BIT)
        need += quic_varint_len((uint64_t)data_len);
    need += data_len;

    if (need == 0 || len < need)
        return -1;

    /* Write type byte */
    buf[pos++] = type;

    /* Write Stream ID */
    size_t n;
    if (quic_varint_encode(stream_id, buf + pos, len - pos, &n) != 0)
        return -1;
    pos += n;

    /* Write Offset if OFF bit set */
    if (flags & QUIC_STREAM_OFF_BIT) {
        if (quic_varint_encode(offset, buf + pos, len - pos, &n) != 0)
            return -1;
        pos += n;
    }

    /* Write Length if LEN bit set */
    if (flags & QUIC_STREAM_LEN_BIT) {
        if (quic_varint_encode((uint64_t)data_len, buf + pos, len - pos, &n) != 0)
            return -1;
        pos += n;
    }

    /* Write data */
    if (data_len > 0) {
        if (pos + data_len > len)
            return -1;
        memcpy(buf + pos, data, data_len);
        pos += data_len;
    }

    *written = pos;
    return 0;
}

/* ---------------------------------------------------------------
 * STREAM frame parser (RFC 9000, §19.8)
 * --------------------------------------------------------------- */

int quic_stream_frame_parse(const uint8_t *buf, size_t len, struct quic_stream_frame *out,
                            size_t *consumed) {
    if (len < 1)
        return -1;

    uint8_t type = buf[0];

    /* Must be a STREAM frame: type in 0x08..0x0f */
    if (type < QUIC_STREAM_TYPE_BASE || type > 0x0f)
        return -1;

    out->type = type;
    out->fin = (type & QUIC_STREAM_FIN_BIT) ? 1 : 0;

    size_t pos = 1;

    /* Stream ID (always present) */
    uint64_t val;
    size_t n;
    if (quic_varint_decode(buf + pos, len - pos, &val, &n) != 0)
        return -1;
    out->stream_id = val;
    pos += n;

    /* Offset (present if OFF bit set) */
    if (type & QUIC_STREAM_OFF_BIT) {
        if (quic_varint_decode(buf + pos, len - pos, &val, &n) != 0)
            return -1;
        out->offset = val;
        pos += n;
    } else {
        out->offset = 0;
    }

    /* Length (present if LEN bit set) */
    if (type & QUIC_STREAM_LEN_BIT) {
        if (quic_varint_decode(buf + pos, len - pos, &val, &n) != 0)
            return -1;
        out->length = val;
        pos += n;
        /* Validate that length bytes are available */
        if (pos + out->length > len)
            return -1;
        out->data = buf + pos;
        out->data_len = (size_t)out->length;
        pos += out->data_len;
    } else {
        /* Without LEN, data extends to end of buffer */
        out->data = buf + pos;
        out->data_len = len - pos;
        out->length = (uint64_t)out->data_len;
        pos = len;
    }

    *consumed = pos;
    return 0;
}

/* ---------------------------------------------------------------
 * Stream reassembly buffer
 *
 * Tracks received chunks and merges contiguous regions.
 * This is a simplified model: real implementations use interval
 * trees for O(log n) insert.  We use a sorted array of chunks
 * and merge on insert, which is O(n) but clear and correct.
 * --------------------------------------------------------------- */

void quic_stream_reasm_init(struct quic_stream_reasm *r) {
    memset(r, 0, sizeof(*r));
}

/* Insert and merge a chunk into the sorted chunk list.
 * Maintains the invariant: chunks are sorted by offset and
 * non-overlapping after each insert. */
static void merge_chunks(struct quic_stream_reasm *r) {
    if (r->num_chunks < 2)
        return;

    /* Simple insertion sort by offset (chunks are nearly sorted) */
    for (size_t i = 1; i < r->num_chunks; i++) {
        struct quic_stream_chunk key = r->chunks[i];
        size_t j = i;
        while (j > 0 && r->chunks[j - 1].offset > key.offset) {
            r->chunks[j] = r->chunks[j - 1];
            j--;
        }
        r->chunks[j] = key;
    }

    /* Merge overlapping/adjacent chunks */
    size_t write = 0;
    for (size_t read = 1; read < r->num_chunks; read++) {
        uint64_t w_end = r->chunks[write].offset + r->chunks[write].len;
        uint64_t r_start = r->chunks[read].offset;

        if (r_start <= w_end) {
            /* Overlapping or adjacent: extend */
            uint64_t r_end = r->chunks[read].offset + r->chunks[read].len;
            if (r_end > w_end)
                r->chunks[write].len = (size_t)(r_end - r->chunks[write].offset);
        } else {
            write++;
            r->chunks[write] = r->chunks[read];
        }
    }
    r->num_chunks = write + 1;
}

int quic_stream_reasm_insert(struct quic_stream_reasm *r, uint64_t offset, const uint8_t *data,
                             size_t len, int fin) {
    if (len == 0 && !fin)
        return 0; /* nothing to do */

    uint64_t end = offset + len;

    /* Check bounds */
    if (end > QUIC_STREAM_MAX_DATA)
        return -1;

    /* If FIN was already received, check consistency */
    if (fin && r->fin_received && r->fin_offset != end)
        return -1;
    if (r->fin_received && end > r->fin_offset)
        return -1;

    /* Copy data into reassembly buffer */
    if (len > 0)
        memcpy(r->data + offset, data, len);

    /* Record the chunk */
    if (len > 0) {
        if (r->num_chunks >= QUIC_STREAM_MAX_CHUNKS)
            return -1;
        r->chunks[r->num_chunks].offset = offset;
        r->chunks[r->num_chunks].len = len;
        r->num_chunks++;
        merge_chunks(r);
    }

    /* Update total size */
    if (end > r->total_size)
        r->total_size = end;

    /* Record FIN */
    if (fin) {
        r->fin_received = 1;
        r->fin_offset = end;
    }

    return 0;
}

uint64_t quic_stream_reasm_readable(const struct quic_stream_reasm *r) {
    if (r->num_chunks == 0)
        return 0;
    if (r->chunks[0].offset != 0)
        return 0;
    return r->chunks[0].len;
}

int quic_stream_reasm_complete(const struct quic_stream_reasm *r) {
    if (!r->fin_received)
        return 0;
    uint64_t readable = quic_stream_reasm_readable(r);
    return readable >= r->fin_offset;
}

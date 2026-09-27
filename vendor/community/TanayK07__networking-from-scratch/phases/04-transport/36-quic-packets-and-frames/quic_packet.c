/*
 * quic_packet.c -- QUIC Long/Short header parser and frame classifier
 *
 * Implements RFC 9000 §17.2 (Long Header), §17.3 (Short Header),
 * and §19 (frame type classification).
 */
#include "quic_packet.h"
#include <arpa/inet.h>
#include <string.h>

/* ---------------------------------------------------------------
 * Long Header (RFC 9000, §17.2)
 * --------------------------------------------------------------- */

int quic_long_hdr_parse(const uint8_t *buf, size_t len, struct quic_long_hdr *out,
                        size_t *consumed) {
    if (len < 7)
        return -1;

    /* First byte must have Header Form = 1 */
    if ((buf[0] & QUIC_HEADER_FORM_BIT) == 0)
        return -1;

    out->first_byte = buf[0];

    /* Version: 4 bytes, network order */
    out->version = (uint32_t)buf[1] << 24 | (uint32_t)buf[2] << 16 | (uint32_t)buf[3] << 8 | buf[4];

    size_t pos = 5;

    /* Destination Connection ID */
    out->dcid_len = buf[pos++];
    if (out->dcid_len > QUIC_MAX_CID_LEN || pos + out->dcid_len > len)
        return -1;
    memcpy(out->dcid, buf + pos, out->dcid_len);
    pos += out->dcid_len;

    /* Source Connection ID */
    if (pos >= len)
        return -1;
    out->scid_len = buf[pos++];
    if (out->scid_len > QUIC_MAX_CID_LEN || pos + out->scid_len > len)
        return -1;
    memcpy(out->scid, buf + pos, out->scid_len);
    pos += out->scid_len;

    *consumed = pos;
    return 0;
}

size_t quic_long_hdr_build(const struct quic_long_hdr *hdr, uint8_t *buf, size_t len) {
    size_t need = 5 + 1 + hdr->dcid_len + 1 + hdr->scid_len;
    if (len < need)
        return 0;
    if (hdr->dcid_len > QUIC_MAX_CID_LEN || hdr->scid_len > QUIC_MAX_CID_LEN)
        return 0;

    buf[0] = hdr->first_byte;
    buf[1] = (uint8_t)(hdr->version >> 24);
    buf[2] = (uint8_t)(hdr->version >> 16);
    buf[3] = (uint8_t)(hdr->version >> 8);
    buf[4] = (uint8_t)(hdr->version);

    size_t pos = 5;
    buf[pos++] = hdr->dcid_len;
    memcpy(buf + pos, hdr->dcid, hdr->dcid_len);
    pos += hdr->dcid_len;

    buf[pos++] = hdr->scid_len;
    memcpy(buf + pos, hdr->scid, hdr->scid_len);
    pos += hdr->scid_len;

    return pos;
}

/* ---------------------------------------------------------------
 * Short Header (RFC 9000, §17.3)
 * --------------------------------------------------------------- */

int quic_short_hdr_parse(const uint8_t *buf, size_t len, uint8_t dcid_len,
                         struct quic_short_hdr *out, size_t *consumed) {
    if (len < 1)
        return -1;

    /* First byte must have Header Form = 0 */
    if ((buf[0] & QUIC_HEADER_FORM_BIT) != 0)
        return -1;

    out->first_byte = buf[0];
    out->dcid_len = dcid_len;

    size_t need = 1 + dcid_len;
    if (len < need)
        return -1;

    memcpy(out->dcid, buf + 1, dcid_len);

    *consumed = need;
    return 0;
}

size_t quic_short_hdr_build(const struct quic_short_hdr *hdr, uint8_t *buf, size_t len) {
    size_t need = 1 + hdr->dcid_len;
    if (len < need || hdr->dcid_len > QUIC_MAX_CID_LEN)
        return 0;

    buf[0] = hdr->first_byte;
    memcpy(buf + 1, hdr->dcid, hdr->dcid_len);

    return need;
}

/* ---------------------------------------------------------------
 * Frame type names (RFC 9000, §19)
 * --------------------------------------------------------------- */

const char *quic_frame_name(uint64_t ft) {
    switch (ft) {
    case 0x00:
        return "PADDING";
    case 0x01:
        return "PING";
    case 0x02:
        return "ACK";
    case 0x03:
        return "ACK_ECN";
    case 0x04:
        return "RESET_STREAM";
    case 0x05:
        return "STOP_SENDING";
    case 0x06:
        return "CRYPTO";
    case 0x07:
        return "NEW_TOKEN";
    case 0x10:
        return "MAX_DATA";
    case 0x11:
        return "MAX_STREAM_DATA";
    case 0x12:
        return "MAX_STREAMS_BIDI";
    case 0x13:
        return "MAX_STREAMS_UNI";
    case 0x14:
        return "DATA_BLOCKED";
    case 0x15:
        return "STREAM_DATA_BLOCKED";
    case 0x16:
        return "STREAMS_BLOCKED_BIDI";
    case 0x17:
        return "STREAMS_BLOCKED_UNI";
    case 0x18:
        return "NEW_CONNECTION_ID";
    case 0x19:
        return "RETIRE_CONNECTION_ID";
    case 0x1a:
        return "PATH_CHALLENGE";
    case 0x1b:
        return "PATH_RESPONSE";
    case 0x1c:
        return "CONNECTION_CLOSE";
    case 0x1d:
        return "CONNECTION_CLOSE_APP";
    case 0x1e:
        return "HANDSHAKE_DONE";
    default:
        if (ft >= 0x08 && ft <= 0x0f)
            return "STREAM";
        return "UNKNOWN";
    }
}

const char *quic_pkt_type_name(int pkt_type) {
    switch (pkt_type) {
    case QUIC_PKT_INITIAL:
        return "Initial";
    case QUIC_PKT_0RTT:
        return "0-RTT";
    case QUIC_PKT_HANDSHAKE:
        return "Handshake";
    case QUIC_PKT_RETRY:
        return "Retry";
    default:
        return "Unknown";
    }
}

/* ---------------------------------------------------------------
 * Frame-in-packet-type validation (RFC 9000, Table 3)
 *
 * Returns 1 if frame_type is permitted in pkt_ctx, 0 otherwise.
 * pkt_ctx is a bitmask: QUIC_PKTCTX_INITIAL | _HANDSHAKE | _0RTT | _1RTT
 * --------------------------------------------------------------- */

int quic_frame_allowed(uint64_t ft, int ctx) {
    /* Allowed-in bitmask for each frame type */
    int allowed;

    if (ft == 0x00 || ft == 0x01) {
        /* PADDING, PING: allowed everywhere */
        allowed = QUIC_PKTCTX_INITIAL | QUIC_PKTCTX_HANDSHAKE | QUIC_PKTCTX_0RTT | QUIC_PKTCTX_1RTT;
    } else if (ft == 0x02 || ft == 0x03) {
        /* ACK, ACK_ECN: Initial, Handshake, 1-RTT (NOT 0-RTT) */
        allowed = QUIC_PKTCTX_INITIAL | QUIC_PKTCTX_HANDSHAKE | QUIC_PKTCTX_1RTT;
    } else if (ft == 0x06) {
        /* CRYPTO: Initial, Handshake, 1-RTT (NOT 0-RTT) */
        allowed = QUIC_PKTCTX_INITIAL | QUIC_PKTCTX_HANDSHAKE | QUIC_PKTCTX_1RTT;
    } else if (ft >= 0x08 && ft <= 0x0f) {
        /* STREAM: 0-RTT and 1-RTT only */
        allowed = QUIC_PKTCTX_0RTT | QUIC_PKTCTX_1RTT;
    } else if (ft == 0x07) {
        /* NEW_TOKEN: 1-RTT only */
        allowed = QUIC_PKTCTX_1RTT;
    } else if (ft == 0x1e) {
        /* HANDSHAKE_DONE: 1-RTT only */
        allowed = QUIC_PKTCTX_1RTT;
    } else if (ft == 0x04 || ft == 0x05 || (ft >= 0x10 && ft <= 0x17)) {
        /* RESET_STREAM, STOP_SENDING, flow control frames: 0-RTT and 1-RTT */
        allowed = QUIC_PKTCTX_0RTT | QUIC_PKTCTX_1RTT;
    } else if (ft == 0x18 || ft == 0x19) {
        /* NEW_CONNECTION_ID, RETIRE_CONNECTION_ID: 0-RTT and 1-RTT */
        allowed = QUIC_PKTCTX_0RTT | QUIC_PKTCTX_1RTT;
    } else if (ft == 0x1a || ft == 0x1b) {
        /* PATH_CHALLENGE, PATH_RESPONSE: 0-RTT and 1-RTT */
        allowed = QUIC_PKTCTX_0RTT | QUIC_PKTCTX_1RTT;
    } else if (ft == 0x1c || ft == 0x1d) {
        /* CONNECTION_CLOSE: Initial, Handshake, 1-RTT
         * (0x1d APPLICATION close is 1-RTT only) */
        if (ft == 0x1c)
            allowed = QUIC_PKTCTX_INITIAL | QUIC_PKTCTX_HANDSHAKE | QUIC_PKTCTX_1RTT;
        else
            allowed = QUIC_PKTCTX_1RTT;
    } else {
        return 0;
    }

    return (allowed & ctx) != 0;
}

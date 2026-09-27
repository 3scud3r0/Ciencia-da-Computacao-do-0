/*
 * test_packet.c -- Tests for QUIC packet header parsing and frame classification
 */
#include "quic_packet.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- Long Header round-trip ----------------------------------------- */

static struct quic_long_hdr make_long_hdr(int pkt_type) {
    struct quic_long_hdr hdr = {0};
    hdr.first_byte =
        QUIC_HEADER_FORM_BIT | QUIC_FIXED_BIT | (uint8_t)(pkt_type << QUIC_LONG_TYPE_SHIFT);
    hdr.version = QUIC_VERSION_1;
    hdr.dcid_len = 8;
    memcpy(hdr.dcid, "\x83\x94\xc8\xf0\x3e\x51\x57\x08", 8);
    hdr.scid_len = 4;
    memcpy(hdr.scid, "\xbe\xef\xca\xfe", 4);
    return hdr;
}

static void test_long_hdr_initial(void) {
    struct quic_long_hdr hdr = make_long_hdr(QUIC_PKT_INITIAL);
    uint8_t buf[64];
    size_t n = quic_long_hdr_build(&hdr, buf, sizeof(buf));
    assert(n > 0);

    struct quic_long_hdr parsed;
    size_t consumed;
    assert(quic_long_hdr_parse(buf, n, &parsed, &consumed) == 0);
    assert(quic_long_hdr_type(parsed.first_byte) == QUIC_PKT_INITIAL);
    assert(parsed.version == QUIC_VERSION_1);
    assert(parsed.dcid_len == 8);
    assert(memcmp(parsed.dcid, hdr.dcid, 8) == 0);
    assert(parsed.scid_len == 4);
    assert(memcmp(parsed.scid, hdr.scid, 4) == 0);
    assert(consumed == n);
}

static void test_long_hdr_0rtt(void) {
    struct quic_long_hdr hdr = make_long_hdr(QUIC_PKT_0RTT);
    uint8_t buf[64];
    size_t n = quic_long_hdr_build(&hdr, buf, sizeof(buf));
    assert(n > 0);

    struct quic_long_hdr parsed;
    size_t consumed;
    assert(quic_long_hdr_parse(buf, n, &parsed, &consumed) == 0);
    assert(quic_long_hdr_type(parsed.first_byte) == QUIC_PKT_0RTT);
}

static void test_long_hdr_handshake(void) {
    struct quic_long_hdr hdr = make_long_hdr(QUIC_PKT_HANDSHAKE);
    uint8_t buf[64];
    size_t n = quic_long_hdr_build(&hdr, buf, sizeof(buf));
    assert(n > 0);

    struct quic_long_hdr parsed;
    size_t consumed;
    assert(quic_long_hdr_parse(buf, n, &parsed, &consumed) == 0);
    assert(quic_long_hdr_type(parsed.first_byte) == QUIC_PKT_HANDSHAKE);
}

static void test_long_hdr_retry(void) {
    struct quic_long_hdr hdr = make_long_hdr(QUIC_PKT_RETRY);
    uint8_t buf[64];
    size_t n = quic_long_hdr_build(&hdr, buf, sizeof(buf));
    assert(n > 0);

    struct quic_long_hdr parsed;
    size_t consumed;
    assert(quic_long_hdr_parse(buf, n, &parsed, &consumed) == 0);
    assert(quic_long_hdr_type(parsed.first_byte) == QUIC_PKT_RETRY);
}

/* ---- Short Header --------------------------------------------------- */

static void test_short_hdr_roundtrip(void) {
    struct quic_short_hdr hdr = {0};
    hdr.first_byte = QUIC_FIXED_BIT | QUIC_SHORT_SPIN_BIT | QUIC_SHORT_KEY_PHASE;
    hdr.dcid_len = 8;
    memcpy(hdr.dcid, "\xde\xad\xbe\xef\xca\xfe\x00\x01", 8);

    uint8_t buf[64];
    size_t n = quic_short_hdr_build(&hdr, buf, sizeof(buf));
    assert(n == 9); /* 1 byte first + 8 DCID */

    struct quic_short_hdr parsed;
    size_t consumed;
    assert(quic_short_hdr_parse(buf, n, 8, &parsed, &consumed) == 0);
    assert((parsed.first_byte & QUIC_SHORT_SPIN_BIT) != 0);
    assert((parsed.first_byte & QUIC_SHORT_KEY_PHASE) != 0);
    assert(parsed.dcid_len == 8);
    assert(memcmp(parsed.dcid, hdr.dcid, 8) == 0);
}

/* ---- Error cases ---------------------------------------------------- */

static void test_reject_truncated_long(void) {
    uint8_t buf[] = {0xc0, 0x00, 0x00}; /* too short */
    struct quic_long_hdr hdr;
    size_t consumed;
    assert(quic_long_hdr_parse(buf, sizeof(buf), &hdr, &consumed) == -1);
}

static void test_reject_truncated_short(void) {
    uint8_t buf[] = {0x40, 0xDE}; /* says DCID is 8 but only 1 byte */
    struct quic_short_hdr hdr;
    size_t consumed;
    assert(quic_short_hdr_parse(buf, sizeof(buf), 8, &hdr, &consumed) == -1);
}

static void test_reject_short_as_long(void) {
    /* First byte has Header Form = 0, should not parse as long */
    uint8_t buf[32] = {0x40};
    struct quic_long_hdr hdr;
    size_t consumed;
    assert(quic_long_hdr_parse(buf, sizeof(buf), &hdr, &consumed) == -1);
}

static void test_dcid_max_20(void) {
    /* DCID len = 21 → reject */
    uint8_t buf[64] = {0xc0, 0x00, 0x00, 0x00, 0x01, 21};
    struct quic_long_hdr hdr;
    size_t consumed;
    assert(quic_long_hdr_parse(buf, sizeof(buf), &hdr, &consumed) == -1);
}

/* ---- Header Form bit ------------------------------------------------ */

static void test_header_form_bit(void) {
    assert(quic_is_long_header(0xC0) == 1);
    assert(quic_is_long_header(0x80) == 1);
    assert(quic_is_long_header(0x40) == 0);
    assert(quic_is_long_header(0x00) == 0);
}

/* ---- Frame names ---------------------------------------------------- */

static void test_frame_names(void) {
    assert(strcmp(quic_frame_name(0x00), "PADDING") == 0);
    assert(strcmp(quic_frame_name(0x01), "PING") == 0);
    assert(strcmp(quic_frame_name(0x02), "ACK") == 0);
    assert(strcmp(quic_frame_name(0x03), "ACK_ECN") == 0);
    assert(strcmp(quic_frame_name(0x04), "RESET_STREAM") == 0);
    assert(strcmp(quic_frame_name(0x05), "STOP_SENDING") == 0);
    assert(strcmp(quic_frame_name(0x06), "CRYPTO") == 0);
    assert(strcmp(quic_frame_name(0x07), "NEW_TOKEN") == 0);
    assert(strcmp(quic_frame_name(0x08), "STREAM") == 0);
    assert(strcmp(quic_frame_name(0x0b), "STREAM") == 0);
    assert(strcmp(quic_frame_name(0x0f), "STREAM") == 0);
    assert(strcmp(quic_frame_name(0x10), "MAX_DATA") == 0);
    assert(strcmp(quic_frame_name(0x18), "NEW_CONNECTION_ID") == 0);
    assert(strcmp(quic_frame_name(0x1a), "PATH_CHALLENGE") == 0);
    assert(strcmp(quic_frame_name(0x1b), "PATH_RESPONSE") == 0);
    assert(strcmp(quic_frame_name(0x1c), "CONNECTION_CLOSE") == 0);
    assert(strcmp(quic_frame_name(0x1d), "CONNECTION_CLOSE_APP") == 0);
    assert(strcmp(quic_frame_name(0x1e), "HANDSHAKE_DONE") == 0);
    assert(strcmp(quic_frame_name(0xFF), "UNKNOWN") == 0);
}

/* ---- Frame-allowed-in-packet ---------------------------------------- */

static void test_frame_allowed_initial(void) {
    /* ACK and CRYPTO allowed in Initial */
    assert(quic_frame_allowed(0x02, QUIC_PKTCTX_INITIAL) == 1);
    assert(quic_frame_allowed(0x06, QUIC_PKTCTX_INITIAL) == 1);
    /* PADDING and PING allowed everywhere */
    assert(quic_frame_allowed(0x00, QUIC_PKTCTX_INITIAL) == 1);
    assert(quic_frame_allowed(0x01, QUIC_PKTCTX_INITIAL) == 1);
    /* STREAM forbidden in Initial */
    assert(quic_frame_allowed(0x08, QUIC_PKTCTX_INITIAL) == 0);
    assert(quic_frame_allowed(0x0f, QUIC_PKTCTX_INITIAL) == 0);
    /* HANDSHAKE_DONE forbidden in Initial */
    assert(quic_frame_allowed(0x1e, QUIC_PKTCTX_INITIAL) == 0);
}

static void test_frame_allowed_0rtt(void) {
    /* STREAM allowed in 0-RTT */
    assert(quic_frame_allowed(0x08, QUIC_PKTCTX_0RTT) == 1);
    /* ACK forbidden in 0-RTT */
    assert(quic_frame_allowed(0x02, QUIC_PKTCTX_0RTT) == 0);
    /* CRYPTO forbidden in 0-RTT */
    assert(quic_frame_allowed(0x06, QUIC_PKTCTX_0RTT) == 0);
}

static void test_frame_allowed_1rtt(void) {
    /* Everything allowed in 1-RTT */
    assert(quic_frame_allowed(0x00, QUIC_PKTCTX_1RTT) == 1);
    assert(quic_frame_allowed(0x02, QUIC_PKTCTX_1RTT) == 1);
    assert(quic_frame_allowed(0x06, QUIC_PKTCTX_1RTT) == 1);
    assert(quic_frame_allowed(0x08, QUIC_PKTCTX_1RTT) == 1);
    assert(quic_frame_allowed(0x1e, QUIC_PKTCTX_1RTT) == 1);
    assert(quic_frame_allowed(0x1c, QUIC_PKTCTX_1RTT) == 1);
    assert(quic_frame_allowed(0x1d, QUIC_PKTCTX_1RTT) == 1);
}

static void test_frame_allowed_handshake(void) {
    /* ACK and CRYPTO allowed */
    assert(quic_frame_allowed(0x02, QUIC_PKTCTX_HANDSHAKE) == 1);
    assert(quic_frame_allowed(0x06, QUIC_PKTCTX_HANDSHAKE) == 1);
    /* STREAM forbidden */
    assert(quic_frame_allowed(0x08, QUIC_PKTCTX_HANDSHAKE) == 0);
    /* HANDSHAKE_DONE forbidden in Handshake packets */
    assert(quic_frame_allowed(0x1e, QUIC_PKTCTX_HANDSHAKE) == 0);
}

/* ---- Packet type names ---------------------------------------------- */

static void test_pkt_type_names(void) {
    assert(strcmp(quic_pkt_type_name(QUIC_PKT_INITIAL), "Initial") == 0);
    assert(strcmp(quic_pkt_type_name(QUIC_PKT_0RTT), "0-RTT") == 0);
    assert(strcmp(quic_pkt_type_name(QUIC_PKT_HANDSHAKE), "Handshake") == 0);
    assert(strcmp(quic_pkt_type_name(QUIC_PKT_RETRY), "Retry") == 0);
}

/* ---- Empty CIDs ----------------------------------------------------- */

static void test_empty_cids(void) {
    struct quic_long_hdr hdr = {0};
    hdr.first_byte = QUIC_HEADER_FORM_BIT | QUIC_FIXED_BIT;
    hdr.version = QUIC_VERSION_1;
    hdr.dcid_len = 0;
    hdr.scid_len = 0;

    uint8_t buf[64];
    size_t n = quic_long_hdr_build(&hdr, buf, sizeof(buf));
    assert(n == 7); /* 1 + 4 + 1 + 0 + 1 + 0 */

    struct quic_long_hdr parsed;
    size_t consumed;
    assert(quic_long_hdr_parse(buf, n, &parsed, &consumed) == 0);
    assert(parsed.dcid_len == 0);
    assert(parsed.scid_len == 0);
}

/* ---- Main ----------------------------------------------------------- */

int main(void) {
    test_long_hdr_initial();
    test_long_hdr_0rtt();
    test_long_hdr_handshake();
    test_long_hdr_retry();
    test_short_hdr_roundtrip();
    test_reject_truncated_long();
    test_reject_truncated_short();
    test_reject_short_as_long();
    test_dcid_max_20();
    test_header_form_bit();
    test_frame_names();
    test_frame_allowed_initial();
    test_frame_allowed_0rtt();
    test_frame_allowed_1rtt();
    test_frame_allowed_handshake();
    test_pkt_type_names();
    test_empty_cids();

    printf("All 17 QUIC packet tests passed.\n");
    return 0;
}

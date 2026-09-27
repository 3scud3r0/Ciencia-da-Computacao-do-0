/*
 * test_stream.c -- Tests for QUIC stream classification, STREAM frame
 *                  codec, and reassembly buffer.
 *
 * Tests pin specific bytes, cover RFC 9000 §2.1 stream ID encoding,
 * §19.8 STREAM frame format, round-trip, edge cases, and out-of-order
 * reassembly.
 */
#include "../35-quic-overview-rfc-9000/quic_varint.h"
#include "quic_stream.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ================================================================
 * Stream ID classification (RFC 9000, §2.1)
 * ================================================================ */

static void test_classify_client_bidi(void) {
    /* Stream IDs 0, 4, 8 are client-initiated bidirectional */
    uint64_t ids[] = {0, 4, 8};
    for (size_t i = 0; i < sizeof(ids) / sizeof(ids[0]); i++) {
        struct quic_stream_info info;
        quic_stream_classify(ids[i], &info);
        assert(info.initiator == QUIC_STREAM_CLIENT);
        assert(info.direction == QUIC_STREAM_BIDI);
        assert(info.seq == ids[i] / 4);
        assert(strcmp(quic_stream_type_name(ids[i]), "client-bidi") == 0);
    }
}

static void test_classify_server_bidi(void) {
    /* Stream IDs 1, 5, 9 are server-initiated bidirectional */
    uint64_t ids[] = {1, 5, 9};
    for (size_t i = 0; i < sizeof(ids) / sizeof(ids[0]); i++) {
        struct quic_stream_info info;
        quic_stream_classify(ids[i], &info);
        assert(info.initiator == QUIC_STREAM_SERVER);
        assert(info.direction == QUIC_STREAM_BIDI);
        assert(info.seq == ids[i] / 4);
        assert(strcmp(quic_stream_type_name(ids[i]), "server-bidi") == 0);
    }
}

static void test_classify_client_uni(void) {
    /* Stream IDs 2, 6, 10 are client-initiated unidirectional */
    uint64_t ids[] = {2, 6, 10};
    for (size_t i = 0; i < sizeof(ids) / sizeof(ids[0]); i++) {
        struct quic_stream_info info;
        quic_stream_classify(ids[i], &info);
        assert(info.initiator == QUIC_STREAM_CLIENT);
        assert(info.direction == QUIC_STREAM_UNI);
        assert(info.seq == ids[i] / 4);
        assert(strcmp(quic_stream_type_name(ids[i]), "client-uni") == 0);
    }
}

static void test_classify_server_uni(void) {
    /* Stream IDs 3, 7, 11 are server-initiated unidirectional */
    uint64_t ids[] = {3, 7, 11};
    for (size_t i = 0; i < sizeof(ids) / sizeof(ids[0]); i++) {
        struct quic_stream_info info;
        quic_stream_classify(ids[i], &info);
        assert(info.initiator == QUIC_STREAM_SERVER);
        assert(info.direction == QUIC_STREAM_UNI);
        assert(info.seq == ids[i] / 4);
        assert(strcmp(quic_stream_type_name(ids[i]), "server-uni") == 0);
    }
}

static void test_classify_sequence_numbers(void) {
    /* First 4 stream IDs all have seq=0, next 4 have seq=1, etc. */
    for (uint64_t id = 0; id < 12; id++) {
        struct quic_stream_info info;
        quic_stream_classify(id, &info);
        assert(info.seq == id / 4);
    }
}

/* ================================================================
 * STREAM frame build/parse (RFC 9000, §19.8)
 * ================================================================ */

static void test_frame_type_byte_no_flags(void) {
    /* Stream 0, no OFF, no LEN, no FIN → type = 0x08 */
    uint8_t data[] = "test";
    uint8_t buf[64];
    size_t written;
    int rc = quic_stream_frame_build(0, 0, data, 4, 0, buf, sizeof(buf), &written);
    assert(rc == 0);
    assert(buf[0] == 0x08);
}

static void test_frame_type_byte_all_flags(void) {
    /* Stream 4, OFF+LEN+FIN → type = 0x0f */
    uint8_t data[50];
    memset(data, 'X', sizeof(data));
    uint8_t buf[128];
    size_t written;
    int flags = QUIC_STREAM_FIN_BIT | QUIC_STREAM_LEN_BIT | QUIC_STREAM_OFF_BIT;

    int rc = quic_stream_frame_build(4, 1000, data, 50, flags, buf, sizeof(buf), &written);
    assert(rc == 0);
    assert(buf[0] == 0x0f);
}

static void test_frame_type_byte_fin_len(void) {
    /* FIN+LEN, no OFF → type = 0x0b */
    uint8_t buf[32];
    size_t written;
    int flags = QUIC_STREAM_FIN_BIT | QUIC_STREAM_LEN_BIT;
    int rc = quic_stream_frame_build(16383, 0, NULL, 0, flags, buf, sizeof(buf), &written);
    assert(rc == 0);
    assert(buf[0] == 0x0b);
}

static void test_frame_roundtrip_simple(void) {
    /* Build, then parse, verify all fields match */
    uint8_t payload[] = "Hello, QUIC!";
    size_t plen = sizeof(payload) - 1;
    uint8_t buf[64];
    size_t written;

    int rc = quic_stream_frame_build(0, 0, payload, plen, 0, buf, sizeof(buf), &written);
    assert(rc == 0);

    struct quic_stream_frame parsed;
    size_t consumed;
    rc = quic_stream_frame_parse(buf, written, &parsed, &consumed);
    assert(rc == 0);
    assert(parsed.stream_id == 0);
    assert(parsed.offset == 0);
    assert(parsed.fin == 0);
    assert(parsed.data_len == plen);
    assert(memcmp(parsed.data, payload, plen) == 0);
    assert(consumed == written);
}

static void test_frame_roundtrip_with_offset_and_fin(void) {
    uint8_t payload[50];
    memset(payload, 'Z', sizeof(payload));
    uint8_t buf[128];
    size_t written;
    int flags = QUIC_STREAM_FIN_BIT | QUIC_STREAM_LEN_BIT | QUIC_STREAM_OFF_BIT;

    int rc = quic_stream_frame_build(4, 1000, payload, 50, flags, buf, sizeof(buf), &written);
    assert(rc == 0);

    struct quic_stream_frame parsed;
    size_t consumed;
    rc = quic_stream_frame_parse(buf, written, &parsed, &consumed);
    assert(rc == 0);
    assert(parsed.type == 0x0f);
    assert(parsed.stream_id == 4);
    assert(parsed.offset == 1000);
    assert(parsed.length == 50);
    assert(parsed.fin == 1);
    assert(parsed.data_len == 50);
    assert(memcmp(parsed.data, payload, 50) == 0);
    assert(consumed == written);
}

static void test_frame_roundtrip_empty_fin(void) {
    /* Empty FIN frame: 0 data bytes, FIN+LEN set */
    uint8_t buf[32];
    size_t written;
    int flags = QUIC_STREAM_FIN_BIT | QUIC_STREAM_LEN_BIT;

    int rc = quic_stream_frame_build(16383, 0, NULL, 0, flags, buf, sizeof(buf), &written);
    assert(rc == 0);

    struct quic_stream_frame parsed;
    size_t consumed;
    rc = quic_stream_frame_parse(buf, written, &parsed, &consumed);
    assert(rc == 0);
    assert(parsed.stream_id == 16383);
    assert(parsed.offset == 0);
    assert(parsed.length == 0);
    assert(parsed.data_len == 0);
    assert(parsed.fin == 1);
}

static void test_frame_pin_bytes(void) {
    /*
     * Build a known frame and verify exact wire bytes:
     *   Stream ID 4 (varint: 0x04, 1 byte)
     *   Offset 1000 (varint: 0x43e8, 2 bytes)
     *   Length 5 (varint: 0x05, 1 byte)
     *   Data: "ABCDE"
     *   FIN set
     *   Type byte: 0x08 | OFF(0x04) | LEN(0x02) | FIN(0x01) = 0x0f
     *
     * Expected wire: 0f 04 43e8 05 4142434445
     *   = 0f 04 43 e8 05 41 42 43 44 45
     */
    uint8_t data[] = "ABCDE";
    uint8_t buf[32];
    size_t written;
    int flags = QUIC_STREAM_FIN_BIT | QUIC_STREAM_LEN_BIT | QUIC_STREAM_OFF_BIT;

    int rc = quic_stream_frame_build(4, 1000, data, 5, flags, buf, sizeof(buf), &written);
    assert(rc == 0);

    uint8_t expected[] = {
        0x0f,                        /* type: OFF+LEN+FIN */
        0x04,                        /* stream ID = 4 (1-byte varint) */
        0x43, 0xe8,                  /* offset = 1000 (2-byte varint) */
        0x05,                        /* length = 5 (1-byte varint) */
        0x41, 0x42, 0x43, 0x44, 0x45 /* "ABCDE" */
    };
    assert(written == sizeof(expected));
    assert(memcmp(buf, expected, sizeof(expected)) == 0);
}

static void test_frame_parse_without_len(void) {
    /* When LEN bit is not set, data extends to end of buffer.
     * Build manually: type=0x0c (OFF, no LEN, no FIN),
     *   stream_id=0, offset=100, data="test" */
    uint8_t buf[32];
    size_t pos = 0;
    buf[pos++] = 0x0c; /* OFF bit only */

    /* Stream ID = 0 */
    size_t n;
    quic_varint_encode(0, buf + pos, sizeof(buf) - pos, &n);
    pos += n;

    /* Offset = 100 */
    quic_varint_encode(100, buf + pos, sizeof(buf) - pos, &n);
    pos += n;

    /* Data without length prefix */
    memcpy(buf + pos, "test", 4);
    pos += 4;

    struct quic_stream_frame parsed;
    size_t consumed;
    int rc = quic_stream_frame_parse(buf, pos, &parsed, &consumed);
    assert(rc == 0);
    assert(parsed.stream_id == 0);
    assert(parsed.offset == 100);
    assert(parsed.data_len == 4);
    assert(memcmp(parsed.data, "test", 4) == 0);
    assert(parsed.fin == 0);
    assert(consumed == pos);
}

/* ---- Error cases ------------------------------------------------ */

static void test_frame_parse_reject_non_stream(void) {
    /* Type byte 0x07 is not a STREAM frame */
    uint8_t buf[] = {0x07, 0x00};
    struct quic_stream_frame parsed;
    size_t consumed;
    assert(quic_stream_frame_parse(buf, sizeof(buf), &parsed, &consumed) == -1);
}

static void test_frame_parse_reject_truncated(void) {
    /* Just the type byte, no stream ID */
    uint8_t buf[] = {0x08};
    struct quic_stream_frame parsed;
    size_t consumed;
    assert(quic_stream_frame_parse(buf, sizeof(buf), &parsed, &consumed) == -1);
}

static void test_frame_parse_reject_short_data(void) {
    /* LEN says 10 bytes of data, but only 3 follow */
    uint8_t buf[] = {
        0x0a,            /* type: LEN bit set, no OFF, no FIN */
        0x00,            /* stream ID = 0 */
        0x0a,            /* length = 10 */
        0x41, 0x42, 0x43 /* only 3 bytes of data */
    };
    struct quic_stream_frame parsed;
    size_t consumed;
    assert(quic_stream_frame_parse(buf, sizeof(buf), &parsed, &consumed) == -1);
}

static void test_frame_build_buffer_too_small(void) {
    uint8_t payload[50];
    memset(payload, 'A', sizeof(payload));
    uint8_t buf[4]; /* too small */
    size_t written;
    int rc = quic_stream_frame_build(0, 0, payload, 50, 0, buf, sizeof(buf), &written);
    assert(rc == -1);
}

/* ================================================================
 * Reassembly buffer
 * ================================================================ */

static void test_reasm_in_order(void) {
    struct quic_stream_reasm r;
    quic_stream_reasm_init(&r);

    assert(quic_stream_reasm_insert(&r, 0, (const uint8_t *)"ABC", 3, 0) == 0);
    assert(quic_stream_reasm_readable(&r) == 3);
    assert(quic_stream_reasm_complete(&r) == 0); /* no FIN yet */

    assert(quic_stream_reasm_insert(&r, 3, (const uint8_t *)"DE", 2, 1) == 0);
    assert(quic_stream_reasm_readable(&r) == 5);
    assert(quic_stream_reasm_complete(&r) == 1);
    assert(memcmp(r.data, "ABCDE", 5) == 0);
}

static void test_reasm_out_of_order(void) {
    /* Deliver 4 chunks out of order, then verify reassembly */
    struct quic_stream_reasm r;
    quic_stream_reasm_init(&r);

    /* offsets: 200, 0, 300, 100 — each 100 bytes */
    uint8_t chunk[100];

    memset(chunk, 'C', 100);
    assert(quic_stream_reasm_insert(&r, 200, chunk, 100, 0) == 0);
    assert(quic_stream_reasm_readable(&r) == 0); /* gap at 0..200 */

    memset(chunk, 'A', 100);
    assert(quic_stream_reasm_insert(&r, 0, chunk, 100, 0) == 0);
    assert(quic_stream_reasm_readable(&r) == 100); /* 0..100 contiguous */

    memset(chunk, 'D', 100);
    assert(quic_stream_reasm_insert(&r, 300, chunk, 100, 1) == 0);
    assert(quic_stream_reasm_readable(&r) == 100); /* still gap at 100..200 */

    memset(chunk, 'B', 100);
    assert(quic_stream_reasm_insert(&r, 100, chunk, 100, 0) == 0);
    assert(quic_stream_reasm_readable(&r) == 400); /* now contiguous */
    assert(quic_stream_reasm_complete(&r) == 1);

    /* Verify data content */
    for (int i = 0; i < 100; i++)
        assert(r.data[i] == 'A');
    for (int i = 100; i < 200; i++)
        assert(r.data[i] == 'B');
    for (int i = 200; i < 300; i++)
        assert(r.data[i] == 'C');
    for (int i = 300; i < 400; i++)
        assert(r.data[i] == 'D');
}

static void test_reasm_overlapping(void) {
    /* Overlapping chunks should merge */
    struct quic_stream_reasm r;
    quic_stream_reasm_init(&r);

    assert(quic_stream_reasm_insert(&r, 0, (const uint8_t *)"ABCDE", 5, 0) == 0);
    assert(quic_stream_reasm_readable(&r) == 5);

    /* Overlapping: bytes 3..7 overlap with existing 3..5 */
    assert(quic_stream_reasm_insert(&r, 3, (const uint8_t *)"DEFG", 4, 0) == 0);
    assert(quic_stream_reasm_readable(&r) == 7);
    assert(memcmp(r.data, "ABCDEFG", 7) == 0);
}

static void test_reasm_duplicate(void) {
    /* Inserting the same data twice should be idempotent */
    struct quic_stream_reasm r;
    quic_stream_reasm_init(&r);

    assert(quic_stream_reasm_insert(&r, 0, (const uint8_t *)"ABCDE", 5, 0) == 0);
    assert(quic_stream_reasm_insert(&r, 0, (const uint8_t *)"ABCDE", 5, 0) == 0);
    assert(quic_stream_reasm_readable(&r) == 5);
    assert(r.num_chunks == 1);
}

static void test_reasm_empty_fin(void) {
    /* Empty FIN with no data */
    struct quic_stream_reasm r;
    quic_stream_reasm_init(&r);

    assert(quic_stream_reasm_insert(&r, 0, (const uint8_t *)"test", 4, 0) == 0);
    assert(quic_stream_reasm_complete(&r) == 0);

    /* FIN at offset 4 with 0 bytes of data */
    assert(quic_stream_reasm_insert(&r, 4, NULL, 0, 1) == 0);
    assert(quic_stream_reasm_complete(&r) == 1);
}

static void test_reasm_fin_before_all_data(void) {
    /* Receive FIN before filling all gaps */
    struct quic_stream_reasm r;
    quic_stream_reasm_init(&r);

    /* FIN at offset 10 */
    assert(quic_stream_reasm_insert(&r, 5, (const uint8_t *)"BBBBB", 5, 1) == 0);
    assert(quic_stream_reasm_complete(&r) == 0); /* gap at 0..5 */

    assert(quic_stream_reasm_insert(&r, 0, (const uint8_t *)"AAAAA", 5, 0) == 0);
    assert(quic_stream_reasm_complete(&r) == 1);
}

static void test_reasm_inconsistent_fin(void) {
    /* Two FINs at different offsets should fail */
    struct quic_stream_reasm r;
    quic_stream_reasm_init(&r);

    assert(quic_stream_reasm_insert(&r, 0, (const uint8_t *)"AB", 2, 1) == 0);
    /* FIN at offset 2, now try FIN at offset 3 — inconsistent */
    assert(quic_stream_reasm_insert(&r, 0, (const uint8_t *)"ABC", 3, 1) == -1);
}

static void test_reasm_data_past_fin(void) {
    /* Data extending past FIN offset should fail */
    struct quic_stream_reasm r;
    quic_stream_reasm_init(&r);

    assert(quic_stream_reasm_insert(&r, 0, (const uint8_t *)"AB", 2, 1) == 0);
    /* FIN says stream ends at offset 2; try inserting at offset 1 with len 3 */
    assert(quic_stream_reasm_insert(&r, 1, (const uint8_t *)"BCD", 3, 0) == -1);
}

static void test_reasm_readable_with_gap(void) {
    /* Data starting at offset 10 — readable from 0 should be 0 */
    struct quic_stream_reasm r;
    quic_stream_reasm_init(&r);

    assert(quic_stream_reasm_insert(&r, 10, (const uint8_t *)"test", 4, 0) == 0);
    assert(quic_stream_reasm_readable(&r) == 0);
}

/* ================================================================
 * Main
 * ================================================================ */

int main(void) {
    /* Stream ID classification */
    test_classify_client_bidi();
    test_classify_server_bidi();
    test_classify_client_uni();
    test_classify_server_uni();
    test_classify_sequence_numbers();

    /* STREAM frame codec */
    test_frame_type_byte_no_flags();
    test_frame_type_byte_all_flags();
    test_frame_type_byte_fin_len();
    test_frame_roundtrip_simple();
    test_frame_roundtrip_with_offset_and_fin();
    test_frame_roundtrip_empty_fin();
    test_frame_pin_bytes();
    test_frame_parse_without_len();

    /* Frame error cases */
    test_frame_parse_reject_non_stream();
    test_frame_parse_reject_truncated();
    test_frame_parse_reject_short_data();
    test_frame_build_buffer_too_small();

    /* Reassembly */
    test_reasm_in_order();
    test_reasm_out_of_order();
    test_reasm_overlapping();
    test_reasm_duplicate();
    test_reasm_empty_fin();
    test_reasm_fin_before_all_data();
    test_reasm_inconsistent_fin();
    test_reasm_data_past_fin();
    test_reasm_readable_with_gap();

    printf("All 26 QUIC stream tests passed.\n");
    return 0;
}

/*
 * test_varint.c -- Tests for QUIC variable-length integer encoding
 *
 * Tests cover the four RFC 9000 §16 examples, boundary values,
 * error cases, and round-trip encoding/decoding.
 */
#include "quic_varint.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- RFC 9000 §16 examples ----------------------------------------- */

static void test_rfc_decode_1byte(void) {
    /* 0x25 = 00|100101 → prefix 00 → 1 byte → value 37 */
    uint8_t buf[] = {0x25};
    uint64_t val;
    size_t consumed;
    assert(quic_varint_decode(buf, sizeof(buf), &val, &consumed) == 0);
    assert(val == 37);
    assert(consumed == 1);
}

static void test_rfc_decode_2byte(void) {
    /* 0x7bbd = 01|111011 10111101 → prefix 01 → 2 bytes → value 15293 */
    uint8_t buf[] = {0x7b, 0xbd};
    uint64_t val;
    size_t consumed;
    assert(quic_varint_decode(buf, sizeof(buf), &val, &consumed) == 0);
    assert(val == 15293);
    assert(consumed == 2);
}

static void test_rfc_decode_4byte(void) {
    /* 0x9d7f3e7d → prefix 10 → 4 bytes → value 494878333 */
    uint8_t buf[] = {0x9d, 0x7f, 0x3e, 0x7d};
    uint64_t val;
    size_t consumed;
    assert(quic_varint_decode(buf, sizeof(buf), &val, &consumed) == 0);
    assert(val == 494878333);
    assert(consumed == 4);
}

static void test_rfc_decode_8byte(void) {
    /* 0xc2197c5eff14e88c → prefix 11 → 8 bytes → value 151288809941952652 */
    uint8_t buf[] = {0xc2, 0x19, 0x7c, 0x5e, 0xff, 0x14, 0xe8, 0x8c};
    uint64_t val;
    size_t consumed;
    assert(quic_varint_decode(buf, sizeof(buf), &val, &consumed) == 0);
    assert(val == 151288809941952652ULL);
    assert(consumed == 8);
}

/* ---- Encode the RFC examples and verify byte-identical output ------- */

static void test_rfc_encode_1byte(void) {
    uint8_t buf[8];
    size_t written;
    assert(quic_varint_encode(37, buf, sizeof(buf), &written) == 0);
    assert(written == 1);
    assert(buf[0] == 0x25);
}

static void test_rfc_encode_2byte(void) {
    uint8_t buf[8];
    size_t written;
    assert(quic_varint_encode(15293, buf, sizeof(buf), &written) == 0);
    assert(written == 2);
    assert(buf[0] == 0x7b && buf[1] == 0xbd);
}

static void test_rfc_encode_4byte(void) {
    uint8_t buf[8];
    size_t written;
    assert(quic_varint_encode(494878333, buf, sizeof(buf), &written) == 0);
    assert(written == 4);
    uint8_t expected[] = {0x9d, 0x7f, 0x3e, 0x7d};
    assert(memcmp(buf, expected, 4) == 0);
}

static void test_rfc_encode_8byte(void) {
    uint8_t buf[8];
    size_t written;
    assert(quic_varint_encode(151288809941952652ULL, buf, sizeof(buf), &written) == 0);
    assert(written == 8);
    uint8_t expected[] = {0xc2, 0x19, 0x7c, 0x5e, 0xff, 0x14, 0xe8, 0x8c};
    assert(memcmp(buf, expected, 8) == 0);
}

/* ---- Boundary values ------------------------------------------------ */

static void test_zero(void) {
    uint8_t buf[8];
    size_t written;
    assert(quic_varint_encode(0, buf, sizeof(buf), &written) == 0);
    assert(written == 1);
    assert(buf[0] == 0x00);

    uint64_t val;
    size_t consumed;
    assert(quic_varint_decode(buf, written, &val, &consumed) == 0);
    assert(val == 0);
}

static void test_boundary_1_to_2(void) {
    /* 63 fits in 1 byte, 64 needs 2 bytes */
    assert(quic_varint_len(63) == 1);
    assert(quic_varint_len(64) == 2);

    uint8_t buf[8];
    size_t written;
    assert(quic_varint_encode(63, buf, sizeof(buf), &written) == 0);
    assert(written == 1);
    assert(quic_varint_encode(64, buf, sizeof(buf), &written) == 0);
    assert(written == 2);
}

static void test_boundary_2_to_4(void) {
    /* 16383 fits in 2 bytes, 16384 needs 4 bytes */
    assert(quic_varint_len(16383) == 2);
    assert(quic_varint_len(16384) == 4);
}

static void test_boundary_4_to_8(void) {
    /* 1073741823 fits in 4 bytes, 1073741824 needs 8 bytes */
    assert(quic_varint_len(1073741823) == 4);
    assert(quic_varint_len(1073741824) == 8);
}

static void test_max_value(void) {
    uint64_t max = QUIC_VARINT_MAX;
    assert(quic_varint_len(max) == 8);

    uint8_t buf[8];
    size_t written;
    assert(quic_varint_encode(max, buf, sizeof(buf), &written) == 0);
    assert(written == 8);

    uint64_t val;
    size_t consumed;
    assert(quic_varint_decode(buf, written, &val, &consumed) == 0);
    assert(val == max);
}

/* ---- Error cases ---------------------------------------------------- */

static void test_overflow(void) {
    uint64_t too_big = QUIC_VARINT_MAX + 1;
    assert(quic_varint_len(too_big) == 0);

    uint8_t buf[8];
    size_t written;
    assert(quic_varint_encode(too_big, buf, sizeof(buf), &written) == -1);
}

static void test_truncated_2byte(void) {
    /* Prefix says 2 bytes, but only 1 byte provided */
    uint8_t buf[] = {0x7b};
    uint64_t val;
    size_t consumed;
    assert(quic_varint_decode(buf, sizeof(buf), &val, &consumed) == -1);
}

static void test_truncated_4byte(void) {
    uint8_t buf[] = {0x9d, 0x7f};
    uint64_t val;
    size_t consumed;
    assert(quic_varint_decode(buf, sizeof(buf), &val, &consumed) == -1);
}

static void test_truncated_8byte(void) {
    uint8_t buf[] = {0xc2, 0x19, 0x7c, 0x5e};
    uint64_t val;
    size_t consumed;
    assert(quic_varint_decode(buf, sizeof(buf), &val, &consumed) == -1);
}

static void test_empty_buffer(void) {
    uint64_t val;
    size_t consumed;
    assert(quic_varint_decode(NULL, 0, &val, &consumed) == -1);
}

static void test_encode_buffer_too_small(void) {
    uint8_t buf[1];
    size_t written;
    /* 64 needs 2 bytes, but buffer is only 1 */
    assert(quic_varint_encode(64, buf, sizeof(buf), &written) == -1);
}

/* ---- Round-trip ------------------------------------------------------ */

static void test_roundtrip_all_boundaries(void) {
    uint64_t values[] = {0,
                         1,
                         62,
                         63,
                         64,
                         65,
                         16382,
                         16383,
                         16384,
                         16385,
                         1073741822,
                         1073741823,
                         1073741824,
                         1073741825,
                         QUIC_VARINT_MAX - 1,
                         QUIC_VARINT_MAX};
    size_t n = sizeof(values) / sizeof(values[0]);

    for (size_t i = 0; i < n; i++) {
        uint8_t buf[8];
        size_t written, consumed;
        uint64_t decoded;

        assert(quic_varint_encode(values[i], buf, sizeof(buf), &written) == 0);
        assert(quic_varint_decode(buf, written, &decoded, &consumed) == 0);
        assert(decoded == values[i]);
        assert(consumed == written);
    }
}

/* ---- Main ----------------------------------------------------------- */

int main(void) {
    /* RFC examples */
    test_rfc_decode_1byte();
    test_rfc_decode_2byte();
    test_rfc_decode_4byte();
    test_rfc_decode_8byte();
    test_rfc_encode_1byte();
    test_rfc_encode_2byte();
    test_rfc_encode_4byte();
    test_rfc_encode_8byte();

    /* Boundaries */
    test_zero();
    test_boundary_1_to_2();
    test_boundary_2_to_4();
    test_boundary_4_to_8();
    test_max_value();

    /* Errors */
    test_overflow();
    test_truncated_2byte();
    test_truncated_4byte();
    test_truncated_8byte();
    test_empty_buffer();
    test_encode_buffer_too_small();

    /* Round-trip */
    test_roundtrip_all_boundaries();

    printf("All 20 QUIC varint tests passed.\n");
    return 0;
}
